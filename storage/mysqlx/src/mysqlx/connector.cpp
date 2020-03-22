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

#include <zapata/mysqlx/connector.h>

auto
zpt::storage::mysqlx::to_search_str(zpt::json _search) -> std::string {
    if (_search->is_object()) {
        std::ostringstream _oss;
        bool _first{ true };
        for (auto [_, _key, _value] : _search) {
            if (!_first) {
                _oss << " and " << std::flush;
            }
            _first = false;
            if (_value->type() == zpt::JSRegex) {
                _oss << _key << " = :" << _value->rgx().to_string() << std::flush;
            }
            else {
                _oss << _key << " = " << _value << std::flush;
            }
        }
        return _oss.str();
    }
    return static_cast<std::string>(_search);
}

auto
zpt::storage::mysqlx::to_db_doc(zpt::json _document) -> ::mysqlx::DbDoc {
    return ::mysqlx::DbDoc{ static_cast<std::string>(_document) };
}

auto
zpt::storage::mysqlx::from_db_doc(const ::mysqlx::DbDoc& _document) -> zpt::json {
    if (!_document.isNull()) {
        std::stringstream _ss;
        _document.print(_ss);
        zpt::json _to_return;
        _ss >> _to_return;
        return _to_return;
    }
    return zpt::undefined;
}

zpt::storage::mysqlx::connection::connection(zpt::json _options)
  : __options{ _options } {}

auto
zpt::storage::mysqlx::connection::open(zpt::json _options) -> zpt::storage::connection::type* {
    this->__options = _options;
    return this;
}

auto
zpt::storage::mysqlx::connection::close() -> zpt::storage::connection::type* {
    return this;
}

auto
zpt::storage::mysqlx::connection::session() -> zpt::storage::session {
    return zpt::storage::session::alloc<zpt::storage::mysqlx::session>(*this);
}

auto
zpt::storage::mysqlx::connection::options() -> zpt::json& {
    return this->__options;
}

zpt::storage::mysqlx::session::session(zpt::storage::mysqlx::connection& _connection)
  : __underlying{ ::mysqlx::SessionOption::USER,
                  _connection.options()["user"]->str(),
                  ::mysqlx::SessionOption::PWD,
                  _connection.options()["password"]->str(),
                  ::mysqlx::SessionOption::HOST,
                  _connection.options()["host"]->str(),
                  ::mysqlx::SessionOption::PORT,
                  _connection.options()["port"]->ok() ? _connection.options()["port"]->intr()
                                                      : 33060,
                  ::mysqlx::SessionOption::SSL_MODE,
                  ::mysqlx::SSLMode::REQUIRED } {}

zpt::storage::mysqlx::session::~session() {
    this->__underlying.close();
}

auto
zpt::storage::mysqlx::session::is_open() -> bool {
    return true;
}

auto
zpt::storage::mysqlx::session::commit() -> zpt::storage::session::type* {
    this->__underlying.commit();
    return this;
}

auto
zpt::storage::mysqlx::session::rollback() -> zpt::storage::session::type* {
    this->__underlying.rollback();
    return this;
}

auto zpt::storage::mysqlx::session::operator-> () -> ::mysqlx::Session* {
    return &this->__underlying;
}

auto
zpt::storage::mysqlx::session::database(std::string _db) -> zpt::storage::database {
    return zpt::storage::database::alloc<zpt::storage::mysqlx::database>(*this, _db);
}

zpt::storage::mysqlx::database::database(zpt::storage::mysqlx::session& _session, std::string _db)
  : __underlying{ _session->getSchema(_db) } {}

auto zpt::storage::mysqlx::database::operator-> () -> ::mysqlx::Schema* {
    return &this->__underlying;
}

auto
zpt::storage::mysqlx::database::collection(std::string _collection) -> zpt::storage::collection {
    return zpt::storage::collection::alloc<zpt::storage::mysqlx::collection>(*this, _collection);
}

zpt::storage::mysqlx::collection::collection(zpt::storage::mysqlx::database& _database,
                                             std::string _collection)
  : __underlying{ _database->getCollection(_collection) } {}

auto
zpt::storage::mysqlx::collection::add(zpt::json _document) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::mysqlx::action_add>(*this, _document);
}

auto
zpt::storage::mysqlx::collection::modify(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::mysqlx::action_modify>(*this, _search);
}

auto
zpt::storage::mysqlx::collection::remove(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::mysqlx::action_remove>(*this, _search);
}

auto
zpt::storage::mysqlx::collection::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::mysqlx::action_replace>(*this, _id, _document);
}

auto
zpt::storage::mysqlx::collection::find(zpt::json _search) -> zpt::storage::action {
    return zpt::storage::action::alloc<zpt::storage::mysqlx::action_find>(*this, _search);
}

auto
zpt::storage::mysqlx::collection::count() -> size_t {
    return this->__underlying.count();
}

auto zpt::storage::mysqlx::collection::operator-> () -> ::mysqlx::Collection* {
    return &this->__underlying;
}

zpt::storage::mysqlx::action::action(zpt::storage::mysqlx::collection& _collection) {}

zpt::storage::mysqlx::action_add::action_add(zpt::storage::mysqlx::collection& _collection,
                                             zpt::json _document)
  : zpt::storage::mysqlx::action::action{ _collection }
  , __underlying{ _collection->add(zpt::storage::mysqlx::to_db_doc(_document)) } {}

auto
zpt::storage::mysqlx::action_add::add(zpt::json _document) -> zpt::storage::action::type* {
    this->__underlying.add(zpt::storage::mysqlx::to_db_doc(_document));
    return this;
}

auto
zpt::storage::mysqlx::action_add::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_add::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_add::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_add::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_add::set(std::string _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    expect(false, "can't set from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_add::unset(std::string _attribute) -> zpt::storage::action::type* {
    expect(false, "can't unset from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_add::patch(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't patch from an 'add' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_add::sort(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_add::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_add::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_add::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_add::bind(zpt::json _map) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_add::execute() -> zpt::storage::result {
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::mysqlx::result>(*this);
    return _to_return;
}

auto zpt::storage::mysqlx::action_add::operator-> () -> ::mysqlx::CollectionAdd* {
    return &this->__underlying;
}

zpt::storage::mysqlx::action_modify::action_modify(zpt::storage::mysqlx::collection& _collection,
                                                   zpt::json _search)
  : zpt::storage::mysqlx::action::action{ _collection }
  , __underlying{ _collection->modify(zpt::storage::mysqlx::to_search_str(_search)) } {}

auto
zpt::storage::mysqlx::action_modify::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_modify::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_modify::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_modify::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_modify::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'modify' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_modify::set(std::string _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    this->__underlying.set(_attribute, zpt::storage::mysqlx::to_db_doc(_value));
    return this;
}

auto
zpt::storage::mysqlx::action_modify::unset(std::string _attribute) -> zpt::storage::action::type* {
    this->__underlying.unset(_attribute);
    return this;
}

auto
zpt::storage::mysqlx::action_modify::patch(zpt::json _document) -> zpt::storage::action::type* {
    this->__underlying.patch(static_cast<std::string>(_document));
    return this;
}

auto
zpt::storage::mysqlx::action_modify::sort(std::string _attribute) -> zpt::storage::action::type* {
    this->__underlying.sort(_attribute);
    return this;
}

auto
zpt::storage::mysqlx::action_modify::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_modify::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_modify::limit(size_t _number) -> zpt::storage::action::type* {
    this->__underlying.limit(_number);
    return this;
}

auto
zpt::storage::mysqlx::action_modify::bind(zpt::json _map) -> zpt::storage::action::type* {
    for (auto [_, _key, _value] : _map) {
        this->__underlying.bind(_key, static_cast<std::string>(_value));
    }
    return this;
}

auto
zpt::storage::mysqlx::action_modify::execute() -> zpt::storage::result {
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::mysqlx::result>(*this);
    return _to_return;
}

auto zpt::storage::mysqlx::action_modify::operator-> () -> ::mysqlx::CollectionModify* {
    return &this->__underlying;
}

zpt::storage::mysqlx::action_remove::action_remove(zpt::storage::mysqlx::collection& _collection,
                                                   zpt::json _search)
  : zpt::storage::mysqlx::action::action(_collection)
  , __underlying{ _collection->remove(zpt::storage::mysqlx::to_search_str(_search)) } {}

auto
zpt::storage::mysqlx::action_remove::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_remove::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_remove::remove(zpt::json _search) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_remove::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_remove::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'remove' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_remove::set(std::string _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_remove::unset(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_remove::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_remove::sort(std::string _attribute) -> zpt::storage::action::type* {
    this->__underlying.sort(_attribute);
    return this;
}

auto
zpt::storage::mysqlx::action_remove::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_remove::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_remove::limit(size_t _number) -> zpt::storage::action::type* {
    this->__underlying.limit(_number);
    return this;
}

auto
zpt::storage::mysqlx::action_remove::bind(zpt::json _map) -> zpt::storage::action::type* {
    for (auto [_, _key, _value] : _map) {
        this->__underlying.bind(_key, static_cast<std::string>(_value));
    }
    return this;
}

auto
zpt::storage::mysqlx::action_remove::execute() -> zpt::storage::result {
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::mysqlx::result>(*this);
    return _to_return;
}

auto zpt::storage::mysqlx::action_remove::operator-> () -> ::mysqlx::CollectionRemove* {
    return &this->__underlying;
}

zpt::storage::mysqlx::action_replace::action_replace(zpt::storage::mysqlx::collection& _collection,
                                                     std::string _id,
                                                     zpt::json _document)
  : zpt::storage::mysqlx::action::action(_collection)
  , __id{ _id }
  , __document{ _document } {}

auto
zpt::storage::mysqlx::action_replace::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_replace::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_replace::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_replace::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_replace::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'replace' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_replace::set(std::string _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_replace::unset(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_replace::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_replace::sort(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_replace::fields(zpt::json _fields) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_replace::offset(size_t _rows) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_replace::limit(size_t _number) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_replace::bind(zpt::json _map) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_replace::execute() -> zpt::storage::result {
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::mysqlx::result>(*this);
    return _to_return;
}

auto
zpt::storage::mysqlx::action_replace::replace_one() -> ::mysqlx::Result {
    return this->__underlying->replaceOne(this->__id,
                                          zpt::storage::mysqlx::to_db_doc(this->__document));
}

auto zpt::storage::mysqlx::action_replace::operator-> () -> ::mysqlx::Collection* {
    return this->__underlying;
}

zpt::storage::mysqlx::action_find::action_find(zpt::storage::mysqlx::collection& _collection,
                                               zpt::json _search)
  : zpt::storage::mysqlx::action::action(_collection)
  , __underlying{ _collection->find(zpt::storage::mysqlx::to_search_str(_search)) } {}

auto
zpt::storage::mysqlx::action_find::add(zpt::json _document) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_find::modify(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_find::remove(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_find::replace(std::string _id, zpt::json _document)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_find::find(zpt::json _search) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'find' action", 500, 0);
    return this;
}

auto
zpt::storage::mysqlx::action_find::set(std::string _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_find::unset(std::string _attribute) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_find::patch(zpt::json _document) -> zpt::storage::action::type* {
    return this;
}

auto
zpt::storage::mysqlx::action_find::sort(std::string _attribute) -> zpt::storage::action::type* {
    this->__underlying.sort(_attribute);
    return this;
}

auto
zpt::storage::mysqlx::action_find::fields(zpt::json _fields) -> zpt::storage::action::type* {
    for (auto [_, __, _value] : _fields) {
        this->__underlying.fields(_value->str());
    }
    return this;
}

auto
zpt::storage::mysqlx::action_find::offset(size_t _rows) -> zpt::storage::action::type* {
    this->__underlying.offset(_rows);
    return this;
}

auto
zpt::storage::mysqlx::action_find::limit(size_t _number) -> zpt::storage::action::type* {
    this->__underlying.limit(_number);
    return this;
}

auto
zpt::storage::mysqlx::action_find::bind(zpt::json _map) -> zpt::storage::action::type* {
    for (auto [_, _key, _value] : _map) {
        this->__underlying.bind(_key, static_cast<std::string>(_value));
    }
    return this;
}

auto
zpt::storage::mysqlx::action_find::execute() -> zpt::storage::result {
    zpt::storage::result _to_return =
      zpt::storage::result::alloc<zpt::storage::mysqlx::result>(*this);
    return _to_return;
}

auto zpt::storage::mysqlx::action_find::operator-> () -> ::mysqlx::CollectionFind* {
    return &this->__underlying;
}

zpt::storage::mysqlx::result::result(zpt::storage::mysqlx::action_add& _action)
  : __result{ _action->execute() } {}

zpt::storage::mysqlx::result::result(zpt::storage::mysqlx::action_modify& _action)
  : __result{ _action->execute() } {}

zpt::storage::mysqlx::result::result(zpt::storage::mysqlx::action_remove& _action)
  : __result{ _action->execute() } {}

zpt::storage::mysqlx::result::result(zpt::storage::mysqlx::action_replace& _action)
  : __result{ _action.replace_one() } {}

zpt::storage::mysqlx::result::result(zpt::storage::mysqlx::action_find& _action)
  : __is_doc_result{ true }
  , __doc_result{ _action->execute() } {}

auto
zpt::storage::mysqlx::result::fetch(size_t _amount) -> zpt::json {
    if (this->__is_doc_result) {
        zpt::json _to_return{ zpt::json::array() };
        if (_amount == 0) {
            _amount = std::numeric_limits<size_t>::max();
        }
        for (size_t _idx = 0; _idx != _amount; ++_idx) {
            ::mysqlx::DbDoc _doc = this->__doc_result.fetchOne();
            if (_doc.isNull()) {
                if (_idx == 0) {
                    return zpt::undefined;
                }
                break;
            }
            _to_return << zpt::storage::mysqlx::from_db_doc(_doc);
        }
        if (_amount == 1 && _to_return->size() != 0) {
            return _to_return[0];
        }
        return _to_return;
    }
    return zpt::undefined;
}

auto
zpt::storage::mysqlx::result::generated_id() -> zpt::json {
    if (!this->__is_doc_result) {
        zpt::json _to_return{ zpt::json::array() };
        for (auto _id : this->__result.getGeneratedIds()) {
            _to_return << _id;
        }
        return _to_return;
    }
    return zpt::undefined;
}

auto
zpt::storage::mysqlx::result::count() -> size_t {
    if (this->__is_doc_result) {
        return this->__doc_result.count();
    }
    return 0;
}

auto
zpt::storage::mysqlx::result::status() -> zpt::status {
    if (this->__is_doc_result) {
        return this->__doc_result.getWarning(0).getCode();
    }
    else {
        return this->__result.getWarning(0).getCode();
    }
}

auto
zpt::storage::mysqlx::result::message() -> std::string {
    if (this->__is_doc_result) {
        return this->__doc_result.getWarning(0).getMessage();
    }
    else {
        return this->__result.getWarning(0).getMessage();
    }
}
