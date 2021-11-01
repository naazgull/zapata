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
#include <algorithm>

#define sqlite_expect(_error, _message)                                                            \
    expect(_error == SQLITE_OK || _error == SQLITE_DONE || error == SQLITE_ROW,                    \
           std::get<0>(__messages[_error])                                                         \
             << ": " << _message << " due to: " << std::get<1>(__messages[_error]),                \
           500,                                                                                    \
           _error);

std::map<int, std::tuple<std::string, std::string>> __messages = {
    { SQLITE_OK, { "SQLITE_OK", "Successful result." } },
    { SQLITE_ERROR, { "SQLITE_ERROR", "Generic error." } },
    { SQLITE_INTERNAL, { "SQLITE_INTERNAL", "Internal logic error in SQLite." } },
    { SQLITE_PERM, { "SQLITE_PERM", "Access permission denied." } },
    { SQLITE_ABORT, { "SQLITE_ABORT", "Callback routine requested an abort." } },
    { SQLITE_BUSY, { "SQLITE_BUSY", "The database file is locked." } },
    { SQLITE_LOCKED, { "SQLITE_LOCKED", "A table in the database is locked." } },
    { SQLITE_NOMEM, { "SQLITE_NOMEM", "A malloc() failed." } },
    { SQLITE_READONLY, { "SQLITE_READONLY", "Attempt to write a readonly database." } },
    { SQLITE_INTERRUPT, { "SQLITE_INTERRUPT", "Operation terminated by sqlite3_interrupt(." } },
    { SQLITE_IOERR, { "SQLITE_IOERR", "Some kind of disk I/O error occurred." } },
    { SQLITE_CORRUPT, { "SQLITE_CORRUPT", "The database disk image is malformed." } },
    { SQLITE_NOTFOUND, { "SQLITE_NOTFOUND", "Unknown opcode in sqlite3_file_control()." } },
    { SQLITE_FULL, { "SQLITE_FULL", "Insertion failed because database is full." } },
    { SQLITE_CANTOPEN, { "SQLITE_CANTOPEN", "Unable to open the database file." } },
    { SQLITE_PROTOCOL, { "SQLITE_PROTOCOL", "Database lock protocol error." } },
    { SQLITE_EMPTY, { "SQLITE_EMPTY", "Internal use only." } },
    { SQLITE_SCHEMA, { "SQLITE_SCHEMA", "The database schema changed." } },
    { SQLITE_TOOBIG, { "SQLITE_TOOBIG", "String or BLOB exceeds size limit." } },
    { SQLITE_CONSTRAINT, { "SQLITE_CONSTRAINT", "Abort due to constraint violation." } },
    { SQLITE_MISMATCH, { "SQLITE_MISMATCH", "Data type mismatch." } },
    { SQLITE_MISUSE, { "SQLITE_MISUSE", "Library used incorrectly." } },
    { SQLITE_NOLFS, { "SQLITE_NOLFS", "Uses OS features not supported on host." } },
    { SQLITE_AUTH, { "SQLITE_AUTH", "Authorization denied." } },
    { SQLITE_FORMAT, { "SQLITE_FORMAT", "Not used." } },
    { SQLITE_RANGE, { "SQLITE_RANGE", "2nd parameter to sqlite3_bind out of range." } },
    { SQLITE_NOTADB, { "SQLITE_NOTADB", "File opened that is not a database file." } },
    { SQLITE_NOTICE, { "SQLITE_NOTICE", "Notifications from sqlite3_log()." } },
    { SQLITE_WARNING, { "SQLITE_WARNING", "Warnings from sqlite3_log()." } },
    { SQLITE_ROW, { "SQLITE_ROW", "sqlite3_step() has another row ready." } },
    { SQLITE_DONE, { "SQLITE_DONE", "sqlite3_step() has finished executing." } },
};

auto
zpt::storage::sqlite::to_db_key(zpt::json _document) -> std::string {
    return _document["_id"]->string();
}

auto
zpt::storage::sqlite::to_db_doc(zpt::json _document) -> std::string {
    return static_cast<std::string>(_document);
}

auto
zpt::storage::sqlite::from_db_doc(std::string const& _document) -> zpt::json {
    return zpt::json::parse_json_str(_document);
}

auto
zpt::storage::sqlite::prepare_insert(prepared_list _prepared,
                                     std::string const& _table_name,
                                     zpt::json _document) -> void {
}

zpt::storage::sqlite::connection::connection(zpt::json _options)
  : __options(_options["storage"]["sqlite"]) {}

auto
zpt::storage::sqlite::connection::open(zpt::json _options) -> zpt::storage::connection::type* {
    this->__options = _options;
    return this;
}

auto
zpt::storage::sqlite::connection::close() -> zpt::storage::connection::type* {
    sqlite_expect(sqlite3_close(this->__underlying),
                  "unable to properly close database handle for "
                    << this->__options["storage"]["sqlite"]["path"]);
    return this;
}

auto
zpt::storage::sqlite::connection::session() -> zpt::storage::session {
    return zpt::storage::session::alloc<zpt::storage::sqlite::session>(*this);
}

auto
zpt::storage::sqlite::connection::options() -> zpt::json& {
    return this->__options;
}

zpt::storage::sqlite::session::session(zpt::storage::sqlite::connection& _connection)
  : __options{ _connection.options() }
  , __underlying{ *_connection.__underlying } {}

auto
zpt::storage::sqlite::session::is_open() -> bool {
    return this->__underlying != nullptr;
}

auto
zpt::storage::sqlite::session::commit() -> zpt::storage::session::type* {
    std::string _to_execute{ "COMMIT" };
    sqlite3_stmt* _stmt{ nullptr };
    sqlite_expect(
      sqlite3_prepare_v2(
        this->__underlying.get(), _to_execute.data(), _to_execute.length(), &_stmt, nullptr),
      "unable to prepare statement for commit");
    sqlite_expect(sqlite3_step(_stmt), "unable to execute commit statement");
    sqlite_expect(sqlite3_finalize(_stmt), "unable to cleanup statement");
    return this;
}

auto
zpt::storage::sqlite::session::rollback() -> zpt::storage::session::type* {
    std::string _to_execute{ "ROLLBACK" };
    sqlite3_stmt* _stmt{ nullptr };
    sqlite_expect(
      sqlite3_prepare_v2(
        this->__underlying.get(), _to_execute.data(), _to_execute.length(), &_stmt, nullptr),
      "unable to prepare statement for rollback");
    sqlite_expect(sqlite3_step(_stmt), "unable to execute rollback statement");
    sqlite_expect(sqlite3_finalize(_stmt), "unable to cleanup statement");
    return this;
}

auto
zpt::storage::sqlite::session::database(std::string const& _db) -> zpt::storage::database {
    return zpt::storage::database::alloc<zpt::storage::sqlite::database>(*this, _db);
}

zpt::storage::sqlite::database::database(zpt::storage::sqlite::session& _session,
                                         std::string const& _db)
  : __path{ _session.__options["path"]->string() + std::string{ "/" } + _db } {
    sqlite3* _underlying{ nullptr };
    sqlite_expect(sqlite3_open(this->__path.data(), &_underlying),
                  "couldn't open collection at " << this->__path);
    this->__underlying.reset(_underlying, zpt::storage::sqlite::close_connection{});
    _session.__underlying = this->__underlying;
}

auto
zpt::storage::sqlite::database::path() -> std::string& {
    return this->__path;
}

auto
zpt::storage::sqlite::database::collection(std::string const& _collection)
  -> zpt::storage::collection {
    return zpt::storage::collection::alloc<zpt::storage::sqlite::collection>(*this, _collection);
}

zpt::storage::sqlite::collection::collection(zpt::storage::sqlite::database& _database,
                                             std::string const& _collection)
  : __underlying{ _database.__underlying }
  , __collection_name{ _collection } {}

auto
zpt::storage::sqlite::collection::add(zpt::json _document) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::sqlite::action_add>(*this, _document);
}

auto
zpt::storage::sqlite::collection::modify(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::sqlite::action_modify>(*this, _search);
}

auto
zpt::storage::sqlite::collection::remove(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::sqlite::action_remove>(*this, _search);
}

auto
zpt::storage::sqlite::collection::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::sqlite::action_replace>(*this, _id, _document);
}

auto
zpt::storage::sqlite::collection::find(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::sqlite::action_find>(*this, _search);
}

auto
zpt::storage::sqlite::collection::count() -> size_t {
    auto _count{ 0 };
    return _count;
}

auto
zpt::storage::sqlite::action::action(zpt::storage::sqlite::collection& _collection)
  : __underlying{ _collection.__underlying }
  , __collection_name{ _collection.__collection_name } {}

auto
zpt::storage::sqlite::action::set_state(int _error) -> void {
    if (_error != 0) {
        this->__state = { "code",
                          _error,
                          "message",
                          std::get<0>(__messages[_error]) + std::string{ ": " } +
                            std::get<1>(__messages[_error]) };
    }
}

auto
zpt::storage::sqlite::action::get_state() -> zpt::json {
    return this->__state;
}

zpt::storage::sqlite::action_add::action_add(zpt::storage::sqlite::collection& _collection,
                                             zpt::json _document)
  : zpt::storage::sqlite::action::action{ _collection } {
    this->add(_document);
}

auto
zpt::storage::sqlite::action_add::add(zpt::json _document) -> zpt::storage::action::type* {
    if (!_document["_id"]->ok()) {
        std::string _id{ zpt::generate::r_uuid() };
        _document << "_id" << _id;
        this->__generated_uuid << _id;
    }
    this->__to_add << _document;
    zpt::storage::sqlite::prepare_insert(this->__prepared, this->__collection_name, _document);
    return this;
}

auto
zpt::storage::sqlite::action_add::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_add::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_add::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_add::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_add::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    expect(false, "can't set from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_add::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    expect(false, "can't unset from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_add::patch(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't patch from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_add::sort(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_add::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_add::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_add::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_add::bind(zpt::json _map) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_add::execute() -> zpt::storage::result {
    try {
        for (auto _stmt : this->__prepared) {
            sqlite_expect(sqlite3_step(_stmt), "unable to execute prepared statement for '...'");
        }
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(_e.code());
        throw;
    }
    zpt::json _result{ "state", this->get_state(), "generated", this->__generated_uuid };
    return zpt::storage::result::alloc<zpt::storage::sqlite::result>(_result);
}

zpt::storage::sqlite::action_modify::action_modify(zpt::storage::sqlite::collection& _collection,
                                                   zpt::json _search)
  : zpt::storage::sqlite::action::action{ _collection }
  , __search{ _search }
  , __set{ zpt::json::object() }
  , __unset{ zpt::json::object() } {}

auto
zpt::storage::sqlite::action_modify::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_modify::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_modify::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_modify::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_modify::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_modify::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    this->__set << _attribute << _value;
    return this;
}

auto
zpt::storage::sqlite::action_modify::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    this->__unset << _attribute << true;
    return this;
}

auto
zpt::storage::sqlite::action_modify::patch(zpt::json _document) -> zpt::storage::action::type* {
    for (auto [_, _key, _member] : _document) { this->__set << _key << _member; }
    return this;
}

auto
zpt::storage::sqlite::action_modify::sort(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_modify::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_modify::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_modify::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_modify::bind(zpt::json _map) -> zpt::storage::action::type* {
    auto _transform =
      [&](std::string const& _key, zpt::json _item, std::string const& _path) -> void {
        if (_item->type() != zpt::JSString) { return; }
        for (auto [_, _key, _value] : _map) {
            if (_item == _key) { _item << _value; }
        }
    };
    zpt::json::traverse(this->__set, _transform);
    zpt::json::traverse(this->__search, _transform);
    return this;
}

auto
zpt::storage::sqlite::action_modify::execute() -> zpt::storage::result {
    size_t _count{ 0 };
    MDB_txn* _trx{ nullptr };
    MDB_dbi _dbi{ 0 };
    MDB_cursor* _cursor{ nullptr };

    try {
        auto _error = mdb_txn_begin(this->__underlying->env(), nullptr, 0, &_trx);
        mdb_expect(_error, "unable to create transaction");

        _error = mdb_dbi_open(_trx, nullptr, 0, &_dbi);
        mdb_expect(_error, "unable to create db handle");

        if (this->__search["_id"]->is_string()) {
            std::string _document_key = zpt::storage::sqlite::to_db_key(this->__search);
            MDB_val _key{ _document_key.length(), _document_key.data() };
            MDB_val _value_f;
            _error = mdb_get(_trx, _dbi, &_key, &_value_f);
            mdb_expect(_error, "unable to find the record");

            auto _doc = zpt::storage::sqlite::from_db_doc(
                          std::string{ static_cast<char*>(_value_f.mv_data), _value_f.mv_size }) +
                        this->__set - this->__unset;
            auto _value = zpt::storage::sqlite::to_db_doc(_doc);
            MDB_val _value_v{ _value.length(), _value.data() };
            _error = mdb_put(_trx, _dbi, &_key, &_value_v, 0);
            mdb_expect(_error, "unable to store record in the database");
            ++_count;
        }
        else {
            _error = mdb_cursor_open(_trx, _dbi, &_cursor);
            mdb_expect(_error, "unable to open cursor");

            do {
                MDB_val _key;
                MDB_val _value_f;
                _error = mdb_cursor_get(_cursor, &_key, &_value_f, MDB_NEXT);
                if (_error != 0 && _error != MDB_NOTFOUND) {
                    mdb_expect(_error, "unable to retrieve from cursor");
                }

                auto _object = zpt::storage::sqlite::from_db_doc(
                  std::string{ static_cast<char*>(_value_f.mv_data), _value_f.mv_size });
                if (!this->is_filtered_out(this->__search, _object)) {
                    auto _doc = _object + this->__set - this->__unset;

                    auto _value = zpt::storage::sqlite::to_db_doc(_doc);
                    MDB_val _value_v{ _value.length(), _value.data() };
                    _error = mdb_put(_trx, _dbi, &_key, &_value_v, 0);
                    mdb_expect(_error, "unable to store record in the database");
                    ++_count;
                }
            } while (_error != MDB_NOTFOUND);
            mdb_cursor_close(_cursor);
        }

        mdb_dbi_close(this->__underlying->env(), _dbi);
        _error = mdb_txn_commit(_trx);
        mdb_expect(_error, "unable to commit transaction");
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(_e.code());
        mdb_cursor_close(_cursor);
        mdb_dbi_close(this->__underlying->env(), _dbi);
        mdb_txn_abort(_trx);
        throw;
    }
    zpt::json _result{ "state", this->get_state(), "modified", _count };
    return zpt::storage::result::alloc<zpt::storage::sqlite::result>(_result);
}

zpt::storage::sqlite::action_remove::action_remove(zpt::storage::sqlite::collection& _collection,
                                                   zpt::json _search)
  : zpt::storage::sqlite::action::action{ _collection }
  , __search{ _search } {}

auto
zpt::storage::sqlite::action_remove::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_remove::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_remove::remove(zpt::json _search) -> zpt::storage::action::type* {
    this->__search += _search;
    return this;
}

auto
zpt::storage::sqlite::action_remove::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_remove::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_remove::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_remove::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_remove::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_remove::sort(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_remove::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_remove::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_remove::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_remove::bind(zpt::json _map) -> zpt::storage::action::type* {
    auto _transform =
      [&](std::string const& _key, zpt::json _item, std::string const& _path) -> void {
        if (_item->type() != zpt::JSString) { return; }
        for (auto [_, _key, _value] : _map) {
            if (_item == _key) { _item << _value; }
        }
    };
    zpt::json::traverse(this->__search, _transform);
    return this;
}

auto
zpt::storage::sqlite::action_remove::execute() -> zpt::storage::result {
    size_t _count{ 0 };
    MDB_txn* _trx{ nullptr };
    MDB_dbi _dbi{ 0 };
    MDB_cursor* _cursor{ nullptr };

    try {
        auto _error = mdb_txn_begin(this->__underlying->env(), nullptr, 0, &_trx);
        mdb_expect(_error, "unable to create transaction");

        _error = mdb_dbi_open(_trx, nullptr, 0, &_dbi);
        mdb_expect(_error, "unable to create db handle");

        if (this->__search["_id"]->is_string()) {
            std::string _document_key = zpt::storage::sqlite::to_db_key(this->__search);
            MDB_val _key{ _document_key.length(), _document_key.data() };
            _error = mdb_del(_trx, _dbi, &_key, nullptr);
            if (_error != 0 && _error != MDB_NOTFOUND) {
                mdb_expect(_error, "unable to remove the record");
            }
            else if (_error == 0) {
                ++_count;
            }
        }
        else {
            _error = mdb_cursor_open(_trx, _dbi, &_cursor);
            mdb_expect(_error, "unable to open cursor");

            do {
                MDB_val _key;
                MDB_val _value_f;

                _error = mdb_cursor_get(_cursor, &_key, &_value_f, MDB_NEXT);
                if (_error != 0 && _error != MDB_NOTFOUND) {
                    mdb_expect(_error, "unable to retrieve from cursor");
                }
                auto _object = zpt::storage::sqlite::from_db_doc(
                  std::string{ static_cast<char*>(_value_f.mv_data), _value_f.mv_size });
                if (!this->is_filtered_out(this->__search, _object)) {
                    _error = mdb_del(_trx, _dbi, &_key, nullptr);
                    if (_error != 0 && _error != MDB_NOTFOUND) {
                        mdb_expect(_error, "unable to remove the record");
                    }
                    else if (_error == 0) {
                        ++_count;
                    }
                }
            } while (_error != MDB_NOTFOUND);
            mdb_cursor_close(_cursor);
        }

        mdb_dbi_close(this->__underlying->env(), _dbi);
        _error = mdb_txn_commit(_trx);
        mdb_expect(_error, "unable to commit transaction");
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(_e.code());
        mdb_cursor_close(_cursor);
        mdb_dbi_close(this->__underlying->env(), _dbi);
        mdb_txn_abort(_trx);
        throw;
    }
    zpt::json _result{ "state", this->get_state(), "removed", _count };
    return zpt::storage::result::alloc<zpt::storage::sqlite::result>(_result);
}

zpt::storage::sqlite::action_replace::action_replace(zpt::storage::sqlite::collection& _collection,
                                                     std::string _id,
                                                     zpt::json _document)
  : zpt::storage::sqlite::action::action{ _collection }
  , __id{ _id }
  , __set{ _document } {}

auto
zpt::storage::sqlite::action_replace::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_replace::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_replace::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_replace::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_replace::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_replace::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_replace::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_replace::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_replace::sort(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_replace::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_replace::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_replace::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_replace::bind(zpt::json _map) -> zpt::storage::action::type* {
    auto _transform =
      [&](std::string const& _key, zpt::json _item, std::string const& _path) -> void {
        if (_item->type() != zpt::JSString) { return; }
        for (auto [_, _key, _value] : _map) {
            if (_item == _key) { _item << _value; }
        }
    };
    zpt::json::traverse(this->__set, _transform);
    return this;
}

auto
zpt::storage::sqlite::action_replace::execute() -> zpt::storage::result {
    size_t _count{ 0 };
    MDB_txn* _trx{ nullptr };
    MDB_dbi _dbi{ 0 };

    try {
        auto _error = mdb_txn_begin(this->__underlying->env(), nullptr, 0, &_trx);
        mdb_expect(_error, "unable to create transaction");

        _error = mdb_dbi_open(_trx, nullptr, 0, &_dbi);
        mdb_expect(_error, "unable to create db handle");

        auto _value = zpt::storage::sqlite::to_db_doc(this->__set);
        MDB_val _key_v{ this->__id.length(), this->__id.data() };
        MDB_val _value_v{ _value.length(), _value.data() };
        _error = mdb_put(_trx, _dbi, &_key_v, &_value_v, 0);
        mdb_expect(_error, "unable to store record in the database");

        mdb_dbi_close(this->__underlying->env(), _dbi);
        _error = mdb_txn_commit(_trx);
        mdb_expect(_error, "unable to commit transaction");
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(_e.code());
        mdb_dbi_close(this->__underlying->env(), _dbi);
        mdb_txn_abort(_trx);
        throw;
    }
    zpt::json _result{ "state", this->get_state(), "replaced", _count };
    return zpt::storage::result::alloc<zpt::storage::sqlite::result>(_result);
}

auto
zpt::storage::sqlite::action_replace::replace_one() -> void {
    this->execute();
}

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

auto
zpt::storage::sqlite::action_find::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_find::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_find::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_find::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_find::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::sqlite::action_find::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_find::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_find::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::sqlite::action_find::sort(std::string const& _attribute)
  -> zpt::storage::action::type* {
    this->__sort << _attribute;
    return this;
}

auto
zpt::storage::sqlite::action_find::fields(zpt::json _fields) -> zpt::storage::action::type* {
    this->__fields += _fields;
    return this;
}

auto
zpt::storage::sqlite::action_find::offset(size_t _rows) -> zpt::storage::action::type* {
    this->__offset = _rows;
    return this;
}

auto
zpt::storage::sqlite::action_find::limit(size_t _number) -> zpt::storage::action::type* {
    this->__limit = _number;
    return this;
}

auto
zpt::storage::sqlite::action_find::bind(zpt::json _map) -> zpt::storage::action::type* {
    auto _transform =
      [&](std::string const& _key, zpt::json _item, std::string const& _path) -> void {
        if (_item->type() != zpt::JSString) { return; }
        for (auto [_, _key, _value] : _map) {
            if (_item == _key) { _item << _value; }
        }
    };
    zpt::json::traverse(this->__search, _transform);
    return this;
}

auto
zpt::storage::sqlite::action_find::execute() -> zpt::storage::result {
    auto _found = zpt::json::array();
    MDB_txn* _trx{ nullptr };
    MDB_dbi _dbi{ 0 };
    MDB_cursor* _cursor{ nullptr };

    MDB_envinfo _stat;
    mdb_env_info(this->__underlying->env(), &_stat);
    try {
        auto _error = mdb_txn_begin(this->__underlying->env(), nullptr, MDB_RDONLY, &_trx);
        mdb_expect(_error, "unable to create transaction");

        _error = mdb_dbi_open(_trx, nullptr, 0, &_dbi);
        mdb_expect(_error, "unable to create db handle");

        if (this->__search["_id"]->is_string()) {
            std::string _document_key = zpt::storage::sqlite::to_db_key(this->__search);
            MDB_val _key{ _document_key.length(), _document_key.data() };
            MDB_val _value_f;
            _error = mdb_get(_trx, _dbi, &_key, &_value_f);
            mdb_expect(_error, "unable to find the record");

            _found << zpt::storage::sqlite::from_db_doc(
              std::string{ static_cast<char*>(_value_f.mv_data), _value_f.mv_size });
        }
        else {
            _error = mdb_cursor_open(_trx, _dbi, &_cursor);
            mdb_expect(_error, "unable to open cursor");
            mdb_cursor_get(_cursor, nullptr, nullptr, MDB_FIRST);

            size_t _consumed{ 0 };
            do {
                MDB_val _key;
                MDB_val _value_f;
                _error = mdb_cursor_get(_cursor, &_key, &_value_f, MDB_NEXT);
                if (_error != 0 && _error != MDB_NOTFOUND) {
                    mdb_expect(_error, "unable to retrieve from cursor");
                }
                ++_consumed;
                if (_error == 0 && _consumed > this->__offset) {
                    _found << zpt::storage::sqlite::from_db_doc(
                      std::string{ static_cast<char*>(_value_f.mv_data), _value_f.mv_size });
                }
            } while (_error == 0 && _found->size() != this->__limit);
            mdb_cursor_close(_cursor);
        }

        mdb_dbi_close(this->__underlying->env(), _dbi);
        mdb_txn_abort(_trx);
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(_e.code());
        mdb_cursor_close(_cursor);
        mdb_dbi_close(this->__underlying->env(), _dbi);
        mdb_txn_abort(_trx);
        throw;
    }
    zpt::json _result{ "state", this->get_state(), "cursor", _found };
    return zpt::storage::result::alloc<zpt::storage::sqlite::result>(_result);
}

zpt::storage::sqlite::result::result(zpt::json _result)
  : __result{ _result }
  , __current{ __result["cursor"].begin() } {}

auto
zpt::storage::sqlite::result::fetch(size_t _amount) -> zpt::json {
    auto _result = zpt::json::array();
    for (size_t _fetched = 0; this->__current != this->__result["cursor"].end();
         ++this->__current, ++_fetched) {
        _result << std::get<2>(*this->__current);
        if (_amount != 0 && _fetched == _amount) { break; }
    }
    return _result;
}

auto
zpt::storage::sqlite::result::generated_id() -> zpt::json {
    return this->__result["generated"];
}

auto
zpt::storage::sqlite::result::count() -> size_t {
    if (this->__result["generated"]->ok()) { return this->__result["generated"]->size(); }
    return this->__result["cursor"]->size();
}

auto
zpt::storage::sqlite::result::status() -> zpt::status {
    return static_cast<size_t>(this->__result["state"]["code"]);
}

auto
zpt::storage::sqlite::result::message() -> std::string {
    return this->__result["state"]["message"]->string();
}

auto
zpt::storage::sqlite::result::to_json() -> zpt::json {
    return this->__result;
}
