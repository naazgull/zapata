/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <zapata/mongodb/Client.h>

zpt::mongodb::ClientPtr::ClientPtr(zpt::mongodb::Client* _target)
  : std::shared_ptr<zpt::mongodb::Client>(_target) {}

zpt::mongodb::ClientPtr::ClientPtr(zpt::json _options, std::string _conf_path)
  : std::shared_ptr<zpt::mongodb::Client>(new zpt::mongodb::Client(_options, _conf_path)) {}

zpt::mongodb::ClientPtr::~ClientPtr() {}

zpt::mongodb::Client::Client(zpt::json _options, std::string _conf_path)
  : __options(_options)
  , __conn(nullptr) {
    this->connection(_options->get_path(_conf_path));
}

zpt::mongodb::Client::~Client() {
    if (this->__conn.get() != nullptr) {
        this->conn().done();
    }
}

auto
zpt::mongodb::Client::name() -> std::string {
    return std::string("mongodb://") + ((std::string)this->connection()["bind"]) +
           std::string(":") + ((std::string)this->connection()["port"]) + std::string("/") +
           ((std::string)this->connection()["db"]);
}

auto
zpt::mongodb::Client::options() -> zpt::json {
    return this->__options;
}

auto
zpt::mongodb::Client::events(zpt::ev::emitter _emitter) -> void {
    this->__events = _emitter;
}

auto
zpt::mongodb::Client::events() -> zpt::ev::emitter {
    return this->__events;
}

auto
zpt::mongodb::Client::conn() -> mongo::ScopedDbConnection& {
    return (*this->__conn.get());
}

auto
zpt::mongodb::Client::connect() -> void {
    zpt::Connector::connect();
}

auto
zpt::mongodb::Client::reconnect() -> void {
    zpt::Connector::reconnect();
}

auto
zpt::mongodb::Client::insert(std::string _collection,
                             std::string _href_prefix,
                             zpt::json _document,
                             zpt::json _opts) -> std::string {
    assertz(_document->ok() && _document->type() == zpt::JSObject,
            "'_document' must be of type JSObject",
            412,
            0);

    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);
    if (this->connection()["user"]->ok()) {
        _conn->auth(BSON("mechanism"
                         << "MONGODB-CR"
                         << "user" << (std::string)this->connection()["user"] << "pwd"
                         << (std::string)this->connection()["passwd"] << "db"
                         << (std::string)this->connection()["db"]));
    }
    _conn->setWriteConcern((mongo::WriteConcern)2);

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    if (!_document["id"]->ok()) {
        _document << "id" << zpt::generate::r_uuid();
    }
    if (!_document["href"]->ok() && _href_prefix.length() != 0) {
        _document << "href"
                  << (_href_prefix +
                      (_href_prefix.back() != '/' ? std::string("/") : std::string("")) +
                      _document["id"]->str());
    }
    _document << "_id" << _document["href"];

    zpt::json _exclude =
      (_opts["fields"]->is_array() ? _document - zpt::mongodb::get_fields(_opts) : zpt::undefined);
    mongo::BSONObjBuilder _mongo_document;
    zpt::mongodb::tomongo(_document - _exclude, _mongo_document);
    _conn->insert(_full_collection, _mongo_document.obj());
    _conn.done();

    if (!bool(_opts["mutated-event"]))
        zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
    return _document["id"]->str();
}

auto
zpt::mongodb::Client::upsert(std::string _collection,
                             std::string _href_prefix,
                             zpt::json _document,
                             zpt::json _opts) -> std::string {
    assertz(_document->ok() && _document->type() == zpt::JSObject,
            "'_document' must be of type JSObject",
            412,
            0);
    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    {
        if (_document["href"]->ok() || _document["id"]->ok()) {
            if (!_document["href"]->ok()) {
                _document << "href"
                          << (_href_prefix +
                              (_href_prefix.back() != '/' ? std::string("/") : std::string("")) +
                              _document["id"]->str());
            }
            if (!_document["id"]->ok()) {
                zpt::json _split = zpt::split(_document["href"]->str(), "/");
                _document << "id" << _split->arr()->back();
            }
            std::string _href = std::string(_document["href"]);
            unsigned long _size = 0;
            _size =
              _conn->count(_full_collection, BSON("_id" << _href), (int)mongo::QueryOption_SlaveOk);
            if (_size != 0) {
                zpt::json _exclude =
                  (_opts["fields"]->is_array() ? _document - zpt::mongodb::get_fields(_opts)
                                               : zpt::undefined);
                mongo::BSONObjBuilder _mongo_document;
                zpt::mongodb::tomongo(zpt::json({ "$set", _document - _exclude }), _mongo_document);
                _conn->update(
                  _full_collection, BSON("_id" << _href), _mongo_document.obj(), false, false);
                _conn.done();

                if (!bool(_opts["mutated-event"]))
                    zpt::Connector::set(_collection, _href, _document, _opts);
                return _document["id"]->str();
            }
        }
    }
    {
        if (!_document["id"]->ok()) {
            _document << "id" << zpt::generate::r_uuid();
        }
        if (!_document["href"]->ok() && _href_prefix.length() != 0) {
            _document << "href"
                      << (_href_prefix +
                          (_href_prefix.back() != '/' ? std::string("/") : std::string("")) +
                          _document["id"]->str());
        }
        _document << "_id" << _document["href"];

        zpt::json _exclude =
          (_opts["fields"]->is_array() ? _document - zpt::mongodb::get_fields(_opts)
                                       : zpt::undefined);
        mongo::BSONObjBuilder _mongo_document;
        zpt::mongodb::tomongo(_document - _exclude, _mongo_document);
        _conn->insert(_full_collection, _mongo_document.obj());
        _conn.done();

        if (!bool(_opts["mutated-event"]))
            zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
    }
    return _document["id"]->str();
}

auto
zpt::mongodb::Client::save(std::string _collection,
                           std::string _href,
                           zpt::json _document,
                           zpt::json _opts) -> int {
    assertz(_document->ok() && _document->type() == zpt::JSObject,
            "'_document' must be of type JSObject",
            412,
            0);
    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    zpt::json _exclude =
      (_opts["fields"]->is_array() ? _document - zpt::mongodb::get_fields(_opts) : zpt::undefined);
    mongo::BSONObjBuilder _mongo_document;
    zpt::mongodb::tomongo(_document - _exclude, _mongo_document);
    _conn->update(_full_collection, BSON("_id" << _href), _mongo_document.obj(), false, false);
    _conn.done();

    if (!bool(_opts["mutated-event"]))
        zpt::Connector::save(_collection, _href, _document, _opts);
    return 1;
}

auto
zpt::mongodb::Client::set(std::string _collection,
                          std::string _href,
                          zpt::json _document,
                          zpt::json _opts) -> int {
    assertz(_document->ok() && _document->type() == zpt::JSObject,
            "'_document' must be of type JSObject",
            412,
            0);
    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    zpt::json _exclude =
      (_opts["fields"]->is_array() ? _document - zpt::mongodb::get_fields(_opts) : zpt::undefined);
    mongo::BSONObjBuilder _mongo_document;
    zpt::mongodb::tomongo(zpt::json({ "$set", _document - _exclude }), _mongo_document);
    _conn->update(_full_collection, BSON("_id" << _href), _mongo_document.obj(), false, false);
    _conn.done();

    if (!bool(_opts["mutated-event"]))
        zpt::Connector::set(_collection, _href, _document, _opts);
    return 1;
}

auto
zpt::mongodb::Client::set(std::string _collection,
                          zpt::json _pattern,
                          zpt::json _document,
                          zpt::json _opts) -> int {
    assertz(_document->ok() && _document->type() == zpt::JSObject,
            "'_document' must be of type JSObject",
            412,
            0);
    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);
    if (!_pattern->ok()) {
        _pattern = zpt::json::object();
    }

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    mongo::BSONObjBuilder _query_b;
    zpt::mongodb::get_query(_pattern, _query_b);
    mongo::Query _filter(_query_b.done());

    unsigned long _size = 0;
    _size = _conn->count(_full_collection, _filter.obj, (int)mongo::QueryOption_SlaveOk);

    zpt::json _exclude =
      (_opts["fields"]->is_array() ? _document - zpt::mongodb::get_fields(_opts) : zpt::undefined);
    mongo::BSONObjBuilder _mongo_document;
    zpt::mongodb::tomongo(zpt::json({ "$set", _document - _exclude }), _mongo_document);
    _conn->update(_full_collection, _filter, _mongo_document.obj(), false, bool(_opts["multi"]));
    _conn.done();

    if (!bool(_opts["mutated-event"]) && _size != 0)
        zpt::Connector::set(_collection, _pattern, _document, _opts);
    return _size;
}

auto
zpt::mongodb::Client::unset(std::string _collection,
                            std::string _href,
                            zpt::json _document,
                            zpt::json _opts) -> int {
    assertz(_document->ok() && _document->type() == zpt::JSObject,
            "'_document' must be of type JSObject",
            412,
            0);
    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    zpt::json _exclude =
      (_opts["fields"]->is_array() ? _document - zpt::mongodb::get_fields(_opts) : zpt::undefined);
    mongo::BSONObjBuilder _mongo_document;
    zpt::mongodb::tomongo(zpt::json({ "$unset", _document - _exclude }), _mongo_document);
    _conn->update(_full_collection, BSON("_id" << _href), _mongo_document.obj(), false, false);
    _conn.done();

    if (!bool(_opts["mutated-event"]))
        zpt::Connector::unset(_collection, _href, _document, _opts);
    return 1;
}

auto
zpt::mongodb::Client::unset(std::string _collection,
                            zpt::json _pattern,
                            zpt::json _document,
                            zpt::json _opts) -> int {
    assertz(_document->ok() && _document->type() == zpt::JSObject,
            "'_document' must be of type JSObject",
            412,
            0);
    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);
    if (!_pattern->ok()) {
        _pattern = zpt::json::object();
    }

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    mongo::BSONObjBuilder _query_b;
    zpt::mongodb::get_query(_pattern, _query_b);
    mongo::Query _filter(_query_b.done());

    unsigned long _size = 0;
    _size = _conn->count(_full_collection, _filter.obj, (int)mongo::QueryOption_SlaveOk);

    zpt::json _exclude =
      (_opts["fields"]->is_array() ? _document - zpt::mongodb::get_fields(_opts) : zpt::undefined);
    mongo::BSONObjBuilder _mongo_document;
    zpt::mongodb::tomongo(zpt::json({ "$unset", _document - _exclude }), _mongo_document);
    _conn->update(_full_collection, _filter, _mongo_document.obj(), false, bool(_opts["multi"]));
    _conn.done();

    if (!bool(_opts["mutated-event"]) && _size != 0)
        zpt::Connector::unset(_collection, _pattern, _document, _opts);
    return _size;
}

auto
zpt::mongodb::Client::remove(std::string _collection, std::string _href, zpt::json _opts) -> int {
    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    zpt::json _removed;
    if (!bool(_opts["mutated-event"]))
        _removed = this->get(_collection, _href);
    _conn->remove(_full_collection, BSON("_id" << _href));
    _conn.done();

    if (!bool(_opts["mutated-event"]))
        zpt::Connector::remove(
          _collection, _removed["href"]->str(), _opts + zpt::json{ "removed", _removed });
    return 1;
}

auto
zpt::mongodb::Client::remove(std::string _collection, zpt::json _pattern, zpt::json _opts) -> int {
    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);
    if (!_pattern->ok()) {
        _pattern = zpt::json::object();
    }

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    zpt::json _selected = this->query(_collection, _pattern, _opts);
    if (!_selected->ok()) {
        return 0;
    }
    for (auto _record : _selected["elements"]->arr()) {
        _conn->remove(_full_collection, BSON("id" << _record["id"]->str()));

        if (!bool(_opts["mutated-event"]))
            zpt::Connector::remove(
              _collection, _record["href"]->str(), _opts + zpt::json{ "removed", _record });
    }
    _conn.done();

    return int(_selected["size"]);
}

auto
zpt::mongodb::Client::get(std::string _collection, std::string _topic, zpt::json _opts)
  -> zpt::json {
    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    zpt::json _fields = zpt::mongodb::get_fields(_opts);
    mongo::BSONObjBuilder _bb_fields;
    zpt::mongodb::tomongo(_fields, _bb_fields);
    mongo::BSONObj _filter = _bb_fields.obj();
    std::unique_ptr<mongo::DBClientCursor> _result;
    _result.reset(
      _conn
        ->query(_full_collection,
                BSON("_id" << _topic),
                0,
                0,
                (_fields->is_object() && _fields->obj()->size() != 0 ? &_filter : nullptr),
                (int)mongo::QueryOption_SlaveOk)
        .release());

    if (_result->more()) {
        mongo::BSONObj _record = _result->next();
        zpt::json _obj = zpt::json::object();
        zpt::mongodb::frommongo(_record, _obj);
        return _obj;
    }
    _conn.done();

    return zpt::undefined;
}

auto
zpt::mongodb::Client::query(std::string _collection, std::string _pattern, zpt::json _opts)
  -> zpt::json {
    return this->query(_collection, zpt::json(_pattern), _opts);
}

auto
zpt::mongodb::Client::query(std::string _collection, zpt::json _pattern, zpt::json _opts)
  -> zpt::json {
    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);
    if (!_pattern->ok()) {
        _pattern = zpt::json::object();
    }

    zpt::JSONArr _elements;

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    size_t _page_size = size_t(_opts["page_size"]);
    size_t _page_start_index = size_t(_opts["page_start_index"]);
    mongo::BSONObjBuilder _order_b;
    mongo::BSONObjBuilder _query_b;
    zpt::mongodb::get_query(_pattern, _query_b);

    if (_opts["order_by"]->ok()) {
        std::istringstream lss(((std::string)_opts["order_by"]).data());
        std::string _part;
        while (std::getline(lss, _part, ',')) {
            if (_part.length() > 0) {
                int _dir = 1;

                if (_part[0] == '-') {
                    _dir = -1;
                    _part.erase(0, 1);
                }
                else if (_part[0] == '+') {
                    _part.erase(0, 1);
                }

                if (_part.length() > 0) {
                    ostringstream oss;
                    oss << _part << std::flush;

                    _order_b.append(oss.str(), _dir);
                }
            }
        }
    }

    mongo::Query _query(_query_b.done());
    unsigned long _size = 0;
    _size = _conn->count(_full_collection, _query.obj, (int)mongo::QueryOption_SlaveOk);
    mongo::BSONObj _order = _order_b.done();
    if (!_order.isEmpty()) {
        _query.sort(_order);
    }

    // zdbg(_query.obj.jsonString(mongo::JS));
    zpt::json _fields = zpt::mongodb::get_fields(_opts);
    mongo::BSONObjBuilder _bb_fields;
    zpt::mongodb::tomongo(_fields, _bb_fields);
    mongo::BSONObj _filter = _bb_fields.obj();
    std::unique_ptr<mongo::DBClientCursor> _result;
    _result.reset(
      _conn
        ->query(_full_collection,
                _query,
                _page_size,
                _page_start_index,
                (_fields->is_object() && _fields->obj()->size() != 0 ? &_filter : nullptr),
                (int)mongo::QueryOption_SlaveOk)
        .release());
    while (_result->more()) {
        mongo::BSONObj _record = _result->next();
        zpt::JSONObj _obj;
        zpt::mongodb::frommongo(_record, _obj);
        _elements << _obj;
    }
    _conn.done();

    if (_elements->size() == 0) {
        return zpt::undefined;
    }
    zpt::json _return = { "size", _size, "elements", _elements };
    if (_page_size != 0) {
        _return << "links"
                << zpt::json(
                     { "next",
                       (std::string("?page_size=") + std::to_string(_page_size) +
                        std::string("&page_start_index=") +
                        std::to_string(_page_start_index + _page_size)),
                       "prev",
                       (std::string("?page_size=") + std::to_string(_page_size) +
                        std::string("&page_start_index=") +
                        std::to_string(
                          _page_size < _page_start_index ? _page_start_index - _page_size : 0)) });
    }
    return _return;
}

auto
zpt::mongodb::Client::all(std::string _collection, zpt::json _opts) -> zpt::json {
    mongo::ScopedDbConnection _conn((std::string)this->connection()["bind"]);
    zpt::JSONArr _elements;

    std::string _full_collection(_collection);
    _full_collection.insert(0, ".");
    _full_collection.insert(0, (std::string)this->connection()["db"]);

    size_t _page_size = size_t(_opts["page_size"]);
    size_t _page_start_index = size_t(_opts["page_start_index"]);
    mongo::BSONObjBuilder _order_b;
    mongo::BSONObjBuilder _query_b;

    if (_opts["order_by"]->ok()) {
        std::istringstream lss(((std::string)_opts["order_by"]).data());
        std::string _part;
        while (std::getline(lss, _part, ',')) {
            if (_part.length() > 0) {
                int _dir = 1;

                if (_part[0] == '-') {
                    _dir = -1;
                    _part.erase(0, 1);
                }
                else if (_part[0] == '+') {
                    _part.erase(0, 1);
                }

                if (_part.length() > 0) {
                    ostringstream oss;
                    oss << _part << std::flush;

                    _order_b.append(oss.str(), _dir);
                }
            }
        }
    }

    mongo::Query _query(_query_b.done());
    unsigned long _size = 0;
    _size = _conn->count(_full_collection, _query.obj, (int)mongo::QueryOption_SlaveOk);
    mongo::BSONObj _order = _order_b.done();
    if (!_order.isEmpty()) {
        _query.sort(_order);
    }

    zpt::json _fields = zpt::mongodb::get_fields(_opts);
    mongo::BSONObjBuilder _bb_fields;
    zpt::mongodb::tomongo(_fields, _bb_fields);
    mongo::BSONObj _filter = _bb_fields.obj();
    std::unique_ptr<mongo::DBClientCursor> _result;
    _result.reset(
      _conn
        ->query(_full_collection,
                _query,
                _page_size,
                _page_start_index,
                (_fields->is_object() && _fields->obj()->size() != 0 ? &_filter : nullptr),
                (int)mongo::QueryOption_SlaveOk)
        .release());

    while (_result->more()) {
        mongo::BSONObj _record = _result->next();
        zpt::JSONObj _obj;
        zpt::mongodb::frommongo(_record, _obj);
        _elements << _obj;
    }
    _conn.done();

    if (_elements->size() == 0) {
        return zpt::undefined;
    }
    zpt::json _return = { "size", _size, "elements", _elements };
    if (_page_size != 0) {
        _return << "links"
                << zpt::json(
                     { "next",
                       (std::string("?page_size=") + std::to_string(_page_size) +
                        std::string("&page_start_index=") +
                        std::to_string(_page_start_index + _page_size)),
                       "prev",
                       (std::string("?page_size=") + std::to_string(_page_size) +
                        std::string("&page_start_index=") +
                        std::to_string(
                          _page_size < _page_start_index ? _page_start_index - _page_size : 0)) });
    }
    return _return;
}

extern "C" auto
zpt_mongodb() -> int {
    return 1;
}
