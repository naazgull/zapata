/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <algorithm>
#include <zapata/mysqlx/connector.h>
#include <zapata/uuid.h>

auto zpt::storage::mysqlx::mysql_deinit::operator()(MYSQL* _to_dispose) const -> void {
    mysql_close(_to_dispose);
}

zpt::storage::mysqlx::mysql_thread_deinit::~mysql_thread_deinit() { mysql_thread_end(); }

auto zpt::storage::mysqlx::mysql_stmt_end::operator()(MYSQL_STMT* _to_dispose) const -> void {
    mysql_stmt_close(_to_dispose);
}

zpt::storage::mysqlx::library::library() {
    expect(!mysql_library_init(0, nullptr, nullptr), "Unable to initialize MySQL library");
}

zpt::storage::mysqlx::library::~library() { mysql_library_end(); }

auto zpt::storage::mysqlx::init() -> zpt::storage::mysqlx::library& {
    static library _global;
    return _global;
}

zpt::storage::mysqlx::connection::connection(zpt::json _options)
  : __options{ _options("storage")("mysqlx") } {
    zpt::storage::mysqlx::init();
    this->open(_options("storage")("mysqlx"));
}

auto zpt::storage::mysqlx::connection::open(zpt::json _options) -> zpt::storage::connection::type* {
    this->__options = _options;

    this->__mysql.reset(mysql_init(nullptr), zpt::storage::mysqlx::mysql_deinit{});
    thread_local std::unique_ptr<zpt::storage::mysqlx::mysql_thread_deinit> _thread_end =
      std::make_unique<zpt::storage::mysqlx::mysql_thread_deinit>();

    auto _host = this->__options("host")->ok() ? this->__options("host")->string() : "127.0.0.1";
    auto _user = this->__options("user")->string();
    auto _pass = this->__options("password")->ok() ? this->__options("password")->string() : "";
    auto _port = this->__options("port")->ok() ? this->__options("port")->integer() : 3306;
    auto _ssl_mode = this->__options("ssl_mode")->ok() && this->__options("ssl_mode")->boolean();

    expect(nullptr != mysql_real_connect(this->__mysql.get(), //
                                         _host.data(),
                                         _user.data(),
                                         _pass.empty() ? nullptr : _pass.data(),
                                         nullptr,
                                         _port,
                                         nullptr,
                                         _ssl_mode ? CLIENT_SSL : 0),
           std::format("Unable to connect to 'mysqlx://{}@{}:{}?ssl_mode={}: {}",
                       _user,
                       _host,
                       _port,
                       _ssl_mode,
                       mysql_error(this->__mysql.get())));
    return this;
}

auto zpt::storage::mysqlx::connection::close() -> zpt::storage::connection::type* {
    this->__mysql.reset();
    return this;
}

auto zpt::storage::mysqlx::connection::session() const -> zpt::storage::session {
    return zpt::make_session<zpt::storage::mysqlx::session>(*this);
}

auto zpt::storage::mysqlx::connection::options() const -> zpt::json { return this->__options; }

auto zpt::storage::mysqlx::connection::mysql() const -> mysql_ptr { return this->__mysql; }

zpt::storage::mysqlx::session::session(zpt::storage::mysqlx::connection const& _connection)
  : __mysql{ _connection.mysql() } {
    expect(0 == mysql_query(this->__mysql.get(), "START TRANSACTION"),
           std::format("Transaction failed to start: {}", mysql_error(this->__mysql.get())));
}

zpt::storage::mysqlx::session::~session() { this->rollback(); }

auto zpt::storage::mysqlx::session::is_open() const -> bool { return this->__mysql != nullptr; }

auto zpt::storage::mysqlx::session::commit() -> zpt::storage::session::type* {
    expect(0 == mysql_query(this->__mysql.get(), "COMMIT"),
           std::format("Commit failed: {}", mysql_error(this->__mysql.get())));
    expect(0 == mysql_query(this->__mysql.get(), "START TRANSACTION"),
           std::format("Transaction failed to start: {}", mysql_error(this->__mysql.get())));
    return this;
}

auto zpt::storage::mysqlx::session::rollback() -> zpt::storage::session::type* {
    expect(0 == mysql_query(this->__mysql.get(), "ROLLBACK"),
           std::format("Rollback failed: {}", mysql_error(this->__mysql.get())));
    return this;
}

auto zpt::storage::mysqlx::session::sql(std::string const& _statement)
  -> zpt::storage::session::type* {
    mysql_stmt_ptr _to_exec{ mysql_stmt_init(this->__mysql.get()),
                             zpt::storage::mysqlx::mysql_stmt_end{} };
    expect(0 == mysql_stmt_prepare(_to_exec.get(), _statement.data(), _statement.length()),
           std::format(
             "failed to prepare statement '{}': {}", _statement, mysql_error(this->__mysql.get())));
    expect(0 == mysql_stmt_execute(_to_exec.get()),
           std::format(
             "failed to execute statement '{}': {}", _statement, mysql_error(this->__mysql.get())));
    return this;
}

auto zpt::storage::mysqlx::session::database(std::string const& _db) const
  -> zpt::storage::database {
    return zpt::make_database<zpt::storage::mysqlx::database>(*this, _db);
}

auto zpt::storage::mysqlx::session::mysql() const -> mysql_ptr { return this->__mysql; }

zpt::storage::mysqlx::database::database(zpt::storage::mysqlx::session const& _session,
                                         std::string const& _db)
  : __mysql{ _session.mysql() }
  , __database{ _db } {
    auto _statement = std::format("USE `{}`", this->__database);
    expect(0 == mysql_query(this->__mysql.get(), _statement.data()),
           std::format(
             "failed to execute statement '{}': {}", _statement, mysql_error(this->__mysql.get())));
}

auto zpt::storage::mysqlx::database::sql(std::string const&) -> zpt::storage::database::type* {
    expect(false, "database `sql` method not implemented for MySQL XDevAPI, use session's");
    return this;
}

auto zpt::storage::mysqlx::database::collection(std::string const& _collection) const
  -> zpt::storage::collection {
    return zpt::make_collection<zpt::storage::mysqlx::collection>(*this, _collection);
}

auto zpt::storage::mysqlx::database::mysql() const -> mysql_ptr { return this->__mysql; }

zpt::storage::mysqlx::collection::collection(zpt::storage::mysqlx::database const& _database,
                                             std::string const& _collection)
  : __mysql{ _database.mysql() }
  , __table{ _collection } {}

auto zpt::storage::mysqlx::collection::add(zpt::json _document) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::mysqlx::action_add>(*this, _document);
}

auto zpt::storage::mysqlx::collection::modify(zpt::json _search) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::mysqlx::action_modify>(*this, _search);
}

auto zpt::storage::mysqlx::collection::remove(zpt::json _search) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::mysqlx::action_remove>(*this, _search);
}

auto zpt::storage::mysqlx::collection::replace(std::string const& _id, zpt::json _document) const
  -> zpt::storage::action {
    return zpt::make_action<zpt::storage::mysqlx::action_replace>(*this, _id, _document);
}

auto zpt::storage::mysqlx::collection::find(zpt::json _search) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::mysqlx::action_find>(*this, _search);
}

auto zpt::storage::mysqlx::collection::count() -> size_t {
    auto _statement = std::format("select count(1) from `{}`", this->__table);
    expect(0 == mysql_query(this->__mysql.get(), _statement.data()),
           std::format(
             "failed to execute statement '{}': {}", _statement, mysql_error(this->__mysql.get())));
    return 0;
}

auto zpt::storage::mysqlx::collection::table() const -> std::string const& { return this->__table; }

auto zpt::storage::mysqlx::collection::mysql() const -> mysql_ptr { return this->__mysql; }

zpt::storage::mysqlx::action::action(zpt::storage::mysqlx::collection const& _collection)
  : __mysql{ _collection.mysql() }
  , __table{ _collection.table() } {}

auto zpt::storage::mysqlx::action::statement() const -> mysql_stmt_ptr { return this->__statement; }

auto zpt::storage::mysqlx::action::mysql() const -> mysql_ptr { return this->__mysql; }

zpt::storage::mysqlx::action_add::action_add(zpt::storage::mysqlx::collection const& _collection,
                                             zpt::json _document)
  : zpt::storage::mysqlx::action::action{ _collection }
  , __underlying{ zpt::json::array() }
  , __generated_ids{ zpt::json::array() } {
    this->__underlying << _document;
}

auto zpt::storage::mysqlx::action_add::add(zpt::json _document) -> zpt::storage::action::type* {
    this->__underlying << _document;
    return this;
}

auto zpt::storage::mysqlx::action_add::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from an 'add' action");
    return this;
}

auto zpt::storage::mysqlx::action_add::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from an 'add' action");
    return this;
}

auto zpt::storage::mysqlx::action_add::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from an 'add' action");
    return this;
}

auto zpt::storage::mysqlx::action_add::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from an 'add' action");
    return this;
}

auto zpt::storage::mysqlx::action_add::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't set from an 'add' action");
    return this;
}

auto zpt::storage::mysqlx::action_add::unset(std::string const&) -> zpt::storage::action::type* {
    expect(false, "can't unset from an 'add' action");
    return this;
}

auto zpt::storage::mysqlx::action_add::patch(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't patch from an 'add' action");
    return this;
}

auto zpt::storage::mysqlx::action_add::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_add::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_add::offset(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_add::limit(size_t) -> zpt::storage::action::type* { return this; }

auto zpt::storage::mysqlx::action_add::bind(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_add::execute() -> zpt::storage::result {
    std::ostringstream _oss;
    for (auto [_, __, _record] : this->__underlying) {
        auto _id = zpt::uuid{}.to_base64_string();
        _record << "_id" << _id;
        this->__generated_ids << _id;
        _oss << std::vformat(zpt::storage::mysqlx::to_insert(_record),
                             std::make_format_args(this->__table));
    }
    _oss << std::flush;
    auto _sql = _oss.str();

    this->__statement.reset(mysql_stmt_init(this->__mysql.get()),
                            zpt::storage::mysqlx::mysql_stmt_end{});
    expect(
      0 == mysql_stmt_prepare(this->__statement.get(), _sql.data(), _sql.length()),
      std::format("failed to prepare statement '{}': {}", _sql, mysql_error(this->__mysql.get())));
    expect(
      0 == mysql_stmt_execute(this->__statement.get()),
      std::format("failed to execute statement '{}': {}", _sql, mysql_error(this->__mysql.get())));

    zpt::storage::result _to_return = zpt::make_result<zpt::storage::mysqlx::result>(*this);
    return _to_return;
}

auto zpt::storage::mysqlx::action_add::get_generated_ids() const -> zpt::json {
    return this->__generated_ids;
}

zpt::storage::mysqlx::action_modify::action_modify(
  zpt::storage::mysqlx::collection const& _collection,
  zpt::json _search)
  : zpt::storage::mysqlx::action::action{ _collection }
  , __underlying{ zpt::json::object() }
  , __filter{ _search }
  , __bind{ zpt::json::object() } {}

auto zpt::storage::mysqlx::action_modify::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'modify' action");
    return this;
}

auto zpt::storage::mysqlx::action_modify::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'modify' action");
    return this;
}

auto zpt::storage::mysqlx::action_modify::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'modify' action");
    return this;
}

auto zpt::storage::mysqlx::action_modify::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'modify' action");
    return this;
}

auto zpt::storage::mysqlx::action_modify::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'modify' action");
    return this;
}

auto zpt::storage::mysqlx::action_modify::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    this->__underlying << _attribute << _value;
    return this;
}

auto zpt::storage::mysqlx::action_modify::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    this->__underlying << _attribute << zpt::undefined;
    return this;
}

auto zpt::storage::mysqlx::action_modify::patch(zpt::json _document)
  -> zpt::storage::action::type* {
    this->__underlying += _document;
    return this;
}

auto zpt::storage::mysqlx::action_modify::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    expect(false, "can't sort from a 'modify' action");
    return this;
}

auto zpt::storage::mysqlx::action_modify::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_modify::offset(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_modify::limit(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_modify::bind(zpt::json _map) -> zpt::storage::action::type* {
    this->__bind += _map;
    return this;
}

auto zpt::storage::mysqlx::action_modify::execute() -> zpt::storage::result {
    if (this->__filter->ok()) {
        for (auto const& [_, _key, _value] : this->__bind) {
            zpt::replace(this->__filter->string(),
                         std::format(":{}", _key),
                         zpt::storage::mysqlx::quote(_value));
        }
    }

    std::ostringstream _oss;
    _oss << std::vformat(zpt::storage::mysqlx::to_update(this->__underlying, this->__filter),
                         std::make_format_args(this->__table));
    _oss << std::flush;
    auto _sql = _oss.str();

    this->__statement.reset(mysql_stmt_init(this->__mysql.get()),
                            zpt::storage::mysqlx::mysql_stmt_end{});
    expect(
      0 == mysql_stmt_prepare(this->__statement.get(), _sql.data(), _sql.length()),
      std::format("failed to prepare statement '{}': {}", _sql, mysql_error(this->__mysql.get())));
    expect(
      0 == mysql_stmt_execute(this->__statement.get()),
      std::format("failed to execute statement '{}': {}", _sql, mysql_error(this->__mysql.get())));

    zpt::storage::result _to_return = zpt::make_result<zpt::storage::mysqlx::result>(*this);
    return _to_return;
}

zpt::storage::mysqlx::action_remove::action_remove(
  zpt::storage::mysqlx::collection const& _collection,
  zpt::json _search)
  : zpt::storage::mysqlx::action::action{ _collection }
  , __filter{ _search } {}

auto zpt::storage::mysqlx::action_remove::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'remove' action");
    return this;
}

auto zpt::storage::mysqlx::action_remove::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'remove' action");
    return this;
}

auto zpt::storage::mysqlx::action_remove::remove(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_remove::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'remove' action");
    return this;
}

auto zpt::storage::mysqlx::action_remove::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'remove' action");
    return this;
}

auto zpt::storage::mysqlx::action_remove::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_remove::unset(std::string const&) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_remove::patch(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_remove::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_remove::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_remove::offset(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_remove::limit(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_remove::bind(zpt::json _map) -> zpt::storage::action::type* {
    this->__bind += _map;
    return this;
}

auto zpt::storage::mysqlx::action_remove::execute() -> zpt::storage::result {
    if (this->__filter->ok()) {
        for (auto const& [_, _key, _value] : this->__bind) {
            zpt::replace(this->__filter->string(),
                         std::format(":{}", _key),
                         zpt::storage::mysqlx::quote(_value));
        }
    }

    std::ostringstream _oss;
    _oss << std::vformat(zpt::storage::mysqlx::to_delete(this->__filter),
                         std::make_format_args(this->__table));
    _oss << std::flush;
    auto _sql = _oss.str();

    this->__statement.reset(mysql_stmt_init(this->__mysql.get()),
                            zpt::storage::mysqlx::mysql_stmt_end{});
    expect(
      0 == mysql_stmt_prepare(this->__statement.get(), _sql.data(), _sql.length()),
      std::format("failed to prepare statement '{}': {}", _sql, mysql_error(this->__mysql.get())));
    expect(
      0 == mysql_stmt_execute(this->__statement.get()),
      std::format("failed to execute statement '{}': {}", _sql, mysql_error(this->__mysql.get())));

    zpt::storage::result _to_return = zpt::make_result<zpt::storage::mysqlx::result>(*this);
    return _to_return;
}

zpt::storage::mysqlx::action_replace::action_replace(
  zpt::storage::mysqlx::collection const& _collection,
  std::string _id,
  zpt::json _document)
  : zpt::storage::mysqlx::action::action{ _collection }
  , __underlying{ _document } {
    this->__underlying << "_id" << _id;
}

auto zpt::storage::mysqlx::action_replace::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'replace' action");
    return this;
}

auto zpt::storage::mysqlx::action_replace::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'replace' action");
    return this;
}

auto zpt::storage::mysqlx::action_replace::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'replace' action");
    return this;
}

auto zpt::storage::mysqlx::action_replace::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'replace' action");
    return this;
}

auto zpt::storage::mysqlx::action_replace::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'replace' action");
    return this;
}

auto zpt::storage::mysqlx::action_replace::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_replace::unset(std::string const&)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_replace::patch(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_replace::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_replace::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_replace::offset(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_replace::limit(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_replace::bind(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_replace::execute() -> zpt::storage::result {
    std::ostringstream _oss;
    _oss << std::vformat(zpt::storage::mysqlx::to_replace(this->__underlying),
                         std::make_format_args(this->__table));
    _oss << std::flush;
    auto _sql = _oss.str();

    this->__statement.reset(mysql_stmt_init(this->__mysql.get()),
                            zpt::storage::mysqlx::mysql_stmt_end{});
    expect(
      0 == mysql_stmt_prepare(this->__statement.get(), _sql.data(), _sql.length()),
      std::format("failed to prepare statement '{}': {}", _sql, mysql_error(this->__mysql.get())));
    expect(
      0 == mysql_stmt_execute(this->__statement.get()),
      std::format("failed to execute statement '{}': {}", _sql, mysql_error(this->__mysql.get())));

    zpt::storage::result _to_return = zpt::make_result<zpt::storage::mysqlx::result>(*this);
    return _to_return;
}

zpt::storage::mysqlx::action_find::action_find(zpt::storage::mysqlx::collection const& _collection)
  : zpt::storage::mysqlx::action::action{ _collection } {}

zpt::storage::mysqlx::action_find::action_find(zpt::storage::mysqlx::collection const& _collection,
                                               zpt::json _search)
  : zpt::storage::mysqlx::action::action{ _collection }
  , __underlying{ _search }
  , __fields{ zpt::json::array() }
  , __bind{ zpt::json::object() }
  , __suffix{ zpt::json::object() } {}

auto zpt::storage::mysqlx::action_find::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'find' action");
    return this;
}

auto zpt::storage::mysqlx::action_find::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'find' action");
    return this;
}

auto zpt::storage::mysqlx::action_find::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'find' action");
    return this;
}

auto zpt::storage::mysqlx::action_find::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'find' action");
    return this;
}

auto zpt::storage::mysqlx::action_find::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'find' action");
    return this;
}

auto zpt::storage::mysqlx::action_find::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_find::unset(std::string const&) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_find::patch(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mysqlx::action_find::sort(std::string const& _attribute, bool asc)
  -> zpt::storage::action::type* {
    this->__suffix["order by"][_attribute] = (asc ? "asc" : "desc");
    return this;
}

auto zpt::storage::mysqlx::action_find::fields(zpt::json _fields) -> zpt::storage::action::type* {
    this->__fields += _fields;
    return this;
}

auto zpt::storage::mysqlx::action_find::offset(size_t _rows) -> zpt::storage::action::type* {
    this->__suffix["offset"] = _rows;
    return this;
}

auto zpt::storage::mysqlx::action_find::limit(size_t _number) -> zpt::storage::action::type* {
    this->__suffix["limit"] = _number;
    return this;
}

auto zpt::storage::mysqlx::action_find::bind(zpt::json _map) -> zpt::storage::action::type* {
    this->__bind += _map;
    return this;
}

auto zpt::storage::mysqlx::action_find::execute() -> zpt::storage::result {
    if (this->__underlying->ok()) {
        for (auto const& [_, _key, _value] : this->__bind) {
            zpt::replace(this->__underlying->string(),
                         std::format(":{}", _key),
                         zpt::storage::mysqlx::quote(_value));
        }
    }

    std::ostringstream _oss;
    _oss << std::vformat(zpt::storage::mysqlx::to_query(this->__fields, this->__underlying),
                         std::make_format_args(this->__table));

    if (this->__suffix["limit"]->ok()) { _oss << " limit " << this->__suffix["limit"]; }
    if (this->__suffix["offset"]->ok()) { _oss << " offset " << this->__suffix["offset"]; }
    if (this->__suffix["order by"]->ok()) {
        _oss << " order by ";
        bool _first{ true };
        for (auto const& [_, _field, _direction] : this->__suffix["order by"]) {
            if (!_first) { _oss << ", "; }
            _first = false;
            _oss << _field << " " << _direction->string();
        }
    }

    _oss << ";" << std::flush;
    auto _sql = _oss.str();

    this->__statement.reset(mysql_stmt_init(this->__mysql.get()),
                            zpt::storage::mysqlx::mysql_stmt_end{});
    expect(
      0 == mysql_stmt_prepare(this->__statement.get(), _sql.data(), _sql.length()),
      std::format("failed to prepare statement '{}': {}", _sql, mysql_error(this->__mysql.get())));
    expect(
      0 == mysql_stmt_execute(this->__statement.get()),
      std::format("failed to execute statement '{}': {}", _sql, mysql_error(this->__mysql.get())));

    zpt::storage::result _to_return = zpt::make_result<zpt::storage::mysqlx::result>(*this);
    return _to_return;
}

zpt::storage::mysqlx::result::result(zpt::storage::mysqlx::action& _action)
  : __mysql{ _action.mysql() }
  , __statement{ _action.statement() }
  , __metadata{ _action.statement().get() } {
    mysql_stmt_bind_result(this->__statement.get(), this->__metadata.__bind.get());
    mysql_stmt_store_result(this->__statement.get());
}

zpt::storage::mysqlx::result::result(zpt::storage::mysqlx::action_add& _action)
  : zpt::storage::mysqlx::result{ static_cast<zpt::storage::mysqlx::action&>(_action) } {
    this->__generated_ids = _action.get_generated_ids();
}

zpt::storage::mysqlx::result::result(zpt::storage::mysqlx::action_modify& _action)
  : zpt::storage::mysqlx::result{ static_cast<zpt::storage::mysqlx::action&>(_action) } {}

zpt::storage::mysqlx::result::result(zpt::storage::mysqlx::action_remove& _action)
  : zpt::storage::mysqlx::result{ static_cast<zpt::storage::mysqlx::action&>(_action) } {}

zpt::storage::mysqlx::result::result(zpt::storage::mysqlx::action_replace& _action)
  : zpt::storage::mysqlx::result{ static_cast<zpt::storage::mysqlx::action&>(_action) } {}

zpt::storage::mysqlx::result::result(zpt::storage::mysqlx::action_find& _action)
  : zpt::storage::mysqlx::result{ static_cast<zpt::storage::mysqlx::action&>(_action) } {
    this->__is_doc_result = true;
}

zpt::storage::mysqlx::result::~result() { mysql_stmt_free_result(this->__statement.get()); }

auto zpt::storage::mysqlx::result::fetch(size_t _amount) -> zpt::json {
    zpt::json _result = zpt::json::array();
    if (_amount == 0) { _amount = std::numeric_limits<size_t>::max(); }

    int _status{ MYSQL_NO_DATA };
    size_t _fetched{ 0 };
    do {
        _status = mysql_stmt_fetch(this->__statement.get());
        if (_status == MYSQL_NO_DATA) { break; }

        _result << zpt::storage::mysqlx::to_json(this->__statement.get(), this->__metadata);
        ++_fetched;
    } while (_fetched != _amount);

    return _result;
}

auto zpt::storage::mysqlx::result::generated_id() -> zpt::json { return this->__generated_ids; }

auto zpt::storage::mysqlx::result::count() const -> size_t {
    if (this->__is_doc_result) { return mysql_stmt_num_rows(this->__statement.get()); }
    else { return mysql_stmt_affected_rows(this->__statement.get()); }
}

auto zpt::storage::mysqlx::result::status() const -> zpt::status { return 0; }

auto zpt::storage::mysqlx::result::message() const -> std::string { return {}; }

auto zpt::storage::mysqlx::result::to_json() const -> zpt::json {
    return { "state", { "code", this->status(), "message", this->message() } };
}
