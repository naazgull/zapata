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
#include <zapata/pgsql/connector.h>
#include <zapata/uuid.h>

// ---- PGconn deleter ----

auto zpt::storage::pgsql::pgsql_conn_deinit::operator()(PGconn* _conn) const -> void {
    if (_conn != nullptr) { PQfinish(_conn); }
}

// ---- PGresult deleter ----

auto zpt::storage::pgsql::pgsql_result_deinit::operator()(PGresult* _res) const -> void {
    if (_res != nullptr) { PQclear(_res); }
}

// ---- Library (no-op: libpq is lazy-initialized per connection) ----

zpt::storage::pgsql::library::library() {}
zpt::storage::pgsql::library::~library() {}

auto zpt::storage::pgsql::init() -> zpt::storage::pgsql::library& {
    static library _global;
    return _global;
}

// ---- Connection ----

zpt::storage::pgsql::connection::connection(zpt::json _options)
  : __options{ _options("storage")("pgsql") } {
    zpt::storage::pgsql::init();
    this->open(_options("storage")("pgsql"));
}

auto zpt::storage::pgsql::connection::open(zpt::json _options) -> zpt::storage::connection::type* {
    this->__options = _options;

    auto _host = this->__options("host")->ok() ? this->__options("host")->string() : "127.0.0.1";
    auto _user = this->__options("user")->string();
    auto _pass = this->__options("password")->ok() ? this->__options("password")->string() : "";
    auto _port =
      this->__options("port")->ok() ? std::to_string(this->__options("port")->integer()) : "5432";
    auto _db = this->__options("db")->ok() ? this->__options("db")->string() : "";

    std::ostringstream _connstr;
    _connstr << "host='" << _host << "'";
    _connstr << " port=" << _port;
    _connstr << " user='" << _user << "'";
    if (!_pass.empty()) { _connstr << " password='" << _pass << "'"; }
    if (!_db.empty()) { _connstr << " dbname='" << _db << "'"; }

    std::cout << _connstr.str() << std::endl;

    auto* _pg = PQconnectdb(_connstr.str().c_str());
    if (PQstatus(_pg) != CONNECTION_OK) {
        auto _err = std::string{ PQerrorMessage(_pg) };
        PQfinish(_pg);
        expect(false, std::format("Unable to connect to PostgreSQL: {}", _err));
    }

    this->__pgsql.reset(_pg, zpt::storage::pgsql::pgsql_conn_deinit{});
    return this;
}

auto zpt::storage::pgsql::connection::close() -> zpt::storage::connection::type* {
    this->__pgsql.reset();
    return this;
}

auto zpt::storage::pgsql::connection::session() -> zpt::storage::session {
    if (PQstatus(this->__pgsql.get()) != CONNECTION_OK) { this->open(this->__options); }
    return zpt::make_session<zpt::storage::pgsql::session>(*this);
}

auto zpt::storage::pgsql::connection::options() const -> zpt::json { return this->__options; }

auto zpt::storage::pgsql::connection::pgsql() const -> pgsql_ptr { return this->__pgsql; }

// ---- Session ----

zpt::storage::pgsql::session::session(zpt::storage::pgsql::connection const& _connection)
  : __pgsql{ _connection.pgsql() } {
    auto* _res = PQexec(this->__pgsql.get(), "BEGIN");
    auto _ok = PQresultStatus(_res) == PGRES_COMMAND_OK;
    PQclear(_res);
    expect(_ok,
           std::format("Transaction failed to start: {}", PQerrorMessage(this->__pgsql.get())));
}

zpt::storage::pgsql::session::~session() { this->rollback(); }

auto zpt::storage::pgsql::session::is_open() const -> bool {
    return this->__pgsql != nullptr && PQstatus(this->__pgsql.get()) == CONNECTION_OK;
}

auto zpt::storage::pgsql::session::commit() -> zpt::storage::session::type* {
    auto* _res = PQexec(this->__pgsql.get(), "COMMIT");
    auto _ok = PQresultStatus(_res) == PGRES_COMMAND_OK;
    PQclear(_res);
    expect(_ok, std::format("Commit failed: {}", PQerrorMessage(this->__pgsql.get())));

    _res = PQexec(this->__pgsql.get(), "BEGIN");
    _ok = PQresultStatus(_res) == PGRES_COMMAND_OK;
    PQclear(_res);
    expect(_ok,
           std::format("Transaction failed to start: {}", PQerrorMessage(this->__pgsql.get())));
    return this;
}

auto zpt::storage::pgsql::session::rollback() -> zpt::storage::session::type* {
    auto* _res = PQexec(this->__pgsql.get(), "ROLLBACK");
    auto _ok = PQresultStatus(_res) == PGRES_COMMAND_OK;
    PQclear(_res);
    // If we're already in a broken state, PQexec may fail — that's OK
    if (PQresultStatus(_res) == PGRES_FATAL_ERROR &&
        std::string{ PQerrorMessage(this->__pgsql.get()) }.find("no transaction in progress") !=
          std::string::npos) {
        PQclear(_res);
        return this;
    }
    expect(_ok, std::format("Rollback failed: {}", PQerrorMessage(this->__pgsql.get())));
    return this;
}

auto zpt::storage::pgsql::session::sql(std::string const& _statement)
  -> zpt::storage::session::type* {
    auto* _res = PQexec(this->__pgsql.get(), _statement.c_str());
    auto _ok = PQresultStatus(_res) == PGRES_COMMAND_OK || PQresultStatus(_res) == PGRES_TUPLES_OK;
    if (!_ok) {
        auto _err = std::string{ PQerrorMessage(this->__pgsql.get()) };
        PQclear(_res);
        expect(false, std::format("SQL failed: {} — {}", _err, _statement));
    }
    PQclear(_res);
    return this;
}

auto zpt::storage::pgsql::session::database(std::string const& _db) const
  -> zpt::storage::database {
    return zpt::make_database<zpt::storage::pgsql::database>(*this, _db);
}

auto zpt::storage::pgsql::session::pgsql() const -> pgsql_ptr { return this->__pgsql; }

// ---- Database ----

zpt::storage::pgsql::database::database(zpt::storage::pgsql::session const& _session,
                                        std::string const& _db)
  : __pgsql{ _session.pgsql() }
  , __database{ _db } {
    // For PostgreSQL, we just store the database name.
    // Actual "USE" is implicit via the connection string.
}

auto zpt::storage::pgsql::database::sql(std::string const&) -> zpt::storage::database::type* {
    expect(false, "database `sql` method not implemented for PostgreSQL, use session's");
    return this;
}

auto zpt::storage::pgsql::database::collection(std::string const& _collection) const
  -> zpt::storage::collection {
    return zpt::make_collection<zpt::storage::pgsql::collection>(*this, _collection);
}

auto zpt::storage::pgsql::database::pgsql() const -> pgsql_ptr { return this->__pgsql; }

// ---- Collection ----

zpt::storage::pgsql::collection::collection(zpt::storage::pgsql::database const& _database,
                                            std::string const& _collection)
  : __pgsql{ _database.pgsql() }
  , __table{ _collection } {}

auto zpt::storage::pgsql::collection::add(zpt::json _document) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::pgsql::action_add>(*this, _document);
}

auto zpt::storage::pgsql::collection::modify(zpt::json _search) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::pgsql::action_modify>(*this, _search);
}

auto zpt::storage::pgsql::collection::remove(zpt::json _search) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::pgsql::action_remove>(*this, _search);
}

auto zpt::storage::pgsql::collection::replace(std::string const& _id, zpt::json _document) const
  -> zpt::storage::action {
    return zpt::make_action<zpt::storage::pgsql::action_replace>(*this, _id, _document);
}

auto zpt::storage::pgsql::collection::find(zpt::json _search) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::pgsql::action_find>(*this, _search);
}

auto zpt::storage::pgsql::collection::count(zpt::json _search) -> size_t {
    auto _statement = std::format("SELECT count(*) FROM \"{}\"{}",
                                  this->__table,
                                  (_search->ok() && _search->string().length() != 0
                                     ? std::format(" WHERE {}", _search->string())
                                     : ""));

    auto* _res = PQexec(this->__pgsql.get(), _statement.c_str());
    auto _status = PQresultStatus(_res);
    if (_status != PGRES_TUPLES_OK) {
        auto _err = std::string{ PQerrorMessage(this->__pgsql.get()) };
        PQclear(_res);
        expect(false, std::format("count failed: {} — {}", _err, _statement));
    }

    size_t _count = 0;
    if (PQntuples(_res) > 0) { _count = std::stoul(PQgetvalue(_res, 0, 0)); }
    PQclear(_res);
    return _count;
}

auto zpt::storage::pgsql::collection::table() const -> std::string const& { return this->__table; }

auto zpt::storage::pgsql::collection::pgsql() const -> pgsql_ptr { return this->__pgsql; }

// ---- Action (base) ----

zpt::storage::pgsql::action::action(zpt::storage::pgsql::collection const& _collection)
  : __pgsql{ _collection.pgsql() }
  , __table{ _collection.table() } {}

auto zpt::storage::pgsql::action::result() const -> pgsql_result_ptr { return this->__result; }

auto zpt::storage::pgsql::action::pgsql() const -> pgsql_ptr { return this->__pgsql; }

// ---- action_add ----

zpt::storage::pgsql::action_add::action_add(zpt::storage::pgsql::collection const& _collection,
                                            zpt::json _document)
  : zpt::storage::pgsql::action::action{ _collection }
  , __underlying{ zpt::json::array() }
  , __generated_ids{ zpt::json::array() } {
    this->__underlying << _document;
}

auto zpt::storage::pgsql::action_add::add(zpt::json _document) -> zpt::storage::action::type* {
    this->__underlying << _document;
    return this;
}

auto zpt::storage::pgsql::action_add::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from an 'add' action");
    return this;
}

auto zpt::storage::pgsql::action_add::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from an 'add' action");
    return this;
}

auto zpt::storage::pgsql::action_add::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from an 'add' action");
    return this;
}

auto zpt::storage::pgsql::action_add::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from an 'add' action");
    return this;
}

auto zpt::storage::pgsql::action_add::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't set from an 'add' action");
    return this;
}

auto zpt::storage::pgsql::action_add::unset(std::string const&) -> zpt::storage::action::type* {
    expect(false, "can't unset from an 'add' action");
    return this;
}

auto zpt::storage::pgsql::action_add::patch(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't patch from an 'add' action");
    return this;
}

auto zpt::storage::pgsql::action_add::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_add::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_add::offset(size_t) -> zpt::storage::action::type* { return this; }

auto zpt::storage::pgsql::action_add::limit(size_t) -> zpt::storage::action::type* { return this; }

auto zpt::storage::pgsql::action_add::bind(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_add::execute() -> zpt::storage::result {
    std::ostringstream _oss;
    for (auto [_, __, _record] : this->__underlying) {
        auto _id = zpt::uuid{}.to_base64_string();
        _record << "_id" << _id;
        this->__generated_ids << _id;
        _oss << std::vformat(zpt::storage::pgsql::to_insert(_record),
                             std::make_format_args(this->__table));
    }
    _oss << std::flush;
    auto _sql = _oss.str();

    auto* _res = PQexec(this->__pgsql.get(), _sql.c_str());
    auto _ok = PQresultStatus(_res) == PGRES_COMMAND_OK;
    if (!_ok) {
        auto _err = std::string{ PQerrorMessage(this->__pgsql.get()) };
        PQclear(_res);
        expect(false, std::format("INSERT failed: {} — {}", _err, _sql));
    }

    this->__result.reset(_res, zpt::storage::pgsql::pgsql_result_deinit{});

    zpt::storage::result _to_return = zpt::make_result<zpt::storage::pgsql::result>(*this);
    return _to_return;
}

auto zpt::storage::pgsql::action_add::get_generated_ids() const -> zpt::json {
    return this->__generated_ids;
}

// ---- action_modify ----

zpt::storage::pgsql::action_modify::action_modify(
  zpt::storage::pgsql::collection const& _collection,
  zpt::json _search)
  : zpt::storage::pgsql::action::action{ _collection }
  , __underlying{ zpt::json::object() }
  , __filter{ _search }
  , __bind{ zpt::json::object() } {}

auto zpt::storage::pgsql::action_modify::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'modify' action");
    return this;
}

auto zpt::storage::pgsql::action_modify::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'modify' action");
    return this;
}

auto zpt::storage::pgsql::action_modify::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'modify' action");
    return this;
}

auto zpt::storage::pgsql::action_modify::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'modify' action");
    return this;
}

auto zpt::storage::pgsql::action_modify::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'modify' action");
    return this;
}

auto zpt::storage::pgsql::action_modify::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    this->__underlying << _attribute << _value;
    return this;
}

auto zpt::storage::pgsql::action_modify::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    this->__underlying << _attribute << zpt::undefined;
    return this;
}

auto zpt::storage::pgsql::action_modify::patch(zpt::json _document) -> zpt::storage::action::type* {
    this->__underlying += _document;
    return this;
}

auto zpt::storage::pgsql::action_modify::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    expect(false, "can't sort from a 'modify' action");
    return this;
}

auto zpt::storage::pgsql::action_modify::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_modify::offset(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_modify::limit(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_modify::bind(zpt::json _map) -> zpt::storage::action::type* {
    this->__bind += _map;
    return this;
}

auto zpt::storage::pgsql::action_modify::execute() -> zpt::storage::result {
    if (this->__filter->ok()) {
        for (auto const& [_, _key, _value] : this->__bind) {
            zpt::replace(this->__filter->string(),
                         std::format(":{}", _key),
                         zpt::storage::pgsql::quote(_value));
        }
    }

    std::ostringstream _oss;
    _oss << std::vformat(zpt::storage::pgsql::to_update(this->__underlying, this->__filter),
                         std::make_format_args(this->__table));
    _oss << std::flush;
    auto _sql = _oss.str();

    auto* _res = PQexec(this->__pgsql.get(), _sql.c_str());
    auto _ok = PQresultStatus(_res) == PGRES_COMMAND_OK;
    if (!_ok) {
        auto _err = std::string{ PQerrorMessage(this->__pgsql.get()) };
        PQclear(_res);
        expect(false, std::format("UPDATE failed: {} — {}", _err, _sql));
    }

    this->__result.reset(_res, zpt::storage::pgsql::pgsql_result_deinit{});

    zpt::storage::result _to_return = zpt::make_result<zpt::storage::pgsql::result>(*this);
    return _to_return;
}

// ---- action_remove ----

zpt::storage::pgsql::action_remove::action_remove(
  zpt::storage::pgsql::collection const& _collection,
  zpt::json _search)
  : zpt::storage::pgsql::action::action{ _collection }
  , __filter{ _search } {}

auto zpt::storage::pgsql::action_remove::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'remove' action");
    return this;
}

auto zpt::storage::pgsql::action_remove::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'remove' action");
    return this;
}

auto zpt::storage::pgsql::action_remove::remove(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_remove::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'remove' action");
    return this;
}

auto zpt::storage::pgsql::action_remove::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'remove' action");
    return this;
}

auto zpt::storage::pgsql::action_remove::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_remove::unset(std::string const&) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_remove::patch(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_remove::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_remove::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_remove::offset(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_remove::limit(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_remove::bind(zpt::json _map) -> zpt::storage::action::type* {
    this->__bind += _map;
    return this;
}

auto zpt::storage::pgsql::action_remove::execute() -> zpt::storage::result {
    if (this->__filter->ok()) {
        for (auto const& [_, _key, _value] : this->__bind) {
            zpt::replace(this->__filter->string(),
                         std::format(":{}", _key),
                         zpt::storage::pgsql::quote(_value));
        }
    }

    std::ostringstream _oss;
    _oss << std::vformat(zpt::storage::pgsql::to_delete(this->__filter),
                         std::make_format_args(this->__table));
    _oss << std::flush;
    auto _sql = _oss.str();

    auto* _res = PQexec(this->__pgsql.get(), _sql.c_str());
    auto _ok = PQresultStatus(_res) == PGRES_COMMAND_OK;
    if (!_ok) {
        auto _err = std::string{ PQerrorMessage(this->__pgsql.get()) };
        PQclear(_res);
        expect(false, std::format("DELETE failed: {} — {}", _err, _sql));
    }

    this->__result.reset(_res, zpt::storage::pgsql::pgsql_result_deinit{});

    zpt::storage::result _to_return = zpt::make_result<zpt::storage::pgsql::result>(*this);
    return _to_return;
}

// ---- action_replace ----

zpt::storage::pgsql::action_replace::action_replace(
  zpt::storage::pgsql::collection const& _collection,
  std::string _id,
  zpt::json _document)
  : zpt::storage::pgsql::action::action{ _collection }
  , __underlying{ _document } {
    this->__underlying << "_id" << _id;
}

auto zpt::storage::pgsql::action_replace::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'replace' action");
    return this;
}

auto zpt::storage::pgsql::action_replace::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'replace' action");
    return this;
}

auto zpt::storage::pgsql::action_replace::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'replace' action");
    return this;
}

auto zpt::storage::pgsql::action_replace::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'replace' action");
    return this;
}

auto zpt::storage::pgsql::action_replace::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'replace' action");
    return this;
}

auto zpt::storage::pgsql::action_replace::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_replace::unset(std::string const&) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_replace::patch(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_replace::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_replace::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_replace::offset(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_replace::limit(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_replace::bind(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_replace::execute() -> zpt::storage::result {
    std::ostringstream _oss;
    _oss << std::vformat(zpt::storage::pgsql::to_upsert(this->__underlying),
                         std::make_format_args(this->__table));
    _oss << std::flush;
    auto _sql = _oss.str();

    auto* _res = PQexec(this->__pgsql.get(), _sql.c_str());
    auto _ok = PQresultStatus(_res) == PGRES_COMMAND_OK;
    if (!_ok) {
        auto _err = std::string{ PQerrorMessage(this->__pgsql.get()) };
        PQclear(_res);
        expect(false, std::format("UPSERT failed: {} — {}", _err, _sql));
    }

    this->__result.reset(_res, zpt::storage::pgsql::pgsql_result_deinit{});

    zpt::storage::result _to_return = zpt::make_result<zpt::storage::pgsql::result>(*this);
    return _to_return;
}

// ---- action_find ----

zpt::storage::pgsql::action_find::action_find(zpt::storage::pgsql::collection const& _collection)
  : zpt::storage::pgsql::action::action{ _collection } {}

zpt::storage::pgsql::action_find::action_find(zpt::storage::pgsql::collection const& _collection,
                                              zpt::json _search)
  : zpt::storage::pgsql::action::action{ _collection }
  , __underlying{ _search }
  , __fields{ zpt::json::array() }
  , __bind{ zpt::json::object() }
  , __suffix{ zpt::json::object() } {}

auto zpt::storage::pgsql::action_find::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'find' action");
    return this;
}

auto zpt::storage::pgsql::action_find::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'find' action");
    return this;
}

auto zpt::storage::pgsql::action_find::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'find' action");
    return this;
}

auto zpt::storage::pgsql::action_find::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'find' action");
    return this;
}

auto zpt::storage::pgsql::action_find::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'find' action");
    return this;
}

auto zpt::storage::pgsql::action_find::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_find::unset(std::string const&) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_find::patch(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::pgsql::action_find::sort(std::string const& _attribute, bool asc)
  -> zpt::storage::action::type* {
    this->__suffix["order by"][_attribute] = (asc ? "asc" : "desc");
    return this;
}

auto zpt::storage::pgsql::action_find::fields(zpt::json _fields) -> zpt::storage::action::type* {
    this->__fields += _fields;
    return this;
}

auto zpt::storage::pgsql::action_find::offset(size_t _rows) -> zpt::storage::action::type* {
    this->__suffix["offset"] = _rows;
    return this;
}

auto zpt::storage::pgsql::action_find::limit(size_t _number) -> zpt::storage::action::type* {
    this->__suffix["limit"] = _number;
    return this;
}

auto zpt::storage::pgsql::action_find::bind(zpt::json _map) -> zpt::storage::action::type* {
    this->__bind += _map;
    return this;
}

auto zpt::storage::pgsql::action_find::execute() -> zpt::storage::result {
    if (this->__underlying->ok()) {
        for (auto const& [_, _key, _value] : this->__bind) {
            zpt::replace(this->__underlying->string(),
                         std::format(":{}", _key),
                         zpt::storage::pgsql::quote(_value));
        }
    }

    std::ostringstream _oss;
    _oss << std::vformat(zpt::storage::pgsql::to_query(this->__fields, this->__underlying),
                         std::make_format_args(this->__table));

    if (this->__suffix["order by"]->ok()) {
        _oss << " ORDER BY ";
        bool _first{ true };
        for (auto const& [_, _field, _direction] : this->__suffix["order by"]) {
            if (!_first) { _oss << ", "; }
            _first = false;
            _oss << "\"" << _field << "\" " << _direction->string();
        }
    }
    if (this->__suffix["limit"]->ok()) { _oss << " LIMIT " << this->__suffix["limit"]; }
    if (this->__suffix["offset"]->ok()) { _oss << " OFFSET " << this->__suffix["offset"]; }

    _oss << ";";
    auto _sql = _oss.str();

    auto* _res = PQexec(this->__pgsql.get(), _sql.c_str());
    auto _status = PQresultStatus(_res);
    if (_status != PGRES_TUPLES_OK && _status != PGRES_COMMAND_OK) {
        auto _err = std::string{ PQerrorMessage(this->__pgsql.get()) };
        PQclear(_res);
        expect(false, std::format("SELECT failed: {} — {}", _err, _sql));
    }

    this->__result.reset(_res, zpt::storage::pgsql::pgsql_result_deinit{});

    zpt::storage::result _to_return = zpt::make_result<zpt::storage::pgsql::result>(*this);
    return _to_return;
}

// ---- result ----

zpt::storage::pgsql::result::result(zpt::storage::pgsql::action& _action)
  : __pgsql{ _action.pgsql() }
  , __result{ _action.result() } {}

zpt::storage::pgsql::result::result(zpt::storage::pgsql::action_add& _action)
  : zpt::storage::pgsql::result{ static_cast<zpt::storage::pgsql::action&>(_action) } {
    this->__generated_ids = _action.get_generated_ids();
}

zpt::storage::pgsql::result::result(zpt::storage::pgsql::action_modify& _action)
  : zpt::storage::pgsql::result{ static_cast<zpt::storage::pgsql::action&>(_action) } {}

zpt::storage::pgsql::result::result(zpt::storage::pgsql::action_remove& _action)
  : zpt::storage::pgsql::result{ static_cast<zpt::storage::pgsql::action&>(_action) } {}

zpt::storage::pgsql::result::result(zpt::storage::pgsql::action_replace& _action)
  : zpt::storage::pgsql::result{ static_cast<zpt::storage::pgsql::action&>(_action) } {}

zpt::storage::pgsql::result::result(zpt::storage::pgsql::action_find& _action)
  : zpt::storage::pgsql::result{ static_cast<zpt::storage::pgsql::action&>(_action) } {
    this->__is_doc_result = true;
}

zpt::storage::pgsql::result::~result() {}

auto zpt::storage::pgsql::result::fetch(size_t _amount) -> zpt::json {
    zpt::json _result = zpt::json::array();
    if (_amount == 0) { _amount = std::numeric_limits<size_t>::max(); }

    auto* _res = this->__result.get();
    if (_res == nullptr) { return _result; }

    int _total_rows = PQntuples(_res);
    size_t _fetched{ 0 };
    for (int _row = 0; _row < _total_rows && _fetched < _amount; ++_row) {
        _result << zpt::storage::pgsql::to_json(_res, _row);
        ++_fetched;
    }

    return _result;
}

auto zpt::storage::pgsql::result::generated_id() -> zpt::json { return this->__generated_ids; }

auto zpt::storage::pgsql::result::count() const -> size_t {
    auto* _res = this->__result.get();
    if (_res == nullptr) { return 0; }
    if (this->__is_doc_result) { return static_cast<size_t>(PQntuples(_res)); }
    auto* _tag = PQcmdTuples(_res);
    if (_tag) { return std::stoul(_tag); }
    return 0;
}

auto zpt::storage::pgsql::result::status() const -> zpt::status { return 0; }

auto zpt::storage::pgsql::result::message() const -> std::string { return {}; }

auto zpt::storage::pgsql::result::to_json() const -> zpt::json {
    return { "state", { "code", this->status(), "message", this->message() } };
}
