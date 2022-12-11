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

#include <zapata/couchdb/Client.h>

zpt::couchdb::ClientPtr::ClientPtr(zpt::couchdb::Client* _target)
  : std::shared_ptr<zpt::couchdb::Client>(_target) {}

zpt::couchdb::ClientPtr::ClientPtr(zpt::json _options, std::string const& _conf_path)
  : std::shared_ptr<zpt::couchdb::Client>(new zpt::couchdb::Client(_options, _conf_path)) {}

zpt::couchdb::ClientPtr::~ClientPtr() {}

zpt::couchdb::Client::Client(zpt::json _options, std::string const& _conf_path)
  : __options(_options)
  , __round_robin(0) {
    try {
        zpt::json _uri = zpt::uri::parse((std::string)_options->get_path(_conf_path)["bind"]);
        if (_uri["scheme"] == zpt::json::string("zpt")) {
            _uri << "scheme"
                 << "http";
        }
        this->connection(_options->get_path(_conf_path) + zpt::json{ "uri", _uri });
    }
    catch (std::exception const& _e) {
        expect(false, std::string("could not connect to CouchDB server: ") + _e.what());
    }

    this->__pool_size = 0;
    if (_options->get_path(_conf_path)["pool"]->ok()) {
        this->__pool_size = int(_options->get_path(_conf_path)["pool"]);
        for (int _k = 0; _k < this->__pool_size; _k++) {
            this->__sockets.push_back(zpt::socketstream_ptr());
            this->__mtxs.push_back(new std::mutex());
        }
    }
}

zpt::couchdb::Client::~Client() {
    for (auto _socket : this->__sockets) { _socket->close(); }
    for (auto _mtx : this->__mtxs) { delete _mtx; }
}

auto zpt::couchdb::Client::name() -> std::string {
    return ((std::string)this->connection()["bind"]) + std::string("/") +
           ((std::string)this->connection()["db"]);
}

auto zpt::couchdb::Client::options() -> zpt::json { return this->__options; }

auto zpt::couchdb::Client::events(zpt::ev::emitter _emitter) -> void { this->__events = _emitter; }

auto zpt::couchdb::Client::events() -> zpt::ev::emitter { return this->__events; }

auto zpt::couchdb::Client::connect() -> void { zpt::Connector::connect(); }

auto zpt::couchdb::Client::reconnect() -> void { zpt::Connector::reconnect(); }

auto zpt::couchdb::Client::send(zpt::http::req _req) -> zpt::http::rep {
    bool _is_ssl = this->connection()["uri"]["scheme"] == zpt::json::string("https");
    zpt::http::rep _rep;
    this->init_request(_req);
    // zverbose(_req);
    if (this->__pool_size) {
        int _k = 0;
        {
            std::lock_guard<std::mutex> _l(this->__global);
            _k = this->__round_robin++;
            if (this->__round_robin == this->__sockets.size()) { this->__round_robin = 0; }
        }

        {
            std::lock_guard<std::mutex> _l(*this->__mtxs[_k]);
            short _n_tries = 0;
            do {
                if (!this->__sockets[_k]->is_open()) {
                    this->__sockets[_k]->open(std::string(this->connection()["uri"]["domain"]),
                                              this->connection()["uri"]["port"]->ok()
                                                ? int(this->connection()["uri"]["port"])
                                                : (_is_ssl ? 443 : 80),
                                              _is_ssl);
                    expect(!this->__sockets[_k]->is_error(),
                           std::string("couchdb: error with socket: ") +
                             this->__sockets[_k]->error_string(),
                           503,
                           this->__sockets[_k]->error_code());
                }
                try {
                    (*this->__sockets[_k]) << _req << std::flush;
                    try {
                        (*this->__sockets[_k]) >> _rep;
                    }
                    catch (zpt::SyntaxErrorException const& _e) {
                    }
                    break;
                }
                catch (std::exception const& _e) {
                    this->__sockets[_k]->close();
                }
                _n_tries++;
            } while (_n_tries != 5);
        }
    }
    else {
        zpt::socketstream _socket;
        _socket.open(std::string(this->connection()["uri"]["domain"]),
                     this->connection()["uri"]["port"]->ok()
                       ? int(this->connection()["uri"]["port"])
                       : (_is_ssl ? 443 : 80),
                     _is_ssl);
        expect(!_socket.is_error(),
               std::string("couchdb: error with socket: ") + _socket.error_string(),
               503,
               _socket.error_code());
        _socket << _req << std::flush;
        try {
            _socket >> _rep;
        }
        catch (zpt::SyntaxErrorException const& _e) {
        }
        _socket.close();
    }
    // zverbose(_rep);
    return _rep;
}

auto zpt::couchdb::Client::send_assync(zpt::json _envelope, zpt::ev::handler _callback) -> void {
    this->init_request(_envelope);
    zpt::emitter()->route(zpt::performative(int(_envelope["performative"])),
                          zpt::uri::to_str(this->connection()["uri"]) +
                            std::string(_envelope["resource"]),
                          _envelope,
                          _callback);
}

auto zpt::couchdb::Client::init_request(zpt::http::req _req) -> void {
    _req->header("Host", std::string(this->connection()["uri"]["authority"]));
    _req->header("Accept", "*/*");
    _req->header("Connection", "keep-alive");
    if (this->connection()["user"]->ok() && this->connection()["passwd"]->ok()) {
        _req->header("Authorization",
                     std::string("Basic ") +
                       zpt::base64::r_encode(std::string(this->connection()["user"]) +
                                             std::string(":") +
                                             std::string(this->connection()["passwd"])));
    }
}

auto zpt::couchdb::Client::init_request(zpt::json _envelope) -> void {
    _envelope << "headers"
              << (_envelope["headers"] +
                  zpt::json{ "Host",
                             std::string(this->connection()["uri"]["authority"]),
                             "Accept",
                             "*/*",
                             "Connection",
                             "keep-alive" } +
                  (this->connection()["user"]->ok() && this->connection()["passwd"]->ok()
                     ? zpt::json{ "Authorization",
                                  std::string("Basic ") +
                                    zpt::base64::r_encode(
                                      std::string(this->connection()["user"]) + std::string(":") +
                                      std::string(this->connection()["passwd"])) }
                     : zpt::undefined));
}

auto zpt::couchdb::Client::create_database(std::string const& _collection) -> void {
    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    zpt::http::req _req;
    _req->method(zpt::ev::Put);
    _req->url(_db_name);

    zpt::http::rep _rep = this->send(_req);
    zpt::json _response(_rep->body());
    expect(_rep->status() == zpt::http::status::HTTP201 && bool(_response["ok"]) == true,
           std::string("could not create database:\n") + std::string(_rep));
}

auto zpt::couchdb::Client::create_index(std::string const& _collection, zpt::json _fields) -> void {
    std::string _db_name = std::string("/") + std::string(this->connection()["db"]) +
                           std::string("_") + _collection + std::string("/_index");
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    zpt::http::req _req;
    _req->method(zpt::ev::Post);
    _req->url(_db_name);
    _req->header("Content-Type", "application/json");

    for (auto _field : _fields->object()) {
        std::string _body = std::string(zpt::json{ "index",
                                                   { "fields", { zpt::array, _field.first } },
                                                   "name",
                                                   (_field.first + std::string("-idx")) });
        _req->header("Content-Length", std::to_string(_body.length()));
        _req->body(_body);
        zpt::http::rep _rep = this->send(_req);
    }
}

auto zpt::couchdb::Client::insert(std::string const& _collection,
                                  std::string _href_prefix,
                                  zpt::json _document,
                                  zpt::json _opts) -> std::string {
    expect(_document->ok() && _document->type() == zpt::JSObject,
           "'_document' must be of type JSObject");
    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    if (!_document["id"]->ok()) { _document << "id" << zpt::generate::r_uuid(); }
    if (!_document["href"]->ok() && _href_prefix.length() != 0) {
        _document << "href"
                  << (_href_prefix +
                      (_href_prefix.back() != '/' ? std::string("/") : std::string("")) +
                      _document["id"]->string());
    }

    _document << "_id" << _document["href"];
    _document >> "_rev";

    zpt::json _exclude =
      (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts) : zpt::undefined);
    std::string _body = std::string(_document - _exclude);
    zpt::http::req _req;
    _req->method(zpt::ev::Post);
    _req->url(_db_name);
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", std::to_string(_body.length()));
    _req->body(_body);

    zpt::http::rep _rep = this->send(_req);
    if (_rep->status() == zpt::http::status::HTTP404) {
        this->create_database(_collection);
        _rep = this->send(_req);
    }
    expect(_rep->status() == zpt::http::status::HTTP201,
           std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") +
             std::string(_rep),
           int(_rep->status()),
           1201);

    if (!bool(_opts["mutated-event"]))
        zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
    return _document["id"]->string();
}

auto zpt::couchdb::Client::upsert(std::string const& _collection,
                                  std::string _href_prefix,
                                  zpt::json _document,
                                  zpt::json _opts) -> std::string {
    expect(_document->ok() && _document->type() == zpt::JSObject,
           "'_document' must be of type JSObject");
    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    {
        if (_document["href"]->ok() || _document["id"]->ok()) {
            if (!_document["href"]->ok()) {
                _document << "href"
                          << (_href_prefix +
                              (_href_prefix.back() != '/' ? std::string("/") : std::string("")) +
                              _document["id"]->string());
            }
            if (!_document["id"]->ok()) {
                zpt::json _split = zpt::split(_document["href"]->string(), "/");
                _document << "id" << _split->array()->back();
            }
            std::string _url =
              _db_name + std::string("/") + zpt::url::r_encode(std::string(_document["href"]));
            zpt::json _upsert;
            zpt::http::rep _rep;
            zpt::http::req _req;
            _req->method(zpt::ev::Put);
            _req->url(_url);
            _req->header("Content-Type", "application/json");
            do {
                zpt::json _revision = this->get(_collection, std::string(_document["href"]));
                if (!_revision->ok()) { break; }
                _upsert = _revision | _document;
                if (!_upsert["id"]->ok()) { _upsert << "id" << zpt::generate::r_uuid(); }
                if (!_upsert["href"]->ok() && _href_prefix.length() != 0) {
                    _upsert << "href"
                            << (_href_prefix +
                                (_href_prefix.back() != '/' ? std::string("/") : std::string("")) +
                                _upsert["id"]->string());
                }
                _upsert << "_id" << _upsert["href"];

                zpt::json _exclude =
                  (_opts["fields"]->is_array() ? _upsert - zpt::couchdb::get_fields(_opts)
                                               : zpt::undefined);
                std::string _body = std::string(_upsert - _exclude);
                _req->header("Content-Length", std::to_string(_body.length()));
                _req->body(_body);
                _rep = this->send(_req);
            } while (_rep->status() == zpt::http::status::HTTP409);
            if (_rep->status() == zpt::http::status::HTTP201) {
                if (!bool(_opts["mutated-event"]))
                    zpt::Connector::set(_collection, std::string(_upsert["href"]), _upsert, _opts);
                return _upsert["id"]->string();
            }
        }
    }
    {
        if (!_document["id"]->ok()) { _document << "id" << zpt::generate::r_uuid(); }
        if (!_document["href"]->ok() && _href_prefix.length() != 0) {
            _document << "href"
                      << (_href_prefix +
                          (_href_prefix.back() != '/' ? std::string("/") : std::string("")) +
                          _document["id"]->string());
        }
        _document << "_id" << _document["href"];
        _document >> "_rev";

        zpt::json _exclude =
          (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts)
                                       : zpt::undefined);
        std::string _body = std::string(_document - _exclude);
        zpt::http::req _req;
        _req->method(zpt::ev::Post);
        _req->url(_db_name);
        _req->header("Content-Type", "application/json");
        _req->header("Content-Length", std::to_string(_body.length()));
        _req->body(_body);

        zpt::http::rep _rep = this->send(_req);
        if (_rep->status() == zpt::http::status::HTTP404) {
            this->create_database(_collection);
            _rep = this->send(_req);
        }
        expect(_rep->status() == zpt::http::status::HTTP201,
               std::string("couldn't upsert document ") + std::string(_document["href"]) +
                 std::string(": ") + _rep->body(),
               _rep->status(),
               2002);
        if (!bool(_opts["mutated-event"]))
            zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
    }
    return _document["id"]->string();
}

auto zpt::couchdb::Client::save(std::string const& _collection,
                                std::string _href,
                                zpt::json _document,
                                zpt::json _opts) -> int {
    expect(_document->ok() && _document->type() == zpt::JSObject,
           std::string("'_document' must be of type JSObject"));
    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    std::string _url = _db_name + std::string("/") + zpt::url::r_encode(_href);
    zpt::http::req _req;
    _req->method(zpt::ev::Put);
    _req->url(_url);
    _req->header("Content-Type", "application/json");
    zpt::http::rep _rep;
    do {
        zpt::json _revision = this->get(_collection, _href);
        _document << "_rev" << _revision["_rev"];

        zpt::json _exclude =
          (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts)
                                       : zpt::undefined);
        std::string _body = std::string(_document - _exclude);
        _req->header("Content-Length", std::to_string(_body.length()));
        _req->body(_body);
        _rep = this->send(_req);
    } while (_rep->status() == zpt::http::status::HTTP409);
    size_t _size = size_t(_rep->status() == zpt::http::status::HTTP201);

    if (!bool(_opts["mutated-event"])) zpt::Connector::save(_collection, _href, _document, _opts);
    return _size;
}

auto zpt::couchdb::Client::set(std::string const& _collection,
                               std::string _href,
                               zpt::json _document,
                               zpt::json _opts) -> int {
    expect(_document->ok() && _document->type() == zpt::JSObject,
           std::string("'_document' must be of type JSObject"));
    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    std::string _url = _db_name + std::string("/") + zpt::url::r_encode(_href);
    zpt::http::req _req;
    _req->method(zpt::ev::Put);
    _req->url(_url);
    _req->header("Content-Type", "application/json");
    zpt::http::rep _rep;
    do {
        zpt::json _revision = this->get(_collection, _href);

        _document = _revision | _document;
        zpt::json _exclude =
          (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts)
                                       : zpt::undefined);
        std::string _body = std::string(_document - _exclude);
        _req->header("Content-Length", std::to_string(_body.length()));
        _req->body(_body);
        _rep = this->send(_req);
    } while (_rep->status() == zpt::http::status::HTTP409);
    size_t _size = size_t(_rep->status() == zpt::http::status::HTTP201);

    if (!bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _href, _document, _opts);
    return _size;
}

auto zpt::couchdb::Client::set(std::string const& _collection,
                               zpt::json _pattern,
                               zpt::json _document,
                               zpt::json _opts) -> int {
    expect(_pattern->ok() && _pattern->type() == zpt::JSObject,
           "'_pattern' must be of type JSObject");
    size_t _size = 0;

    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    zpt::http::req _req;
    _req->method(zpt::ev::Put);
    _req->header("Content-Type", "application/json");
    zpt::json _result = this->query(_collection, _pattern);
    if (!_result->ok()) { return 0; }

    for (auto _revision : _result["elements"]->array()) {
        std::string _url =
          _db_name + std::string("/") + zpt::url::r_encode(std::string(_revision["href"]));

        _document = _revision | _document;
        zpt::json _exclude =
          (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts)
                                       : zpt::undefined);
        std::string _body = std::string(_document - _exclude);
        _req->url(_url);
        _req->header("Content-Length", std::to_string(_body.length()));
        _req->body(_body);
        zpt::http::rep _rep = this->send(_req);
        if (_rep->status() == zpt::http::status::HTTP201) { _size++; }
    }

    if (!bool(_opts["mutated-event"]) && _size != 0)
        zpt::Connector::set(_collection, _pattern, _document, _opts);
    return _size;
}

auto zpt::couchdb::Client::unset(std::string const& _collection,
                                 std::string _href,
                                 zpt::json _document,
                                 zpt::json _opts) -> int {
    expect(_document->ok() && _document->type() == zpt::JSObject,
           "'_document' must be of type JSObject");
    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    std::string _url = _db_name + std::string("/") + zpt::url::r_encode(_href);
    zpt::http::req _req;
    _req->method(zpt::ev::Put);
    _req->url(_url);
    _req->header("Content-Type", "application/json");
    zpt::http::rep _rep;
    do {
        zpt::json _revision = this->get(_collection, _href);

        _document = _revision - _document;
        zpt::json _exclude =
          (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts)
                                       : zpt::undefined);
        std::string _body = std::string(_document - _exclude);
        _req->header("Content-Length", std::to_string(_body.length()));
        _req->body(_body);
        _rep = this->send(_req);
    } while (_rep->status() == zpt::http::status::HTTP409);
    size_t _size = size_t(_rep->status() == zpt::http::status::HTTP201);

    if (!bool(_opts["mutated-event"])) zpt::Connector::unset(_collection, _href, _document, _opts);
    return _size;
}

auto zpt::couchdb::Client::unset(std::string const& _collection,
                                 zpt::json _pattern,
                                 zpt::json _document,
                                 zpt::json _opts) -> int {
    expect(_pattern->ok() && _pattern->type() == zpt::JSObject,
           "'_pattern' must be of type JSObject");
    size_t _size = 0;

    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    zpt::http::req _req;
    _req->method(zpt::ev::Put);
    _req->header("Content-Type", "application/json");
    zpt::json _result = this->query(_collection, _pattern);
    if (!_result->ok()) { return 0; }

    for (auto _revision : _result["elements"]->array()) {
        std::string _url =
          _db_name + std::string("/") + zpt::url::r_encode(std::string(_revision["href"]));

        _document = _revision - _document;
        zpt::json _exclude =
          (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts)
                                       : zpt::undefined);
        std::string _body = std::string(_document - _exclude);
        _req->url(_url);
        _req->header("Content-Length", std::to_string(_body.length()));
        _req->body(_body);
        zpt::http::rep _rep = this->send(_req);
        if (_rep->status() == zpt::http::status::HTTP201) { _size++; }
    }

    if (!bool(_opts["mutated-event"]) && _size != 0)
        zpt::Connector::unset(_collection, _pattern, _document, _opts);
    return _size;
}

auto zpt::couchdb::Client::remove(std::string const& _collection,
                                  std::string const& _href,
                                  zpt::json _opts) -> int {
    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    std::string _url = _db_name + std::string("/") + zpt::url::r_encode(_href);
    zpt::http::req _req;
    _req->method(zpt::ev::Delete);
    _req->url(_url);
    zpt::http::rep _rep;
    zpt::json _revision;
    do {
        _revision = this->get(_collection, _href);
        _req->param("rev", std::string(_revision["_rev"]));
        _rep = this->send(_req);
    } while (_rep->status() == zpt::http::status::HTTP409);
    size_t _size = size_t(_rep->status() == zpt::http::status::HTTP200);

    if (!bool(_opts["mutated-event"]))
        zpt::Connector::remove(_collection, _href, _opts + zpt::json{ "removed", _revision });
    return _size;
}

auto zpt::couchdb::Client::remove(std::string const& _collection,
                                  zpt::json _pattern,
                                  zpt::json _opts) -> int {
    expect(_pattern->ok() && _pattern->type() == zpt::JSObject,
           "'_pattern' must be of type JSObject");
    size_t _size = 0;

    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    zpt::json _result = this->query(_collection, _pattern);
    if (!_result->ok()) { return 0; }
    _size = _result["elements"]->array()->size();
    zpt::json _removed = zpt::json::array();

    for (auto _record : _result["elements"]->array()) {
        _removed << zpt::json{ "_id", _record["_id"], "_rev", _record["_rev"], "_deleted", true };
    }

    zpt::http::req _req;
    _req->method(zpt::ev::Post);
    std::string _url = _db_name + std::string("/_bulk_docs");
    std::string _body = std::string(zpt::json{ "docs", _removed });
    _req->url(_url);
    _req->body(_body);
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", std::to_string(_body.length()));
    zpt::http::rep _rep = this->send(_req);
    if (_rep->status() > 399) { _size = 0; }

    if (!bool(_opts["mutated-event"]) && _size != 0)
        zpt::Connector::remove(
          _collection, _pattern, _opts + zpt::json{ "removed", _result["elements"] });
    return _size;
}

auto zpt::couchdb::Client::get(std::string const& _collection,
                               std::string const& _href,
                               zpt::json _opts) -> zpt::json {
    expect(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
    expect(_href.length() != 0, "'href' parameter must not be empty", 0, 0);

    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    std::string _url = _db_name + std::string("/") + zpt::url::r_encode(_href);
    zpt::http::req _req;
    _req->method(zpt::ev::Get);
    _req->url(_url);
    zpt::http::rep _rep = this->send(_req);
    expect(_rep->status() == zpt::http::status::HTTP200 ||
             _rep->status() == zpt::http::status::HTTP404,
           std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") +
             std::string(_rep),
           int(_rep->status()),
           1201);

    if (!bool(_opts["mutated-event"])) zpt::Connector::get(_collection, _href, _opts);

    if (_rep->status() == zpt::http::status::HTTP200) { return zpt::json(_rep->body()); }
    return zpt::undefined;
}

auto zpt::couchdb::Client::query(std::string const& _collection,
                                 std::string const& _regexp,
                                 zpt::json _opts) -> zpt::json {
    expect(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
    expect(_regexp.length() != 0, "'_regexp' parameter must not be empty", 0, 0);
    return this->query(_collection, zpt::json(_regexp), _opts);
}

auto zpt::couchdb::Client::query(std::string const& _collection, zpt::json _regexp, zpt::json _opts)
  -> zpt::json {
    expect(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    zpt::json _query = zpt::couchdb::get_query(_regexp);
    if (_opts["fields"]->is_array()) { _query << "fields" << _opts["fields"]; }
    std::string _body = std::string(_query);
    zpt::http::req _req;
    size_t _size = 0;

    _req->method(zpt::ev::Post);
    _req->url(_db_name + std::string("/_find"));
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", std::to_string(_body.length()));
    _req->body(_body);

    zpt::http::rep _rep = this->send(_req);
    expect(_rep->status() == zpt::http::status::HTTP200 ||
             _rep->status() == zpt::http::status::HTTP404,
           std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") +
             std::string(_rep),
           int(_rep->status()),
           1201);

    zpt::json _result(_rep->body());
    zpt::json _return = zpt::json::array();
    if (_result["docs"]->is_array()) {
        _return = _result["docs"];
        _size = _result["total_rows"]->ok() ? size_t(_result["total_rows"])
                                            : (_return->is_array() ? _return->array()->size() : 0);
    }
    else if (_result["rows"]->is_array()) {
        _size = size_t(_result["total_rows"]);
        _return = _result->get_path("rows.*.doc");
    }

    if (!bool(_opts["mutated-event"])) zpt::Connector::query(_collection, _regexp, _opts);
    if (_size == 0) { return zpt::undefined; }
    return { "size", _size, "elements", _return };
}

auto zpt::couchdb::Client::all(std::string const& _collection, zpt::json _opts) -> zpt::json {
    expect(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
    std::string _db_name =
      std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
    std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

    zpt::http::req _req;
    _req->method(zpt::ev::Get);
    _req->url(_db_name + std::string("/_all_docs"));
    _req->param("include_docs", "true");
    _req->param("endkey", "\"_\"");
    if (_opts["page_size"]->ok()) { _req->param("limit", std::string(_opts["page_size"])); }
    if (_opts["page_start_index"]->ok()) {
        _req->param("skip", std::string(_opts["page_start_index"]));
    }

    zpt::http::rep _rep = this->send(_req);
    expect(_rep->status() == zpt::http::status::HTTP200,
           std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") +
             std::string(_rep),
           int(_rep->status()),
           1201);

    zpt::json _result(_rep->body());
    zpt::json _return = zpt::json::array();
    size_t _size = 0;
    if (_result["docs"]->is_array()) {
        _size = size_t(_result["total_rows"]) - 1;
        _return = _result["docs"];
    }
    else if (_result["rows"]->is_array()) {
        _size = size_t(_result["total_rows"]) - 1;
        _return = _result->get_path("rows.*.doc");
    }

    if (!bool(_opts["mutated-event"])) zpt::Connector::all(_collection, _opts);
    if (_size == 0) { return zpt::undefined; }
    return { "size", _size, "elements", _return };
}

extern "C" auto zpt_couchdb() -> int { return 1; }
