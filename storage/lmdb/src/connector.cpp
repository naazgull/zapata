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

#include <zapata/lmdb/connector.h>
#include <algorithm>

#define mdb_expect(_error, _message)                                                               \
    {                                                                                              \
        auto __error__ = _error;                                                                   \
        expect(__error__ == 0,                                                                     \
               std::get<0>(__messages[__error__])                                                  \
                 << ": " << _message << " due to: " << std::get<1>(__messages[__error__]));        \
    }

std::map<int, std::tuple<std::string, std::string>> __messages = {
    { MDB_PANIC,
      { "MDB_PANIC", "a fatal error occurred earlier and the environment must be shut down." } },
    { MDB_MAP_RESIZED,
      { "MDB_MAP_RESIZED",
        "another process wrote data beyond this MDB_env's mapsize and this environment's map must "
        "be resized as well. See mdb_env_set_mapsize()." } },
    { MDB_READERS_FULL,
      { "MDB_READERS_FULL",
        "a read-only transaction was requested and the reader lock table is full. See "
        "mdb_env_set_maxreaders()." } },
    { MDB_NOTFOUND,
      { "MDB_NOTFOUND",
        "the specified database doesn't exist in the environment and MDB_CREATE was not "
        "specified." } },
    { MDB_DBS_FULL,
      { "MDB_DBS_FULL", "too many databases have been opened. See mdb_env_set_maxdbs()." } },
    { MDB_MAP_FULL, { "MDB_MAP_FULL", "the database is full, see mdb_env_set_mapsize()." } },
    { MDB_TXN_FULL, { "MDB_TXN_FULL", "the transaction has too many dirty pages." } },
    { MDB_VERSION_MISMATCH,
      { "MDB_VERSION_MISMATCH",
        "the version of the LMDB library doesn't match the version that created the database "
        "environment." } },
    { MDB_INVALID, { "MDB_INVALID", "the environment file headers are corrupted." } },
    { ENOENT, { "ENOENT", "the directory specified by the path parameter doesn't exist." } },
    { EAGAIN, { "EAGAIN", "the environment was locked by another process." } },
    { ENOSPC, { "ENOSPC", "no more disk space." } },
    { EIO, { "EIO", "a low-level I/O error occurred while writing. " } },
    { EACCES, { "EACCES", "an attempt was made to write in a read-only transaction." } },
    { EINVAL, { "EINVAL", "an invalid parameter was specified." } },
    { ENOMEM, { "ENOMEM", "out of memory." } }
};

auto
zpt::storage::lmdb::to_db_key(zpt::json _document) -> std::string {
    return _document["_id"]->string();
}

auto
zpt::storage::lmdb::to_db_doc(zpt::json _document) -> std::string {
    return static_cast<std::string>(_document);
}

auto
zpt::storage::lmdb::from_db_doc(std::string const& _document) -> zpt::json {
    return zpt::json::parse_json_str(_document);
}

zpt::storage::lmdb::connection::connection(zpt::json _options)
  : __options(_options["storage"]["lmdb"]) {}

auto
zpt::storage::lmdb::connection::open(zpt::json _options) -> zpt::storage::connection::type* {
    this->__options = _options;
    return this;
}

auto
zpt::storage::lmdb::connection::close() -> zpt::storage::connection::type* {
    return this;
}

auto
zpt::storage::lmdb::connection::session() -> zpt::storage::session {
    return zpt::storage::session::alloc<zpt::storage::lmdb::session>(*this);
}

auto
zpt::storage::lmdb::connection::options() -> zpt::json& {
    return this->__options;
}

zpt::storage::lmdb::session::session(zpt::storage::lmdb::connection& _connection)
  : __options{ _connection.options() } {}

auto
zpt::storage::lmdb::session::is_open() -> bool {
    return true;
}

auto
zpt::storage::lmdb::session::commit() -> zpt::storage::session::type* {
    this->__commit = true;
    return this;
}

auto
zpt::storage::lmdb::session::rollback() -> zpt::storage::session::type* {
    this->__commit = false;
    return this;
}

auto
zpt::storage::lmdb::session::is_to_commit() -> bool& {
    return this->__commit;
}

auto
zpt::storage::lmdb::session::database(std::string const& _db) -> zpt::storage::database {
    return zpt::storage::database::alloc<zpt::storage::lmdb::database>(*this, _db);
}

zpt::storage::lmdb::database::database(zpt::storage::lmdb::session& _session,
                                       std::string const& _db)
  : __path{ _session.__options["path"]->string() + std::string{ "/" } + _db }
  , __commit{ _session.is_to_commit() } {}

auto
zpt::storage::lmdb::database::is_to_commit() -> bool& {
    return *this->__commit;
}

auto
zpt::storage::lmdb::database::path() -> std::string& {
    return this->__path;
}

auto
zpt::storage::lmdb::database::collection(std::string const& _collection)
  -> zpt::storage::collection {
    return zpt::storage::collection::alloc<zpt::storage::lmdb::collection>(*this, _collection);
}

zpt::storage::lmdb::collection::collection(zpt::storage::lmdb::database& _database,
                                           std::string const& _collection)
  : __underlying{ nullptr }
  , __collection_name{ _collection }
  , __collection_file{ _database.path() + std::string{ "/" } + _collection + std::string{ ".mdb" } }
  , __commit{ _database.is_to_commit() } {
    auto _error = mdb_env_create(&this->__underlying);
    mdb_expect(_error, "unable to create environment");
    _error = mdb_env_set_mapsize(this->__underlying, 1UL * 1024UL * 1024UL * 1024UL); /* 1 GiB */
    mdb_expect(_error, "unable to set map size");
    _error = mdb_env_open(this->__underlying, this->__collection_file.data(), MDB_NOSUBDIR, 0664);
    mdb_expect(_error, "unable to open environment");
}

zpt::storage::lmdb::collection::~collection() { mdb_env_close(this->__underlying); }

auto
zpt::storage::lmdb::collection::add(zpt::json _document) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::lmdb::action_add>(*this, _document);
}

auto
zpt::storage::lmdb::collection::modify(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::lmdb::action_modify>(*this, _search);
}

auto
zpt::storage::lmdb::collection::remove(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::lmdb::action_remove>(*this, _search);
}

auto
zpt::storage::lmdb::collection::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::lmdb::action_replace>(*this, _id, _document);
}

auto
zpt::storage::lmdb::collection::find(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::lmdb::action_find>(*this, _search);
}

auto
zpt::storage::lmdb::collection::count() -> size_t {
    MDB_txn* _read_trx{ nullptr };
    auto _error = mdb_txn_begin(this->__underlying, nullptr, MDB_RDONLY, &_read_trx);
    mdb_expect(_error, "unable to create transaction");

    MDB_dbi _dbi;
    _error = mdb_dbi_open(_read_trx, nullptr, 0, &_dbi);
    mdb_expect(_error, "unable to create db handle");

    MDB_stat _stat;
    _error = mdb_stat(_read_trx, _dbi, &_stat);
    expect(_error == 0, "unable to retrieve statistics from database");

    auto _count = _stat.ms_entries;

    mdb_dbi_close(this->__underlying, _dbi);
    mdb_txn_abort(_read_trx);
    return _count;
}

auto
zpt::storage::lmdb::collection::file() -> std::string& {
    return this->__collection_file;
}

auto
zpt::storage::lmdb::collection::env() -> MDB_env* {
    return this->__underlying;
}

auto
zpt::storage::lmdb::collection::is_to_commit() -> bool& {
    return *this->__commit;
}

zpt::storage::lmdb::action::action(zpt::storage::lmdb::collection& _collection)
  : __underlying{ _collection } {}

auto
zpt::storage::lmdb::action::is_to_commit() -> bool& {
    return this->__underlying->is_to_commit();
}

auto
zpt::storage::lmdb::action::set_state(int _error) -> void {
    if (_error != 0) {
        this->__state = { "code",
                          _error,
                          "message",
                          std::get<0>(__messages[_error]) + std::string{ ": " } +
                            std::get<1>(__messages[_error]) };
    }
}

auto
zpt::storage::lmdb::action::get_state() -> zpt::json {
    return this->__state;
}

auto
zpt::storage::lmdb::action::is_filtered_out(zpt::json _search, zpt::json _to_filter) -> bool {
    if (_search->size() != 0) {
        for (auto [_, _key, _value] : _search) {
            if (_value != _to_filter[_key]) { return true; }
        }
    }
    return false;
}

auto
zpt::storage::lmdb::action::trim(zpt::json _fields, zpt::json _to_trim) -> zpt::json {
    if (_fields->size() != 0) {
        for (auto [_, __, _field] : _fields) { _to_trim->object()->pop(_field->string()); }
    }
    return _to_trim;
}

zpt::storage::lmdb::action_add::action_add(zpt::storage::lmdb::collection& _collection,
                                           zpt::json _document)
  : zpt::storage::lmdb::action::action{ _collection } {
    this->add(_document);
}

auto
zpt::storage::lmdb::action_add::add(zpt::json _document) -> zpt::storage::action::type* {
    if (!_document["_id"]->ok()) {
        std::string _id{ zpt::generate::r_uuid() };
        this->__generated_uuid << zpt::json{ "_id", _id };
        _document << "_id" << _id;
    }
    this->__to_add << _document;
    return this;
}

auto
zpt::storage::lmdb::action_add::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from an 'add' action");
    return this;
}

auto
zpt::storage::lmdb::action_add::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from an 'add' action");
    return this;
}

auto
zpt::storage::lmdb::action_add::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from an 'add' action");
    return this;
}

auto
zpt::storage::lmdb::action_add::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from an 'add' action");
    return this;
}

auto
zpt::storage::lmdb::action_add::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    expect(false, "can't set from an 'add' action");
    return this;
}

auto
zpt::storage::lmdb::action_add::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    expect(false, "can't unset from an 'add' action");
    return this;
}

auto
zpt::storage::lmdb::action_add::patch(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't patch from an 'add' action");
    return this;
}

auto
zpt::storage::lmdb::action_add::sort(std::string const& _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_add::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_add::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_add::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_add::bind(zpt::json _map) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_add::execute() -> zpt::storage::result {
    MDB_txn* _trx{ nullptr };
    MDB_dbi _dbi{ 0 };
    try {
        auto _error = mdb_txn_begin(this->__underlying->env(), nullptr, 0, &_trx);
        mdb_expect(_error, "unable to create transaction");

        _error = mdb_dbi_open(_trx, nullptr, 0, &_dbi);
        mdb_expect(_error, "unable to create db handle");

        for (auto [_, __, _doc] : this->__to_add) {
            auto _key = zpt::storage::lmdb::to_db_key(_doc);
            auto _value = zpt::storage::lmdb::to_db_doc(_doc);
            MDB_val _key_v{ _key.length(), _key.data() };
            MDB_val _value_v{ _value.length(), _value.data() };
            _error = mdb_put(_trx, _dbi, &_key_v, &_value_v, 0);
            mdb_expect(_error, "unable to store record in the database");
        }

        mdb_dbi_close(this->__underlying->env(), _dbi);
        _error = mdb_txn_commit(_trx);
        mdb_expect(_error, "unable to commit transaction");
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        mdb_dbi_close(this->__underlying->env(), _dbi);
        mdb_txn_abort(_trx);
        throw;
    }
    zpt::json _result{ "state", this->get_state(), "generated", this->__generated_uuid };
    return zpt::storage::result::alloc<zpt::storage::lmdb::result>(_result);
}

zpt::storage::lmdb::action_modify::action_modify(zpt::storage::lmdb::collection& _collection,
                                                 zpt::json _search)
  : zpt::storage::lmdb::action::action{ _collection }
  , __search{ _search }
  , __set{ zpt::json::object() }
  , __unset{ zpt::json::object() } {}

auto
zpt::storage::lmdb::action_modify::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'modify' action");
    return this;
}

auto
zpt::storage::lmdb::action_modify::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'modify' action");
    return this;
}

auto
zpt::storage::lmdb::action_modify::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'modify' action");
    return this;
}

auto
zpt::storage::lmdb::action_modify::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'modify' action");
    return this;
}

auto
zpt::storage::lmdb::action_modify::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'modify' action");
    return this;
}

auto
zpt::storage::lmdb::action_modify::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    this->__set << _attribute << _value;
    return this;
}

auto
zpt::storage::lmdb::action_modify::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    this->__unset << _attribute << true;
    return this;
}

auto
zpt::storage::lmdb::action_modify::patch(zpt::json _document) -> zpt::storage::action::type* {
    for (auto [_, _key, _member] : _document) { this->__set << _key << _member; }
    return this;
}

auto
zpt::storage::lmdb::action_modify::sort(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_modify::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_modify::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_modify::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_modify::bind(zpt::json _map) -> zpt::storage::action::type* {
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
zpt::storage::lmdb::action_modify::execute() -> zpt::storage::result {
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
            std::string _document_key = zpt::storage::lmdb::to_db_key(this->__search);
            MDB_val _key{ _document_key.length(), _document_key.data() };
            MDB_val _value_f;
            _error = mdb_get(_trx, _dbi, &_key, &_value_f);
            mdb_expect(_error, "unable to find the record");

            auto _doc = zpt::storage::lmdb::from_db_doc(
                          std::string{ static_cast<char*>(_value_f.mv_data), _value_f.mv_size }) +
                        this->__set - this->__unset;
            auto _value = zpt::storage::lmdb::to_db_doc(_doc);
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

                auto _object = zpt::storage::lmdb::from_db_doc(
                  std::string{ static_cast<char*>(_value_f.mv_data), _value_f.mv_size });
                if (!this->is_filtered_out(this->__search, _object)) {
                    auto _doc = _object + this->__set - this->__unset;

                    auto _value = zpt::storage::lmdb::to_db_doc(_doc);
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
        this->set_state(-1);
        mdb_cursor_close(_cursor);
        mdb_dbi_close(this->__underlying->env(), _dbi);
        mdb_txn_abort(_trx);
        throw;
    }
    zpt::json _result{ "state", this->get_state(), "modified", _count };
    return zpt::storage::result::alloc<zpt::storage::lmdb::result>(_result);
}

zpt::storage::lmdb::action_remove::action_remove(zpt::storage::lmdb::collection& _collection,
                                                 zpt::json _search)
  : zpt::storage::lmdb::action::action{ _collection }
  , __search{ _search } {}

auto
zpt::storage::lmdb::action_remove::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'remove' action");
    return this;
}

auto
zpt::storage::lmdb::action_remove::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'remove' action");
    return this;
}

auto
zpt::storage::lmdb::action_remove::remove(zpt::json _search) -> zpt::storage::action::type* {
    this->__search += _search;
    return this;
}

auto
zpt::storage::lmdb::action_remove::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'remove' action");
    return this;
}

auto
zpt::storage::lmdb::action_remove::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'remove' action");
    return this;
}

auto
zpt::storage::lmdb::action_remove::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_remove::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_remove::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_remove::sort(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_remove::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_remove::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_remove::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_remove::bind(zpt::json _map) -> zpt::storage::action::type* {
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
zpt::storage::lmdb::action_remove::execute() -> zpt::storage::result {
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
            std::string _document_key = zpt::storage::lmdb::to_db_key(this->__search);
            MDB_val _key{ _document_key.length(), _document_key.data() };
            _error = mdb_del(_trx, _dbi, &_key, nullptr);
            if (_error != 0 && _error != MDB_NOTFOUND) {
                mdb_expect(_error, "unable to remove the record");
            }
            else if (_error == 0) { ++_count; }
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
                auto _object = zpt::storage::lmdb::from_db_doc(
                  std::string{ static_cast<char*>(_value_f.mv_data), _value_f.mv_size });
                if (!this->is_filtered_out(this->__search, _object)) {
                    _error = mdb_del(_trx, _dbi, &_key, nullptr);
                    if (_error != 0 && _error != MDB_NOTFOUND) {
                        mdb_expect(_error, "unable to remove the record");
                    }
                    else if (_error == 0) { ++_count; }
                }
            } while (_error != MDB_NOTFOUND);
            mdb_cursor_close(_cursor);
        }

        mdb_dbi_close(this->__underlying->env(), _dbi);
        _error = mdb_txn_commit(_trx);
        mdb_expect(_error, "unable to commit transaction");
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        mdb_cursor_close(_cursor);
        mdb_dbi_close(this->__underlying->env(), _dbi);
        mdb_txn_abort(_trx);
        throw;
    }
    zpt::json _result{ "state", this->get_state(), "removed", _count };
    return zpt::storage::result::alloc<zpt::storage::lmdb::result>(_result);
}

zpt::storage::lmdb::action_replace::action_replace(zpt::storage::lmdb::collection& _collection,
                                                   std::string _id,
                                                   zpt::json _document)
  : zpt::storage::lmdb::action::action{ _collection }
  , __id{ _id }
  , __set{ _document } {}

auto
zpt::storage::lmdb::action_replace::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'replace' action");
    return this;
}

auto
zpt::storage::lmdb::action_replace::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'replace' action");
    return this;
}

auto
zpt::storage::lmdb::action_replace::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'replace' action");
    return this;
}

auto
zpt::storage::lmdb::action_replace::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'replace' action");
    return this;
}

auto
zpt::storage::lmdb::action_replace::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'replace' action");
    return this;
}

auto
zpt::storage::lmdb::action_replace::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_replace::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_replace::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_replace::sort(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_replace::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_replace::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_replace::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_replace::bind(zpt::json _map) -> zpt::storage::action::type* {
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
zpt::storage::lmdb::action_replace::execute() -> zpt::storage::result {
    size_t _count{ 0 };
    MDB_txn* _trx{ nullptr };
    MDB_dbi _dbi{ 0 };

    try {
        auto _error = mdb_txn_begin(this->__underlying->env(), nullptr, 0, &_trx);
        mdb_expect(_error, "unable to create transaction");

        _error = mdb_dbi_open(_trx, nullptr, 0, &_dbi);
        mdb_expect(_error, "unable to create db handle");

        auto _value = zpt::storage::lmdb::to_db_doc(this->__set);
        MDB_val _key_v{ this->__id.length(), this->__id.data() };
        MDB_val _value_v{ _value.length(), _value.data() };
        _error = mdb_put(_trx, _dbi, &_key_v, &_value_v, 0);
        mdb_expect(_error, "unable to store record in the database");

        mdb_dbi_close(this->__underlying->env(), _dbi);
        _error = mdb_txn_commit(_trx);
        mdb_expect(_error, "unable to commit transaction");
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        mdb_dbi_close(this->__underlying->env(), _dbi);
        mdb_txn_abort(_trx);
        throw;
    }
    zpt::json _result{ "state", this->get_state(), "replaced", _count };
    return zpt::storage::result::alloc<zpt::storage::lmdb::result>(_result);
}

auto
zpt::storage::lmdb::action_replace::replace_one() -> void {
    this->execute();
}

zpt::storage::lmdb::action_find::action_find(zpt::storage::lmdb::collection& _collection)
  : zpt::storage::lmdb::action::action{ _collection }
  , __search{ zpt::json::object() }
  , __sort{ zpt::json::array() }
  , __fields{ zpt::json::array() } {}

zpt::storage::lmdb::action_find::action_find(zpt::storage::lmdb::collection& _collection,
                                             zpt::json _search)
  : zpt::storage::lmdb::action::action{ _collection }
  , __search{ _search }
  , __sort{ zpt::json::array() }
  , __fields{ zpt::json::array() } {}

auto
zpt::storage::lmdb::action_find::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'find' action");
    return this;
}

auto
zpt::storage::lmdb::action_find::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'find' action");
    return this;
}

auto
zpt::storage::lmdb::action_find::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'find' action");
    return this;
}

auto
zpt::storage::lmdb::action_find::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'find' action");
    return this;
}

auto
zpt::storage::lmdb::action_find::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'find' action");
    return this;
}

auto
zpt::storage::lmdb::action_find::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_find::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_find::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_find::sort(std::string const& _attribute)
  -> zpt::storage::action::type* {
    this->__sort << _attribute;
    return this;
}

auto
zpt::storage::lmdb::action_find::fields(zpt::json _fields) -> zpt::storage::action::type* {
    this->__fields += _fields;
    return this;
}

auto
zpt::storage::lmdb::action_find::offset(size_t _rows) -> zpt::storage::action::type* {
    this->__offset = _rows;
    return this;
}

auto
zpt::storage::lmdb::action_find::limit(size_t _number) -> zpt::storage::action::type* {
    this->__limit = _number;
    return this;
}

auto
zpt::storage::lmdb::action_find::bind(zpt::json _map) -> zpt::storage::action::type* {
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
zpt::storage::lmdb::action_find::execute() -> zpt::storage::result {
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
            std::string _document_key = zpt::storage::lmdb::to_db_key(this->__search);
            MDB_val _key{ _document_key.length(), _document_key.data() };
            MDB_val _value_f;
            _error = mdb_get(_trx, _dbi, &_key, &_value_f);
            mdb_expect(_error, "unable to find the record");

            _found << zpt::storage::lmdb::from_db_doc(
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
                    _found << zpt::storage::lmdb::from_db_doc(
                      std::string{ static_cast<char*>(_value_f.mv_data), _value_f.mv_size });
                }
            } while (_error == 0 && _found->size() != this->__limit);
            mdb_cursor_close(_cursor);
        }

        mdb_dbi_close(this->__underlying->env(), _dbi);
        mdb_txn_abort(_trx);
    }
    catch (zpt::failed_expectation const& _e) {
        this->set_state(-1);
        mdb_cursor_close(_cursor);
        mdb_dbi_close(this->__underlying->env(), _dbi);
        mdb_txn_abort(_trx);
        throw;
    }
    zpt::json _result{ "state", this->get_state(), "cursor", _found };
    return zpt::storage::result::alloc<zpt::storage::lmdb::result>(_result);
}

zpt::storage::lmdb::result::result(zpt::json _result)
  : __result{ _result }
  , __current{ __result["cursor"].begin() } {}

auto
zpt::storage::lmdb::result::fetch(size_t _amount) -> zpt::json {
    if (_amount == 0) { _amount = std::numeric_limits<size_t>::max(); }
    auto _result = zpt::json::array();
    for (size_t _fetched = 0; this->__current != this->__result["cursor"].end();
         ++this->__current, ++_fetched) {
        if (_amount == 1) { return std::get<2>(*this->__current); }
        _result << std::get<2>(*this->__current);
        if (_fetched == _amount) { break; }
    }
    return (_result->size() != 0 ? _result : zpt::undefined);
}

auto
zpt::storage::lmdb::result::generated_id() -> zpt::json {
    return this->__result["generated"];
}

auto
zpt::storage::lmdb::result::count() -> size_t {
    if (this->__result["generated"]->ok()) { return this->__result["generated"]->size(); }
    return this->__result["cursor"]->size();
}

auto
zpt::storage::lmdb::result::status() -> zpt::status {
    return static_cast<size_t>(this->__result["state"]["code"]);
}

auto
zpt::storage::lmdb::result::message() -> std::string {
    return this->__result["state"]["message"]->string();
}

auto
zpt::storage::lmdb::result::to_json() -> zpt::json {
    return this->__result;
}
