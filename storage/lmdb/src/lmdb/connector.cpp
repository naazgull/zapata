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

// auto
// zpt::storage::lmdb::evaluate_expression(zpt::json _expression, std::string _attribute)
//   -> zpt::json {
//     auto& _funcs = zpt::storage::lmdb::expression_operators();
//     auto _func = _funcs.find(_expression["name"]->string());
//     expect(_func != _funcs.end(),
//            "operator 'zpt::storage::lmdb::" << _expression["name"] << "' could not be found.",
//            500,
//            0);
//     return _func->second(_expression, _attribute);
// }

// auto
// zpt::storage::lmdb::evaluate_bind(zpt::json _expression) -> zpt::json {
//     auto& _funcs = zpt::storage::lmdb::bind_operators();
//     auto _func = _funcs.find(_expression["name"]->string());
//     expect(_func != _funcs.end(),
//            "operator 'zpt::storage::lmdb::" << _expression["name"] << "' could not be found.",
//            500,
//            0);
//     return _func->second(_expression);
// }

// auto
// zpt::storage::lmdb::to_search_str(zpt::json _search) -> std::string {
//     static const std::map<std::string, bool> _reserved = { { "page_size", true },
//                                                            { "page_start_index", true } };
//     if (_search->is_object()) {
//         std::ostringstream _oss;
//         bool _first{ true };
//         for (auto [_, _key, _value] : _search) {
//             if (_reserved.find(_key) != _reserved.end()) {
//                 continue;
//             }

//             if (!_first) {
//                 _oss << " and " << std::flush;
//             }
//             _first = false;
//             if (_value->type() == zpt::JSObject) {
//                 try {
//                     auto _expression = zpt::storage::lmdb::evaluate_expression(_value, _key);
//                     _oss << _expression->string() << std::flush;
//                 }
//                 catch (zpt::failed_expectation const& _e) {
//                     _oss << _key << " = :" << _key << std::flush;
//                 }
//             }
//             else {
//                 _oss << _key << " = :" << _key << std::flush;
//             }
//         }
//         zlog("Search for lmdb connector is: " << _oss.str(), zpt::trace);
//         return _oss.str();
//     }
//     else if (_search->is_string()) {
//         zlog("Search for lmdb connector is: " << _search->string(), zpt::trace);
//         return _search->string();
//     }
//     else if (_search->is_nil()) {
//         zlog("Search for lmdb connector is empty ", zpt::trace);
//         return "";
//     }
//     else {
//         zlog("Search for lmdb connector is: " << static_cast<std::string>(_search), zpt::trace);
//         return static_cast<std::string>(_search);
//     }
// }

// auto
// zpt::storage::lmdb::to_binded_object(zpt::json _binded) -> zpt::json {
//     if (_binded->is_object()) {
//         auto _return = zpt::json::object();
//         for (auto [_, _key, _value] : _binded) {
//             if (_value->is_object()) {
//                 auto _bind = zpt::storage::lmdb::evaluate_bind(_value);
//                 if (_bind->is_object()) {
//                     for (auto [_, _binded_key, _binded_value] : _bind) {
//                         _return << (_key + _binded_key) << _binded_value;
//                     }
//                 }
//                 else {
//                     _return << _key << _bind;
//                 }
//             }
//             else {
//                 _return << _key << _value;
//             }
//         }
//         return _return;
//     }
//     return _binded;
// }

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
    std::stringstream _ss;
    _ss.str(_document);
    zpt::json _to_return;
    _ss >> _to_return;
    return _to_return;
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
  : __options{ _connection.options() }
  , __underlying{ ::lmdb::env::create() } {
    this->__underlying.set_mapsize(1UL * 1024UL * 1024UL * 1024UL); /* 1 GiB */
}

zpt::storage::lmdb::session::~session() { this->__underlying.close(); }

auto
zpt::storage::lmdb::session::is_open() -> bool {
    return true;
}

auto
zpt::storage::lmdb::session::commit() -> zpt::storage::session::type* {
    this->__commit = true;
    ;
    return this;
}

auto
zpt::storage::lmdb::session::rollback() -> zpt::storage::session::type* {
    this->__commit = false;
    return this;
}

auto
zpt::storage::lmdb::session::operator->() -> ::lmdb::env* {
    return &this->__underlying;
}

auto
zpt::storage::lmdb::session::operator*() -> ::lmdb::env& {
    return this->__underlying;
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
  : __underlying{ *_session }
  , __commit{ _session.is_to_commit() } {
    this->__underlying->open(std::string{ _session.__options["path"]->string() +
                                          std::string{ "/" } + _db + std::string{ ".mdb" } }
                               .data(),
                             0,
                             0664);
}

auto
zpt::storage::lmdb::database::operator->() -> ::lmdb::env* {
    return &(*this->__underlying);
}

auto
zpt::storage::lmdb::database::operator*() -> ::lmdb::env& {
    return *this->__underlying;
}

auto
zpt::storage::lmdb::database::is_to_commit() -> bool& {
    return *this->__commit;
}

auto
zpt::storage::lmdb::database::collection(std::string const& _collection)
  -> zpt::storage::collection {
    return zpt::storage::collection::alloc<zpt::storage::lmdb::collection>(*this, _collection);
}

zpt::storage::lmdb::collection::collection(zpt::storage::lmdb::database& _database,
                                           std::string const& _collection)
  : __collection_name{ _collection }
  , __underlying{ *_database }
  , __commit{ _database.is_to_commit() } {}

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
    auto _read_trx = ::lmdb::txn::begin(this->__underlying->handle(), nullptr, MDB_RDONLY);
    auto _dbi = ::lmdb::dbi::open(_read_trx, nullptr);
    return _dbi.size(_read_trx);
}

auto
zpt::storage::lmdb::collection::operator->() -> ::lmdb::env* {
    return this->__underlying.get();
}

auto
zpt::storage::lmdb::collection::operator*() -> ::lmdb::env& {
    return *this->__underlying.get();
}

auto
zpt::storage::lmdb::collection::is_to_commit() -> bool& {
    return *this->__commit;
}

zpt::storage::lmdb::action::action(zpt::storage::lmdb::collection& _collection)
  : __commit{ _collection.is_to_commit() } {}

auto
zpt::storage::lmdb::action::is_to_commit() -> bool& {
    return *this->__commit;
}

zpt::storage::lmdb::action_add::action_add(zpt::storage::lmdb::collection& _collection,
                                           zpt::json _document)
  : zpt::storage::lmdb::action::action{ _collection }
  , __underlying{ ::lmdb::txn::begin(*_collection) }
  , __stub{ ::lmdb::dbi::open(this->__underlying) } {}

auto
zpt::storage::lmdb::action_add::add(zpt::json _document) -> zpt::storage::action::type* {
    this->__stub.put(this->__underlying,
                     zpt::storage::lmdb::to_db_key(_document),
                     zpt::storage::lmdb::to_db_doc(_document));
    return this;
}

auto
zpt::storage::lmdb::action_add::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_add::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_add::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_add::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_add::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    expect(false, "can't set from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_add::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    expect(false, "can't unset from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_add::patch(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't patch from an 'add' action", 500, 0);
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
    this->__underlying.commit();
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::lmdb::result>(*this);
    return _to_return;
}

auto
zpt::storage::lmdb::action_add::operator->() -> ::lmdb::txn* {
    return &this->__underlying;
}

auto
zpt::storage::lmdb::action_add::operator*() -> ::lmdb::txn& {
    return this->__underlying;
}

zpt::storage::lmdb::action_modify::action_modify(zpt::storage::lmdb::collection& _collection,
                                                 zpt::json _search)
  : zpt::storage::lmdb::action::action{ _collection }
  , __underlying{ ::lmdb::txn::begin(*_collection) }
  , __stub{ ::lmdb::dbi::open(this->__underlying) }
  , __search{ _search }
  , __set{ zpt::json::object() }
  , __unset{ zpt::json::object() } {
    expect(this->__search->is_object() && this->__search["_id"]->ok(),
           "search object must be a JSON object with `_id` instantiated",
           500,
           0);
}

auto
zpt::storage::lmdb::action_modify::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_modify::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_modify::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_modify::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_modify::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_modify::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    this->__set[_attribute] = _value;
    return this;
}

auto
zpt::storage::lmdb::action_modify::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    this->__unset[_attribute] = true;
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
    std::string _value;
    std::string _key = zpt::storage::lmdb::to_db_key(this->__search);

    auto _cursor = ::lmdb::cursor::open(this->__underlying, this->__stub);
    _cursor.find(_key);
    _cursor.get(_key, _value, MDB_GET_CURRENT);
    _cursor.close();

    auto _current = zpt::json::parse_json_str(_value) + this->__set - this->__unset;
    this->__stub.put(this->__underlying, _key, zpt::storage::lmdb::to_db_doc(_current));
    this->__underlying.commit();
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::lmdb::result>(*this);
    return _to_return;
}

auto
zpt::storage::lmdb::action_modify::operator->() -> ::lmdb::txn* {
    return &this->__underlying;
}

auto
zpt::storage::lmdb::action_modify::operator*() -> ::lmdb::txn& {
    return this->__underlying;
}

zpt::storage::lmdb::action_remove::action_remove(zpt::storage::lmdb::collection& _collection,
                                                 zpt::json _search)
  : zpt::storage::lmdb::action::action{ _collection }
  , __underlying{ ::lmdb::txn::begin(*_collection) }
  , __stub{ ::lmdb::dbi::open(this->__underlying) }
  , __search{ _search } {
    expect(this->__search->is_object() && this->__search["_id"]->ok(),
           "search object must be a JSON object with `_id` instantiated",
           500,
           0);
}

auto
zpt::storage::lmdb::action_remove::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_remove::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_remove::remove(zpt::json _search) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::lmdb::action_remove::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_remove::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'remove' action", 500, 0);
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
    this->__stub.del(this->__underlying, zpt::storage::lmdb::to_db_key(this->__search));
    this->__underlying.commit();
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::lmdb::result>(*this);
    return _to_return;
}

auto
zpt::storage::lmdb::action_remove::operator->() -> ::lmdb::txn* {
    return &this->__underlying;
}

auto
zpt::storage::lmdb::action_remove::operator*() -> ::lmdb::txn& {
    return this->__underlying;
}

zpt::storage::lmdb::action_replace::action_replace(zpt::storage::lmdb::collection& _collection,
                                                   std::string _id,
                                                   zpt::json _document)
  : zpt::storage::lmdb::action::action{ _collection }
  , __underlying{ ::lmdb::txn::begin(*_collection) }
  , __stub{ ::lmdb::dbi::open(this->__underlying) }
  , __id{ _id }
  , __set{ _document } {}

auto
zpt::storage::lmdb::action_replace::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_replace::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_replace::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_replace::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_replace::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'replace' action", 500, 0);
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
    this->__stub.del(this->__underlying, this->__id);
    this->__stub.put(this->__underlying, this->__id, zpt::storage::lmdb::to_db_doc(this->__set));
    this->__underlying.commit();
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::lmdb::result>(*this);
    return _to_return;
}

auto
zpt::storage::lmdb::action_replace::replace_one() -> void {
    this->execute();
}

auto
zpt::storage::lmdb::action_replace::operator->() -> ::lmdb::txn* {
    return &this->__underlying;
}

auto
zpt::storage::lmdb::action_replace::operator*() -> ::lmdb::txn& {
    return this->__underlying;
}

zpt::storage::lmdb::action_find::action_find(zpt::storage::lmdb::collection& _collection)
  : zpt::storage::lmdb::action::action{ _collection }
  , __underlying{ ::lmdb::txn::begin(*_collection) }
  , __stub{ ::lmdb::dbi::open(this->__underlying) }
  , __search{ zpt::json::object() }
  , __sort{ zpt::json::array() }
  , __fields{ zpt::json::array() } {}

zpt::storage::lmdb::action_find::action_find(zpt::storage::lmdb::collection& _collection,
                                             zpt::json _search)
  : zpt::storage::lmdb::action::action{ _collection }
  , __underlying{ ::lmdb::txn::begin(*_collection) }
  , __stub{ ::lmdb::dbi::open(this->__underlying) }
  , __search{ _search }
  , __sort{ zpt::json::array() }
  , __fields{ zpt::json::array() } {}

auto
zpt::storage::lmdb::action_find::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_find::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_find::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_find::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::lmdb::action_find::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'find' action", 500, 0);
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
    auto _result = zpt::json::array();
    auto _cursor = ::lmdb::cursor::open(this->__underlying, this->__stub);
    try {
        std::string _key = zpt::storage::lmdb::to_db_key(this->__search);
        std::string _value;
        _cursor.find(_key);
        _cursor.get(_key, _value, MDB_GET_CURRENT);
        _cursor.close();
        _result << zpt::json::parse_json_str(_value);
    }
    catch (zpt::failed_expectation& _) {
        std::string _key;
        std::string _value;
        while (cursor.get(_key, _value, MDB_NEXT)) {
            zpt::json _object = zpt::json::parse_json_str(_value);
            if (!this->is_filtered_out(_object)) { _result << this->trim(_object); }
        }
        cursor.close();
    }
    this->__stub.put(this->__underlying, _key, zpt::storage::lmdb::to_db_doc(_current));
    this->__underlying.commit();
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::lmdb::result>(*this);
    return _to_return;
}

auto
zpt::storage::lmdb::action_find::operator->() -> ::lmdb::txn* {
    return &this->__underlying;
}

auto
zpt::storage::lmdb::action_find::operator*() -> ::lmdb::txn& {
    return this->__underlying;
}

zpt::storage::lmdb::result::result(zpt::storage::lmdb::action_add& _action)
  : __result{ _action->execute() } {}

zpt::storage::lmdb::result::result(zpt::storage::lmdb::action_modify& _action)
  : __result{ _action->execute() } {}

zpt::storage::lmdb::result::result(zpt::storage::lmdb::action_remove& _action)
  : __result{ _action->execute() } {}

zpt::storage::lmdb::result::result(zpt::storage::lmdb::action_replace& _action)
  : __result{ _action.replace_one() } {}

zpt::storage::lmdb::result::result(zpt::storage::lmdb::action_find& _action)
  : __is_doc_result{ true }
  , __doc_result{ _action->execute() } {}

auto
zpt::storage::lmdb::result::fetch(size_t _amount) -> zpt::json {
    if (this->__is_doc_result) {
        zpt::json _to_return{ zpt::json::array() };
        if (_amount == 0) { _amount = std::numeric_limits<size_t>::max(); }
        for (size_t _idx = 0; _idx != _amount; ++_idx) {
            ::lmdb::DbDoc _doc = this->__doc_result.fetchOne();
            if (_doc.isNull()) {
                if (_idx == 0) { return zpt::undefined; }
                break;
            }
            _to_return << zpt::storage::lmdb::from_db_doc(_doc);
        }
        if (_amount == 1 && _to_return->size() != 0) { return _to_return[0]; }
        return _to_return;
    }
    return zpt::undefined;
}

auto
zpt::storage::lmdb::result::generated_id() -> zpt::json {
    if (!this->__is_doc_result) {
        zpt::json _to_return{ zpt::json::array() };
        for (auto _id : this->__result.getGeneratedIds()) { _to_return << _id; }
        return _to_return;
    }
    return zpt::undefined;
}

auto
zpt::storage::lmdb::result::count() -> size_t {
    if (this->__is_doc_result) { return this->__doc_result.count(); }
    return 0;
}

auto
zpt::storage::lmdb::result::status() -> zpt::status {
    if (this->__is_doc_result) { return this->__doc_result.getWarning(0).getCode(); }
    else {
        return this->__result.getWarning(0).getCode();
    }
}

auto
zpt::storage::lmdb::result::message() -> std::string {
    if (this->__is_doc_result) { return this->__doc_result.getWarning(0).getMessage(); }
    else {
        return this->__result.getWarning(0).getMessage();
    }
}
