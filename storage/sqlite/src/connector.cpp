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

#include <zapata/sqlite/connector.h>
#include <zapata/base/sentry.h>
#include <algorithm>

#define sqlite_expect(_error, _message)                                                                      \
    {                                                                                                        \
        auto __error__ = _error;                                                                             \
        expect(!zpt::storage::sqlite::is_error(__error__),                                                   \
               std::get<0>(__messages[__error__])                                                            \
                 << "(" << std::get<1>(__messages[__error__]) << "): " << _message);                         \
    }
#define sqlite_print(_error)                                                                                 \
    {                                                                                                        \
        auto __error__ = _error;                                                                             \
        zlog(std::get<0>(__messages[__error__]) << ": " << std::get<1>(__messages[__error__]), zpt::info);   \
    }

std::map<int, std::tuple<std::string, std::string>> __messages = {
    { SQLITE_OK, { "SQLITE_OK", "Successful result" } },
    { SQLITE_ERROR, { "SQLITE_ERROR", "Generic error" } },
    { SQLITE_INTERNAL, { "SQLITE_INTERNAL", "Internal logic error in SQLite" } },
    { SQLITE_PERM, { "SQLITE_PERM", "Access permission denied" } },
    { SQLITE_ABORT, { "SQLITE_ABORT", "Callback routine requested an abort" } },
    { SQLITE_BUSY, { "SQLITE_BUSY", "The database file is locked" } },
    { SQLITE_LOCKED, { "SQLITE_LOCKED", "A table in the database is locked" } },
    { SQLITE_NOMEM, { "SQLITE_NOMEM", "A malloc() failed" } },
    { SQLITE_READONLY, { "SQLITE_READONLY", "Attempt to write a readonly database" } },
    { SQLITE_INTERRUPT, { "SQLITE_INTERRUPT", "Operation terminated by sqlite3_interrupt(" } },
    { SQLITE_IOERR, { "SQLITE_IOERR", "Some kind of disk I/O error occurred" } },
    { SQLITE_CORRUPT, { "SQLITE_CORRUPT", "The database disk image is malformed" } },
    { SQLITE_NOTFOUND, { "SQLITE_NOTFOUND", "Unknown opcode in sqlite3_file_control()" } },
    { SQLITE_FULL, { "SQLITE_FULL", "Insertion failed because database is full" } },
    { SQLITE_CANTOPEN, { "SQLITE_CANTOPEN", "Unable to open the database file" } },
    { SQLITE_PROTOCOL, { "SQLITE_PROTOCOL", "Database lock protocol error" } },
    { SQLITE_EMPTY, { "SQLITE_EMPTY", "Internal use only" } },
    { SQLITE_SCHEMA, { "SQLITE_SCHEMA", "The database schema changed" } },
    { SQLITE_TOOBIG, { "SQLITE_TOOBIG", "String or BLOB exceeds size limit" } },
    { SQLITE_CONSTRAINT, { "SQLITE_CONSTRAINT", "Abort due to constraint violation" } },
    { SQLITE_MISMATCH, { "SQLITE_MISMATCH", "Data type mismatch" } },
    { SQLITE_MISUSE, { "SQLITE_MISUSE", "Library used incorrectly" } },
    { SQLITE_NOLFS, { "SQLITE_NOLFS", "Uses OS features not supported on host" } },
    { SQLITE_AUTH, { "SQLITE_AUTH", "Authorization denied" } },
    { SQLITE_FORMAT, { "SQLITE_FORMAT", "Not used" } },
    { SQLITE_RANGE, { "SQLITE_RANGE", "2nd parameter to sqlite3_bind out of range" } },
    { SQLITE_NOTADB, { "SQLITE_NOTADB", "File opened that is not a database file" } },
    { SQLITE_NOTICE, { "SQLITE_NOTICE", "Notifications from sqlite3_log()" } },
    { SQLITE_WARNING, { "SQLITE_WARNING", "Warnings from sqlite3_log()" } },
    { SQLITE_ROW, { "SQLITE_ROW", "sqlite3_step() has another row ready" } },
    { SQLITE_DONE, { "SQLITE_DONE", "sqlite3_step() has finished executing" } },
};

auto zpt::storage::sqlite::is_error(long _error) -> bool {
    return _error != SQLITE_OK && _error != SQLITE_DONE && _error != SQLITE_ROW;
}

auto zpt::storage::sqlite::from_db_doc(sqlite3_stmt* _stmt) -> zpt::json {
    zpt::json _to_return{ zpt::json::object() };
    int _count = sqlite3_column_count(_stmt);
    for (int _idx = 0; _idx != _count; ++_idx) {
        _to_return << sqlite3_column_name(_stmt, _idx);
        switch (sqlite3_column_type(_stmt, _idx)) {
            case SQLITE_INTEGER: {
                _to_return << sqlite3_column_int64(_stmt, _idx);
                break;
            }
            case SQLITE_FLOAT: {
                _to_return << sqlite3_column_double(_stmt, _idx);
                break;
            }
            case SQLITE_BLOB: {
                std::string _blob{ static_cast<const char*>(sqlite3_column_blob(_stmt, _idx)) };
                _to_return << _blob;
                break;
            }
            case SQLITE_NULL: {
                _to_return << zpt::undefined;
                break;
            }
            case SQLITE3_TEXT: {
                std::string _text{ reinterpret_cast<const char*>(sqlite3_column_text(_stmt, _idx)) };
                _to_return << _text;
                break;
            }
        }
    }
    return _to_return;
}

auto zpt::storage::sqlite::to_byte_array(zpt::json _value) -> std::tuple<char*, size_t> {
    std::string _serialized = static_cast<std::string>(_value);
    size_t _size = _serialized.length();
    char* _bytes = new char[_serialized.length()];
    std::copy(_serialized.data(), _serialized.data() + _size, _bytes);
    return { _bytes, _size };
}

auto zpt::storage::sqlite::free_byte_array(void* _to_delete) -> void {
    delete[] static_cast<char*>(_to_delete);
}

auto zpt::storage::sqlite::bind(sqlite3_stmt* _stmt, std::string const& _name, zpt::json _value) -> void {
    auto _index = sqlite3_bind_parameter_index(_stmt, (std::string{ ":" } + _name).data());
    expect(_index != 0, "No such parameter named :" << _name);
    switch (_value->type()) {
        case zpt::JSString:
        case zpt::JSObject:
        case zpt::JSArray: {
            auto [_bytes, _size] = zpt::storage::sqlite::to_byte_array(_value);
            sqlite3_bind_text(_stmt, _index, _bytes, _size, zpt::storage::sqlite::free_byte_array);
            break;
        }
        case zpt::JSInteger: {
            sqlite3_bind_int64(_stmt, _index, _value->integer());
            break;
        }
        case zpt::JSDouble: {
            sqlite3_bind_double(_stmt, _index, _value->floating());
            break;
        }
        case zpt::JSBoolean: {
            sqlite3_bind_int64(_stmt, _index, _value->integer());
            break;
        }
        case zpt::JSDate: {
            auto [_bytes, _size] = zpt::storage::sqlite::to_byte_array(_value);
            sqlite3_bind_text(_stmt, _index, _bytes, _size, zpt::storage::sqlite::free_byte_array);
            break;
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            sqlite3_bind_null(_stmt, _index);
            break;
        }
        case zpt::JSLambda:
        case zpt::JSRegex: {
            break;
        }
    }
}

zpt::storage::sqlite::connection::connection(zpt::json _options)
  : __options(_options["storage"]["sqlite"]) {}

auto zpt::storage::sqlite::connection::open(zpt::json _options) -> zpt::storage::connection::type* {
    this->__options = _options;
    return this;
}

auto zpt::storage::sqlite::connection::close() -> zpt::storage::connection::type* { return this; }

auto zpt::storage::sqlite::connection::session() -> zpt::storage::session {
    return zpt::make_session<zpt::storage::sqlite::session>(*this);
}

auto zpt::storage::sqlite::connection::options() -> zpt::json& { return this->__options; }

zpt::storage::sqlite::session::session(zpt::storage::sqlite::connection& _connection)
  : __underlying{ nullptr }
  , __options{ _connection.__options } {}

auto zpt::storage::sqlite::session::is_open() -> bool { return this->__underlying.size() != 0; }

auto zpt::storage::sqlite::session::commit() -> zpt::storage::session::type* {
    std::string _to_execute{ "commit" };
    for (auto _db : this->__underlying) {
        sqlite3_stmt* _stmt{ nullptr };
        sqlite_expect(
          sqlite3_prepare_v2(_db.get(), _to_execute.data(), _to_execute.length(), &_stmt, nullptr),
          "unable to prepare statement for commit: " << sqlite3_errmsg(_db.get()));
        sqlite_expect(sqlite3_step(_stmt),
                      "unable to execute commit statement: " << sqlite3_errmsg(_db.get()));
        sqlite_expect(sqlite3_finalize(_stmt), "unable to cleanup statement: " << sqlite3_errmsg(_db.get()));
    }
    return this;
}

auto zpt::storage::sqlite::session::rollback() -> zpt::storage::session::type* {
    std::string _to_execute{ "rollback" };
    for (auto _db : this->__underlying) {
        sqlite3_stmt* _stmt{ nullptr };
        sqlite_expect(
          sqlite3_prepare_v2(_db.get(), _to_execute.data(), _to_execute.length(), &_stmt, nullptr),
          "unable to prepare statement for rollback: " << sqlite3_errmsg(_db.get()));
        sqlite_expect(sqlite3_step(_stmt),
                      "unable to execute rollback statement: " << sqlite3_errmsg(_db.get()));
        sqlite_expect(sqlite3_finalize(_stmt), "unable to cleanup statement: " << sqlite3_errmsg(_db.get()));
    }
    return this;
}

auto zpt::storage::sqlite::session::sql(std::string const& _statement) -> zpt::storage::session::type* {
    expect(false, "Session `sql` method not implemented for SQLite, use database's");
    return this;
}

auto zpt::storage::sqlite::session::database(std::string const& _db) -> zpt::storage::database {
    zlog(this->__options("path"), zpt::info);
    return zpt::make_database<zpt::storage::sqlite::database>(*this, _db);
}

auto zpt::storage::sqlite::session::add_database_connection(sqlite3_ptr _database) -> void {
    this->__underlying.push_back(_database);
}

zpt::storage::sqlite::database::database(zpt::storage::sqlite::session& _session, std::string const& _db)
  : __path{ _session.__options("path")->ok()
              ? _session.__options("path")->string() + std::string{ "/" } + _db
              : std::string{ "file:" } + _db + std::string{ "?mode=memory&cache=shared" } } {
    sqlite3* _underlying{ nullptr };
    sqlite_expect(sqlite3_open(this->__path.data(), &_underlying),
                  "couldn't open database at " << this->__path);
    this->__underlying.reset(_underlying, zpt::storage::sqlite::close_connection{});
    _session.add_database_connection(this->__underlying);
}

auto zpt::storage::sqlite::database::sql(std::string const& _to_execute) -> zpt::storage::database::type* {
    sqlite3_stmt* _stmt{ nullptr };
    sqlite_expect(
      sqlite3_prepare_v2(this->__underlying.get(), _to_execute.data(), _to_execute.length(), &_stmt, nullptr),
      "unable to prepare statement for commit: " << sqlite3_errmsg(this->__underlying.get()));
    sqlite_expect(sqlite3_step(_stmt),
                  "unable to execute commit statement: " << sqlite3_errmsg(this->__underlying.get()));
    sqlite_expect(sqlite3_finalize(_stmt),
                  "unable to cleanup statement: " << sqlite3_errmsg(this->__underlying.get()));
    return this;
}

auto zpt::storage::sqlite::database::connection() -> sqlite3_ptr { return this->__underlying; }

auto zpt::storage::sqlite::database::path() -> std::string& { return this->__path; }

auto zpt::storage::sqlite::database::collection(std::string const& _collection) -> zpt::storage::collection {
    return zpt::make_collection<zpt::storage::sqlite::collection>(*this, _collection);
}

zpt::storage::sqlite::collection::collection(zpt::storage::sqlite::database& _database,
                                             std::string const& _collection)
  : __underlying{ _database.__underlying }
  , __collection_name{ _collection } {}

auto zpt::storage::sqlite::collection::add(zpt::json _document) -> zpt::storage::action {
    return zpt::make_action<zpt::storage::sqlite::action_add>(*this, _document);
}

auto zpt::storage::sqlite::collection::modify(zpt::json _search) -> zpt::storage::action {
    return zpt::make_action<zpt::storage::sqlite::action_modify>(*this, _search);
}

auto zpt::storage::sqlite::collection::remove(zpt::json _search) -> zpt::storage::action {
    return zpt::make_action<zpt::storage::sqlite::action_remove>(*this, _search);
}

auto zpt::storage::sqlite::collection::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action {
    return zpt::make_action<zpt::storage::sqlite::action_replace>(*this, _id, _document);
}

auto zpt::storage::sqlite::collection::find(zpt::json _search) -> zpt::storage::action {
    return zpt::make_action<zpt::storage::sqlite::action_find>(*this, _search);
}

auto zpt::storage::sqlite::collection::count() -> size_t {
    std::ostringstream _oss;
    _oss << "select count(*) from \"" << this->__collection_name << "\"" << std::flush;
    std::string _to_execute{ _oss.str() };
    sqlite3_stmt* _stmt{ nullptr };
    sqlite_expect(
      sqlite3_prepare_v2(this->__underlying.get(), _to_execute.data(), _to_execute.length(), &_stmt, nullptr),
      "unable to prepare statement for commit: " << sqlite3_errmsg(this->__underlying.get()));
    sqlite_expect(sqlite3_step(_stmt),
                  "unable to execute commit statement: " << sqlite3_errmsg(this->__underlying.get()));
    zpt::json _count = zpt::storage::sqlite::from_db_doc(_stmt);
    sqlite_expect(sqlite3_finalize(_stmt),
                  "unable to cleanup statement: " << sqlite3_errmsg(this->__underlying.get()));
    return _count["count(*)"];
}

zpt::storage::sqlite::action::action(zpt::storage::sqlite::collection& _collection)
  : __collection_name{ _collection.__collection_name }
  , __underlying{ _collection.__underlying } {}

auto zpt::storage::sqlite::action::set_state(int _error) -> void {
    if (_error != 0) {
        this->__state = { "code",
                          _error,
                          "message",
                          std::get<0>(__messages[_error]) + std::string{ ": " } +
                            std::get<1>(__messages[_error]) };
    }
}

auto zpt::storage::sqlite::action::get_state() -> zpt::json { return this->__state; }

auto zpt::storage::sqlite::action::prepare(std::string const& _statement) -> void {
    sqlite3_stmt* _stmt{ nullptr };
    sqlite_expect(
      sqlite3_prepare_v2(this->__underlying.get(), _statement.data(), _statement.length(), &_stmt, nullptr),
      "unable to prepare statement for commit: " << sqlite3_errmsg(this->__underlying.get()));
    this->__prepared.push_back(sqlite3_stmt_ptr{ _stmt, zpt::storage::sqlite::finalize_statement{} });
}

zpt::storage::sqlite::action_add::action_add(zpt::storage::sqlite::collection& _collection,
                                             zpt::json _document)
  : zpt::storage::sqlite::action::action{ _collection } {
    this->add(_document);
}

auto zpt::storage::sqlite::action_add::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(_document->is_object(), "expected add parameter to be a JSON object");
    if (!_document["_id"]->ok()) {
        std::string _id{ zpt::generate::r_uuid() };
        _document << "_id" << _id;
        this->__generated_uuid << _id;
    }
    this->add_insert(_document);
    return this;
}

auto zpt::storage::sqlite::action_add::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from an 'add' action");
    return this;
}

auto zpt::storage::sqlite::action_add::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from an 'add' action");
    return this;
}

auto zpt::storage::sqlite::action_add::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from an 'add' action");
    return this;
}

auto zpt::storage::sqlite::action_add::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from an 'add' action");
    return this;
}

auto zpt::storage::sqlite::action_add::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    expect(false, "can't set from an 'add' action");
    return this;
}

auto zpt::storage::sqlite::action_add::unset(std::string const& _attribute) -> zpt::storage::action::type* {
    expect(false, "can't unset from an 'add' action");
    return this;
}

auto zpt::storage::sqlite::action_add::patch(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't patch from an 'add' action");
    return this;
}

auto zpt::storage::sqlite::action_add::sort(std::string const& _attribute) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_add::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_add::offset(size_t _rows) -> zpt::storage::action::type* { return this; }

auto zpt::storage::sqlite::action_add::limit(size_t _number) -> zpt::storage::action::type* { return this; }

auto zpt::storage::sqlite::action_add::bind(zpt::json _map) -> zpt::storage::action::type* { return this; }

auto zpt::storage::sqlite::action_add::execute() -> zpt::storage::result {
    try {
        for (auto _prepared : this->__prepared) {
            sqlite_expect(
              sqlite3_step(_prepared.get()),
              "unable to execute prepared statement: " << sqlite3_errmsg(this->__underlying.get()));
        }
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        throw;
    }
    zpt::json _result{ "state", this->get_state(), "generated", this->__generated_uuid };
    return zpt::make_result<zpt::storage::sqlite::result>(_result);
}

auto zpt::storage::sqlite::action_add::add_insert(zpt::json _document) -> void {
    if (_document->size() == 0) { return; }

    std::ostringstream _names;
    std::ostringstream _values;
    _names << "insert into \"" << this->__collection_name << "\" (" << std::flush;
    _values << " values (" << std::flush;
    bool _first{ true };
    for (auto [_, _key, _value] : _document) {
        if (!_first) {
            _names << ", ";
            _values << ", ";
        }
        else { _first = false; }
        _names << "\"" << _key << "\"" << std::flush;
        if (_value->is_string() || _value->is_object() || _value->is_array()) {
            _values << "'" << static_cast<std::string>(_value) << "'" << std::flush;
        }
        else { _values << _value << std::flush; }
    }
    _names << ")" << std::flush;
    _values << ")" << std::flush;
    _names << _values.str() << ";" << std::flush;
    this->prepare(_names.str());
}

zpt::storage::sqlite::action_modify::action_modify(zpt::storage::sqlite::collection& _collection,
                                                   zpt::json _search)
  : zpt::storage::sqlite::action::action{ _collection }
  , __search{ _search }
  , __set{ zpt::json::object() }
  , __unset{ zpt::json::object() } {}

auto zpt::storage::sqlite::action_modify::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'modify' action");
    return this;
}

auto zpt::storage::sqlite::action_modify::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'modify' action");
    return this;
}

auto zpt::storage::sqlite::action_modify::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'modify' action");
    return this;
}

auto zpt::storage::sqlite::action_modify::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'modify' action");
    return this;
}

auto zpt::storage::sqlite::action_modify::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'modify' action");
    return this;
}

auto zpt::storage::sqlite::action_modify::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    this->__set << _attribute << _value;
    return this;
}

auto zpt::storage::sqlite::action_modify::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    this->__unset << _attribute << true;
    return this;
}

auto zpt::storage::sqlite::action_modify::patch(zpt::json _document) -> zpt::storage::action::type* {
    for (auto [_, _key, _member] : _document) { this->__set << _key << _member; }
    return this;
}

auto zpt::storage::sqlite::action_modify::sort(std::string const& _attribute) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_modify::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_modify::offset(size_t _rows) -> zpt::storage::action::type* { return this; }

auto zpt::storage::sqlite::action_modify::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_modify::bind(zpt::json _map) -> zpt::storage::action::type* {
    expect(_map->is_object(), "expected binding map to be a JSON object");
    try {
        this->add_update();
        for (auto _prepared : this->__prepared) {
            for (auto [_, _name, _value] : _map) {
                zpt::storage::sqlite::bind(_prepared.get(), _name, _value);
            }
        }
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        throw;
    }
    return this;
}

auto zpt::storage::sqlite::action_modify::execute() -> zpt::storage::result {
    try {
        this->add_update();
        for (auto _prepared : this->__prepared) {
            auto _result = sqlite3_step(_prepared.get());
            sqlite_expect(
              _result, "unable to execute prepared statement: " << sqlite3_errmsg(this->__underlying.get()));
        }
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        throw;
    }
    zpt::json _result{ "state", this->get_state() };
    return zpt::make_result<zpt::storage::sqlite::result>(_result);
}

auto zpt::storage::sqlite::action_modify::add_update() -> void {
    if (this->__set->size() == 0 && this->__unset->size() == 0) { return; }

    std::ostringstream _oss;
    _oss << "update \"" << this->__collection_name << "\" set " << std::flush;
    bool _first{ true };
    for (auto [_, _key, _value] : this->__set) {
        if (!_first) { _oss << ", "; }
        else { _first = false; }
        _oss << "\"" << _key << "\" = " << std::flush;
        if (_value->is_string() || _value->is_object() || _value->is_array()) {
            _oss << "'" << static_cast<std::string>(_value) << "'" << std::flush;
        }
        else { _oss << _value << std::flush; }
    }
    this->__set->clear();
    for (auto [_, _key, _value] : this->__unset) {
        if (!_first) { _oss << ", "; }
        else { _first = false; }
        _oss << "\"" << _key << "\" = NULL " << std::flush;
    }
    this->__unset->clear();
    std::string _where = zpt::storage::extract_find(this->__search);
    if (_where.length() != 0) { _oss << " where " << _where << std::flush; }
    this->prepare(_oss.str());
}

zpt::storage::sqlite::action_remove::action_remove(zpt::storage::sqlite::collection& _collection,
                                                   zpt::json _search)
  : zpt::storage::sqlite::action::action{ _collection }
  , __search{ _search } {}

auto zpt::storage::sqlite::action_remove::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'remove' action");
    return this;
}

auto zpt::storage::sqlite::action_remove::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'remove' action");
    return this;
}

auto zpt::storage::sqlite::action_remove::remove(zpt::json _search) -> zpt::storage::action::type* {
    this->__search += _search;
    return this;
}

auto zpt::storage::sqlite::action_remove::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'remove' action");
    return this;
}

auto zpt::storage::sqlite::action_remove::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'remove' action");
    return this;
}

auto zpt::storage::sqlite::action_remove::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_remove::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_remove::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_remove::sort(std::string const& _attribute) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_remove::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_remove::offset(size_t _rows) -> zpt::storage::action::type* { return this; }

auto zpt::storage::sqlite::action_remove::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_remove::bind(zpt::json _map) -> zpt::storage::action::type* {
    expect(_map->is_object(), "expected binding map to be a JSON object");
    try {
        this->add_delete();
        for (auto _prepared : this->__prepared) {
            for (auto [_, _name, _value] : _map) {
                zpt::storage::sqlite::bind(_prepared.get(), _name, _value);
            }
        }
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        throw;
    }
    return this;
}

auto zpt::storage::sqlite::action_remove::execute() -> zpt::storage::result {
    try {
        this->add_delete();
        for (auto _prepared : this->__prepared) {
            sqlite_expect(
              sqlite3_step(_prepared.get()),
              "unable to execute prepared statement: " << sqlite3_errmsg(this->__underlying.get()));
        }
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        throw;
    }
    zpt::json _result{ "state", this->get_state() };
    return zpt::make_result<zpt::storage::sqlite::result>(_result);
}

auto zpt::storage::sqlite::action_remove::add_delete() -> void {
    if (this->__added) { return; }
    std::ostringstream _oss;
    _oss << "delete from \"" << this->__collection_name << "\"" << std::flush;
    std::string _where = zpt::storage::extract_find(this->__search);
    if (_where.length() != 0) { _oss << " where " << _where << std::flush; }
    this->prepare(_oss.str());
    this->__added = true;
}

zpt::storage::sqlite::action_replace::action_replace(zpt::storage::sqlite::collection& _collection,
                                                     std::string _id,
                                                     zpt::json _document)
  : zpt::storage::sqlite::action::action{ _collection }
  , __id{ _id }
  , __set{ _document } {
    this->__set << "_id" << _id;
}

auto zpt::storage::sqlite::action_replace::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'replace' action");
    return this;
}

auto zpt::storage::sqlite::action_replace::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'replace' action");
    return this;
}

auto zpt::storage::sqlite::action_replace::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'replace' action");
    return this;
}

auto zpt::storage::sqlite::action_replace::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'replace' action");
    return this;
}

auto zpt::storage::sqlite::action_replace::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'replace' action");
    return this;
}

auto zpt::storage::sqlite::action_replace::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_replace::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_replace::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_replace::sort(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_replace::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_replace::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_replace::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_replace::bind(zpt::json _map) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_replace::execute() -> zpt::storage::result {
    try {
        this->add_replace();
        for (auto _prepared : this->__prepared) {
            sqlite_expect(
              sqlite3_step(_prepared.get()),
              "unable to execute prepared statement: " << sqlite3_errmsg(this->__underlying.get()));
        }
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        throw;
    }
    zpt::json _result{ "state", this->get_state() };
    return zpt::make_result<zpt::storage::sqlite::result>(_result);
}

auto zpt::storage::sqlite::action_replace::add_replace() -> void {
    if (this->__set->size() == 0) { return; }

    std::ostringstream _names;
    std::ostringstream _values;
    _names << "replace into \"" << this->__collection_name << "\" (" << std::flush;
    _values << " values (" << std::flush;
    bool _first{ true };
    for (auto [_, _key, _value] : this->__set) {
        if (!_first) {
            _names << ", ";
            _values << ", ";
        }
        else { _first = false; }
        _names << "\"" << _key << "\"" << std::flush;
        if (_value->is_string() || _value->is_object() || _value->is_array()) {
            _values << "'" << static_cast<std::string>(_value) << "'" << std::flush;
        }
        else { _values << _value << std::flush; }
    }
    _names << ")" << std::flush;
    _values << ")" << std::flush;
    _names << _values.str() << ";" << std::flush;
    this->prepare(_names.str());
}

auto zpt::storage::sqlite::action_replace::replace_one() -> void { this->execute(); }

zpt::storage::sqlite::action_find::action_find(zpt::storage::sqlite::collection& _collection)
  : zpt::storage::sqlite::action::action{ _collection }
  , __search{ zpt::json::object() }
  , __sort{ zpt::json::array() }
  , __fields{ zpt::json::array() } {}

zpt::storage::sqlite::action_find::action_find(zpt::storage::sqlite::collection& _collection,
                                               zpt::json _search)
  : zpt::storage::sqlite::action::action{ _collection }
  , __search{ _search }
  , __sort{ zpt::json::array() }
  , __fields{ zpt::json::array() } {}

auto zpt::storage::sqlite::action_find::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'find' action");
    return this;
}

auto zpt::storage::sqlite::action_find::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'find' action");
    return this;
}

auto zpt::storage::sqlite::action_find::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'find' action");
    return this;
}

auto zpt::storage::sqlite::action_find::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'find' action");
    return this;
}

auto zpt::storage::sqlite::action_find::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'find' action");
    return this;
}

auto zpt::storage::sqlite::action_find::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_find::unset(std::string const& _attribute) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_find::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::sqlite::action_find::sort(std::string const& _attribute) -> zpt::storage::action::type* {
    this->__sort << _attribute;
    return this;
}

auto zpt::storage::sqlite::action_find::fields(zpt::json _fields) -> zpt::storage::action::type* {
    this->__fields += _fields;
    return this;
}

auto zpt::storage::sqlite::action_find::offset(size_t _rows) -> zpt::storage::action::type* {
    this->__offset = _rows;
    return this;
}

auto zpt::storage::sqlite::action_find::limit(size_t _number) -> zpt::storage::action::type* {
    this->__limit = _number;
    return this;
}

auto zpt::storage::sqlite::action_find::bind(zpt::json _map) -> zpt::storage::action::type* {
    expect(_map->is_object(), "expected binding map to be a JSON object");
    try {
        this->add_select();
        for (auto _prepared : this->__prepared) {
            for (auto [_, _name, _value] : _map) {
                zpt::storage::sqlite::bind(_prepared.get(), _name, _value);
            }
        }
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        throw;
    }
    return this;
}

auto zpt::storage::sqlite::action_find::execute() -> zpt::storage::result {
    try {
        this->add_select();
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        throw;
    }
    auto _found = zpt::json::array();
    zpt::json _result{ "state", this->get_state(), "cursor", _found };
    return zpt::make_result<zpt::storage::sqlite::result>(_result, this->__prepared);
}

auto zpt::storage::sqlite::action_find::add_select() -> void {
    if (this->__already_prepared) return;

    std::ostringstream _oss;
    _oss << "select ";

    if (this->__fields->size() != 0 && this->__fields->is_array()) {
        bool _first{ true };
        for (auto [_, __, _value] : this->__fields) {
            if (!_first) { _oss << ", "; }
            else { _first = false; }
            _oss << _value;
        }
    }
    else { _oss << "*"; }

    _oss << " from \"" << this->__collection_name << "\"" << std::flush;

    if (this->__search->ok()) {
        std::string _where = zpt::storage::extract_find(this->__search);
        if (_where.length() != 0) { _oss << " where " << _where << std::flush; }
    }

    if (this->__limit != std::numeric_limits<size_t>::max()) {
        _oss << " limit " << this->__limit << std::flush;
    }

    if (this->__offset != 0) { _oss << " offset " << this->__offset << std::flush; }

    if (this->__sort->size() != 0 && this->__sort->is_object()) {
        _oss << " order by ";
        bool _first{ true };
        zpt::json _order;
        for (auto [_, _key, _value] : this->__sort) {
            if (!_first) { _oss << ", "; }
            else { _first = false; }
            _oss << "\"" << _key << "\"";
            _order = _value;
        }
        _oss << " " << _order << std::flush;
    }

    this->prepare(_oss.str());
    this->__already_prepared = true;
}

zpt::storage::sqlite::result::result(zpt::json _result)
  : __result{ _result } {}

zpt::storage::sqlite::result::result(zpt::json _result, std::vector<sqlite3_stmt_ptr> _prepared)
  : __result{ _result }
  , __prepared{ _prepared } {}

auto zpt::storage::sqlite::result::fetch(size_t _amount) -> zpt::json {
    if (_amount == 0) { _amount = std::numeric_limits<size_t>::max(); }
    auto _return = zpt::json::array();
    for (auto _prepared : this->__prepared) {
        for (size_t _idx = 0; _idx != _amount; ++_idx) {
            if (sqlite3_step(_prepared.get()) != SQLITE_ROW) { break; }
            auto _row = zpt::storage::sqlite::from_db_doc(_prepared.get());
            if (_amount == 1) { return _row; }
            _return << _row;
        }
    }
    zlog(_return->size(), zpt::info);
    return (_return->size() != 0 ? _return : zpt::undefined);
}

auto zpt::storage::sqlite::result::generated_id() -> zpt::json { return this->__result["generated"]; }

auto zpt::storage::sqlite::result::count() -> size_t {
    if (this->__result["generated"]->ok()) { return this->__result["generated"]->size(); }
    return 0;
}

auto zpt::storage::sqlite::result::status() -> zpt::status {
    return static_cast<size_t>(this->__result["state"]["code"]);
}

auto zpt::storage::sqlite::result::message() -> std::string {
    return this->__result["state"]["message"]->string();
}

auto zpt::storage::sqlite::result::to_json() -> zpt::json { return this->__result; }
