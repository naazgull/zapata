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

#include <zapata/connector/layer.h>

auto
zpt::DB_DRIVER() -> size_t& {
    static size_t _global{ 0 };
    return _global;
}

zpt::storage::layer::connection::connection(zpt::json _config)
  : __config{ _config } {}

auto
zpt::storage::layer::connection::open(zpt::json _options) -> zpt::storage::connection::type* {
    for (auto [_name, _connector] : this->__connectors) {
        _connector->open(_options[_name]);
    }
    return this;
}

auto
zpt::storage::layer::connection::close() -> zpt::storage::connection::type* {
    for (auto [_, _connector] : this->__connectors) {
        _connector->close();
    }
    return this;
}

auto
zpt::storage::layer::connection::session() -> zpt::storage::session {
    return zpt::storage::session::alloc<zpt::storage::layer::session>(*this);
}

auto
zpt::storage::layer::connection::add(std::string _name, zpt::storage::connection _connector)
  -> zpt::storage::connection::type* {
    this->__connectors.insert(std::make_pair(_name, _connector));
    return this;
}

auto
zpt::storage::layer::connection::config() -> zpt::json& {
    return this->__config;
}

auto
zpt::storage::layer::connection::connectors() -> std::map<std::string, zpt::storage::connection>& {
    return this->__connectors;
}

zpt::storage::layer::session::session(zpt::storage::layer::connection& _connection)
  : __connection{ _connection } {
    for (auto [_name, _connector] : this->__connection.connectors()) {
        this->__sessions.insert(std::make_pair(_name, _connector->session()));
    }
}

auto
zpt::storage::layer::session::is_open() -> bool {
    return true;
}

auto
zpt::storage::layer::session::commit() -> zpt::storage::session::type* {
    for (auto [_, _session] : this->__sessions) {
        _session->commit();
    }
    return this;
}

auto
zpt::storage::layer::session::rollback() -> zpt::storage::session::type* {
    for (auto [_, _session] : this->__sessions) {
        _session->rollback();
    }
    return this;
}

auto
zpt::storage::layer::session::connection() -> zpt::storage::layer::connection& {
    return this->__connection;
}

auto
zpt::storage::layer::session::sessions() -> std::map<std::string, zpt::storage::session>& {
    return this->__sessions;
}

auto
zpt::storage::layer::session::database(std::string _db) -> zpt::storage::database {
    return zpt::storage::database::alloc<zpt::storage::layer::database>(*this, _db);
}

zpt::storage::layer::database::database(zpt::storage::layer::session& _session, std::string _db)
  : __session{ _session } {
    for (auto [_name, _sss] : this->__session.sessions()) {
        this->__databases.insert(std::make_pair(_name, _sss->database(_db)));
    }
}

auto
zpt::storage::layer::database::collection(std::string _collection) -> zpt::storage::collection {
    return zpt::storage::collection::alloc<zpt::storage::layer::collection>(*this, _collection);
}

auto
zpt::storage::layer::database::session() -> zpt::storage::layer::session& {
    return this->__session;
}

auto
zpt::storage::layer::database::databases() -> std::map<std::string, zpt::storage::database>& {
    return this->__databases;
}

zpt::storage::layer::collection::collection(zpt::storage::layer::database& _database,
                                            std::string _collection)
  : __database{ _database } {
    for (auto [_name, _db] : this->__database.databases()) {
        this->__collections.insert(std::make_pair(_name, _db->collection(_collection)));
    }
}

auto
zpt::storage::layer::collection::add(zpt::json _document) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::layer::action_add>(*this, _document);
}

auto
zpt::storage::layer::collection::modify(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::layer::action_modify>(*this, _search);
}

auto
zpt::storage::layer::collection::remove(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::layer::action_remove>(*this, _search);
}

auto
zpt::storage::layer::collection::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::layer::action_replace>(*this, _id, _document);
}

auto
zpt::storage::layer::collection::find(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::layer::action_find>(*this, _search);
}

auto
zpt::storage::layer::collection::count() -> size_t {
    return 0;
}

auto
zpt::storage::layer::collection::database() -> zpt::storage::layer::database& {
    return this->__database;
}

auto
zpt::storage::layer::collection::collections() -> std::map<std::string, zpt::storage::collection>& {
    return this->__collections;
}

zpt::storage::layer::action::action(zpt::storage::layer::collection& _collection)
  : __collection{ _collection }
  , __config{ this->__collection.database().session().connection().config()["order"] } {}

auto
zpt::storage::layer::action::collection() -> zpt::storage::layer::collection& {
    return this->__collection;
}

auto
zpt::storage::layer::action::actions() -> std::map<std::string, zpt::storage::action>& {
    return this->__actions;
}

auto
zpt::storage::layer::action::config() -> zpt::json& {
    return this->__config;
}

zpt::storage::layer::action_add::action_add(zpt::storage::layer::collection& _collection,
                                            zpt::json _document)
  : zpt::storage::layer::action::action(_collection) {
    if (this->__config["add"]->ok()) {
        for (auto [_, __, _name] : this->__config["add"]) {
            this->__actions.insert(std::make_pair(
              static_cast<std::string>(_name),
              this->__collection.collections()[static_cast<std::string>(_name)]->add(_document)));
        }
    }
    else {
        for (auto [_name, _col] : this->__collection.collections()) {
            this->__actions.insert(std::make_pair(_name, _col->add(_document)));
        }
    }
}

auto
zpt::storage::layer::action_add::add(zpt::json _document) -> zpt::storage::action::type* {
    if (this->__config["add"]->ok()) {
        for (auto [_, __, _name] : this->__config["add"]) {
            this->__actions[static_cast<std::string>(_name)]->add(_document);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->add(_document);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_add::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_add::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_add::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_add::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_add::set(std::string _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    expect(false, "can't set from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_add::unset(std::string _attribute) -> zpt::storage::action::type* {
    expect(false, "can't unset from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_add::patch(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't patch from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_add::sort(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_add::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_add::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_add::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_add::bind(zpt::json _map) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_add::execute() -> zpt::storage::result {
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::layer::result>(*this);
    zpt::storage::layer::result& _layer = static_cast<zpt::storage::layer::result&>(*_to_return);
    if (this->__config["add"]->ok()) {
        for (auto [_, __, _name] : this->__config["add"]) {
            zpt::storage::result _result =
              this->__actions[static_cast<std::string>(_name)]->execute();
            _layer.results().insert(std::make_pair(_name, _result));
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            zpt::storage::result _result = _flt->execute();
            _layer.results().insert(std::make_pair(_name, _result));
        }
    }
    return _to_return;
}

zpt::storage::layer::action_modify::action_modify(zpt::storage::layer::collection& _collection,
                                                  zpt::json _search)
  : zpt::storage::layer::action::action(_collection) {
    if (this->__config["modify"]->ok()) {
        for (auto [_, __, _name] : this->__config["modify"]) {
            this->__actions.insert(std::make_pair(
              static_cast<std::string>(_name),
              this->__collection.collections()[static_cast<std::string>(_name)]->modify(_search)));
        }
    }
    else {
        for (auto [_name, _col] : this->__collection.collections()) {
            this->__actions.insert(std::make_pair(_name, _col->modify(_search)));
        }
    }
}

auto
zpt::storage::layer::action_modify::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_modify::modify(zpt::json _search) -> zpt::storage::action::type* {
    if (this->__config["modify"]->ok()) {
        for (auto [_, __, _name] : this->__config["modify"]) {
            this->__actions[static_cast<std::string>(_name)]->modify(_search);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->modify(_search);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_modify::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_modify::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_modify::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_modify::set(std::string _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    if (this->__config["modify"]->ok()) {
        for (auto [_, __, _name] : this->__config["modify"]) {
            this->__actions[static_cast<std::string>(_name)]->set(_attribute, _value);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->set(_attribute, _value);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_modify::unset(std::string _attribute) -> zpt::storage::action::type* {
    if (this->__config["modify"]->ok()) {
        for (auto [_, __, _name] : this->__config["modify"]) {
            this->__actions[static_cast<std::string>(_name)]->unset(_attribute);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->unset(_attribute);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_modify::patch(zpt::json _document) -> zpt::storage::action::type* {
    if (this->__config["modify"]->ok()) {
        for (auto [_, __, _name] : this->__config["modify"]) {
            this->__actions[static_cast<std::string>(_name)]->patch(_document);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->patch(_document);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_modify::sort(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_modify::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_modify::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_modify::limit(size_t _number) -> zpt::storage::action::type* {
    if (this->__config["modify"]->ok()) {
        for (auto [_, __, _name] : this->__config["modify"]) {
            this->__actions[static_cast<std::string>(_name)]->limit(_number);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->limit(_number);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_modify::bind(zpt::json _map) -> zpt::storage::action::type* {
    if (this->__config["modify"]->ok()) {
        for (auto [_, __, _name] : this->__config["modify"]) {
            this->__actions[static_cast<std::string>(_name)]->bind(_map);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->bind(_map);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_modify::execute() -> zpt::storage::result {
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::layer::result>(*this);
    zpt::storage::layer::result& _layer = static_cast<zpt::storage::layer::result&>(*_to_return);
    if (this->__config["modify"]->ok()) {
        for (auto [_, __, _name] : this->__config["modify"]) {
            zpt::storage::result _result =
              this->__actions[static_cast<std::string>(_name)]->execute();
            _layer.results().insert(std::make_pair(_name, _result));
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            zpt::storage::result _result = _flt->execute();
            _layer.results().insert(std::make_pair(_name, _result));
        }
    }
    return _to_return;
}

zpt::storage::layer::action_remove::action_remove(zpt::storage::layer::collection& _collection,
                                                  zpt::json _search)
  : zpt::storage::layer::action::action(_collection) {
    if (this->__config["remove"]->ok()) {
        for (auto [_, __, _name] : this->__config["remove"]) {
            this->__actions.insert(std::make_pair(
              static_cast<std::string>(_name),
              this->__collection.collections()[static_cast<std::string>(_name)]->remove(_search)));
        }
    }
    else {
        for (auto [_name, _col] : this->__collection.collections()) {
            this->__actions.insert(std::make_pair(_name, _col->remove(_search)));
        }
    }
}

auto
zpt::storage::layer::action_remove::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_remove::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_remove::remove(zpt::json _search) -> zpt::storage::action::type* {
    if (this->__config["remove"]->ok()) {
        for (auto [_, __, _name] : this->__config["remove"]) {
            this->__actions[static_cast<std::string>(_name)]->remove(_search);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->remove(_search);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_remove::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_remove::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_remove::set(std::string _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_remove::unset(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_remove::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_remove::sort(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_remove::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_remove::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_remove::limit(size_t _number) -> zpt::storage::action::type* {
    if (this->__config["remove"]->ok()) {
        for (auto [_, __, _name] : this->__config["remove"]) {
            this->__actions[static_cast<std::string>(_name)]->limit(_number);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->limit(_number);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_remove::bind(zpt::json _map) -> zpt::storage::action::type* {
    if (this->__config["remove"]->ok()) {
        for (auto [_, __, _name] : this->__config["remove"]) {
            this->__actions[static_cast<std::string>(_name)]->bind(_map);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->bind(_map);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_remove::execute() -> zpt::storage::result {
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::layer::result>(*this);
    zpt::storage::layer::result& _layer = static_cast<zpt::storage::layer::result&>(*_to_return);
    if (this->__config["remove"]->ok()) {
        for (auto [_, __, _name] : this->__config["remove"]) {
            zpt::storage::result _result =
              this->__actions[static_cast<std::string>(_name)]->execute();
            _layer.results().insert(std::make_pair(_name, _result));
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            zpt::storage::result _result = _flt->execute();
            _layer.results().insert(std::make_pair(_name, _result));
        }
    }
    return _to_return;
}

zpt::storage::layer::action_replace::action_replace(zpt::storage::layer::collection& _collection,
                                                    std::string _id,
                                                    zpt::json _document)
  : zpt::storage::layer::action::action(_collection) {
    if (this->__config["replace"]->ok()) {
        for (auto [_, __, _name] : this->__config["replace"]) {
            this->__actions.insert(std::make_pair(
              static_cast<std::string>(_name),
              this->__collection.collections()[static_cast<std::string>(_name)]->replace(
                _id, _document)));
        }
    }
    else {
        for (auto [_name, _col] : this->__collection.collections()) {
            this->__actions.insert(std::make_pair(_name, _col->replace(_id, _document)));
        }
    }
}

auto
zpt::storage::layer::action_replace::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_replace::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_replace::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_replace::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_replace::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_replace::set(std::string _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_replace::unset(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_replace::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_replace::sort(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_replace::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_replace::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_replace::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_replace::bind(zpt::json _map) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_replace::execute() -> zpt::storage::result {
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::layer::result>(*this);
    zpt::storage::layer::result& _layer = static_cast<zpt::storage::layer::result&>(*_to_return);
    if (this->__config["replace"]->ok()) {
        for (auto [_, __, _name] : this->__config["replace"]) {
            zpt::storage::result _result =
              this->__actions[static_cast<std::string>(_name)]->execute();
            _layer.results().insert(std::make_pair(_name, _result));
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            zpt::storage::result _result = _flt->execute();
            _layer.results().insert(std::make_pair(_name, _result));
        }
    }
    return _to_return;
}

zpt::storage::layer::action_find::action_find(zpt::storage::layer::collection& _collection,
                                              zpt::json _search)
  : zpt::storage::layer::action::action(_collection) {
    if (this->__config["find"]->ok()) {
        for (auto [_, __, _name] : this->__config["find"]) {
            this->__actions.insert(std::make_pair(
              static_cast<std::string>(_name),
              this->__collection.collections()[static_cast<std::string>(_name)]->find(_search)));
        }
    }
    else {
        for (auto [_name, _col] : this->__collection.collections()) {
            this->__actions.insert(std::make_pair(_name, _col->find(_search)));
        }
    }
}

auto
zpt::storage::layer::action_find::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_find::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_find::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_find::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_find::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::layer::action_find::set(std::string _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_find::unset(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_find::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::layer::action_find::sort(std::string _attribute) -> zpt::storage::action::type* {
    if (this->__config["find"]->ok()) {
        for (auto [_, __, _name] : this->__config["find"]) {
            this->__actions[static_cast<std::string>(_name)]->sort(_attribute);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->sort(_attribute);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_find::fields(zpt::json _fields) -> zpt::storage::action::type* {
    if (this->__config["find"]->ok()) {
        for (auto [_, __, _name] : this->__config["find"]) {
            this->__actions[static_cast<std::string>(_name)]->fields(_fields);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->fields(_fields);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_find::offset(size_t _rows) -> zpt::storage::action::type* {
    if (this->__config["find"]->ok()) {
        for (auto [_, __, _name] : this->__config["find"]) {
            this->__actions[static_cast<std::string>(_name)]->offset(_rows);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->offset(_rows);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_find::limit(size_t _number) -> zpt::storage::action::type* {
    if (this->__config["find"]->ok()) {
        for (auto [_, __, _name] : this->__config["find"]) {
            this->__actions[static_cast<std::string>(_name)]->limit(_number);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->limit(_number);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_find::bind(zpt::json _map) -> zpt::storage::action::type* {
    if (this->__config["find"]->ok()) {
        for (auto [_, __, _name] : this->__config["find"]) {
            this->__actions[static_cast<std::string>(_name)]->bind(_map);
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            _flt->bind(_map);
        }
    }
    return this;
}

auto
zpt::storage::layer::action_find::execute() -> zpt::storage::result {
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::layer::result>(*this);
    zpt::storage::layer::result& _layer = static_cast<zpt::storage::layer::result&>(*_to_return);
    if (this->__config["find"]->ok()) {
        for (auto [_, __, _name] : this->__config["find"]) {
            zpt::storage::result _result =
              this->__actions[static_cast<std::string>(_name)]->execute();
            _layer.results().insert(std::make_pair(_name, _result));
        }
    }
    else {
        for (auto [_name, _flt] : this->__actions) {
            zpt::storage::result _result = _flt->execute();
            _layer.results().insert(std::make_pair(_name, _result));
        }
    }
    return _to_return;
}

zpt::storage::layer::result::result(zpt::storage::layer::action& _action)
  : __action(_action) {}

auto
zpt::storage::layer::result::fetch(size_t _amount) -> zpt::json {
    if (this->__action.config()["fetch"]->ok()) {
        for (auto [_, __, _name] : this->__action.config()["fetch"]) {
            zpt::json _to_return = this->__results[_name]->fetch(_amount);
            if ((_amount == 1 && _to_return->type() == zpt::JSObject) ||
                (_amount > 1 && _to_return->type() == zpt::JSArray)) {
                return _to_return;
            }
        }
    }
    else {
        for (auto [_, _result] : this->__results) {
            zpt::json _to_return = _result->fetch(_amount);
            if ((_amount == 1 && _to_return->type() == zpt::JSObject) ||
                (_amount > 1 && _to_return->type() == zpt::JSArray)) {
                return _to_return;
            }
        }
    }
    return zpt::undefined;
}

auto
zpt::storage::layer::result::generated_id() -> zpt::json {
    zpt::json _to_return{ zpt::json::array() };
    for (auto [_, _result] : this->__results) {
        _to_return += _result->generated_id();
    }
    return _to_return;
}

auto
zpt::storage::layer::result::count() -> size_t {
    for (auto [_, _result] : this->__results) {
        return _result->count();
    }
    return 0;
}

auto
zpt::storage::layer::result::status() -> zpt::status {
    for (auto [_, _result] : this->__results) {
        return _result->status();
    }
    return 0;
}

auto
zpt::storage::layer::result::message() -> std::string {
    for (auto [_, _result] : this->__results) {
        return _result->message();
    }
    return "";
}

auto
zpt::storage::layer::result::action() -> zpt::storage::layer::action& {
    return this->__action;
}

auto
zpt::storage::layer::result::results() -> std::map<std::string, zpt::storage::result>& {
    return this->__results;
}
