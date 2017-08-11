/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <n@zgul.me>

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

#include <zapata/couchdb/Client.h>

zpt::couchdb::ClientPtr::ClientPtr(zpt::couchdb::Client * _target) : std::shared_ptr<zpt::couchdb::Client>(_target) {
}

zpt::couchdb::ClientPtr::ClientPtr(zpt::json _options, std::string _conf_path) : std::shared_ptr<zpt::couchdb::Client>(new zpt::couchdb::Client(_options, _conf_path)) {
}

zpt::couchdb::ClientPtr::~ClientPtr() {
}

zpt::couchdb::Client::Client(zpt::json _options, std::string _conf_path) : __options( _options), __round_robin(0) {
	try {
		zpt::json _uri = zpt::uri::parse((std::string) _options->getPath(_conf_path)["bind"]);
		if (_uri["scheme"] == zpt::json::string("zpt")) {
			_uri << "scheme" << "http";
		}
		this->connection(_options->getPath(_conf_path) + zpt::json{ "uri", _uri });
	}
	catch(std::exception& _e) {
		assertz(false, std::string("could not connect to CouchDB server: ") + _e.what(), 500, 0);
	}

	this->__pool_size = 0;
	if (_options->getPath(_conf_path)["pool"]->ok()) {
		this->__pool_size = int(_options->getPath(_conf_path)["pool"]);
		for (int _k = 0; _k < this->__pool_size; _k++) {
			this->__sockets.push_back(zpt::socketstream_ptr(new zpt::socketstream()));
			this->__mtxs.push_back(new std::mutex());
		}		
	}

}

zpt::couchdb::Client::~Client() {
	for (auto _socket : this->__sockets) {
		_socket->close();
	}
	for (auto _mtx : this->__mtxs) {
		delete _mtx;
	}
}

auto zpt::couchdb::Client::name() -> std::string {
	return ((std::string) this->connection()["bind"]) + std::string("/") + ((std::string) this->connection()["db"]);
}

auto zpt::couchdb::Client::options() -> zpt::json{
	return this->__options;
}

auto zpt::couchdb::Client::events(zpt::ev::emitter _emitter) -> void {
	this->__events = _emitter;
}

auto zpt::couchdb::Client::events() -> zpt::ev::emitter {
	return this->__events;
}

auto zpt::couchdb::Client::connect() -> void {
	zpt::Connector::connect();
}

auto zpt::couchdb::Client::reconnect() -> void {
	zpt::Connector::reconnect();
}

auto zpt::couchdb::Client::send(zpt::http::req _req) -> zpt::http::rep {
	bool _is_ssl = this->connection()["uri"]["scheme"] == zpt::json::string("https");
	zpt::http::rep _rep;
	this->init_request(_req);
	// zdbg(_req);
	if (this->__pool_size) {
		int _k = 0;
		{ std::lock_guard< std::mutex> _l(this->__global);
			_k = this->__round_robin++;
			if (this->__round_robin == this->__sockets.size()) {
				this->__round_robin = 0;
			} }

		{ std::lock_guard< std::mutex> _l(*this->__mtxs[_k]);
			short _n_tries = 0;
			do {
				if (!this->__sockets[_k]->is_open()) {
					zdbg("couchdb: opening socket");
					this->__sockets[_k]->open(std::string(this->connection()["uri"]["domain"]), this->connection()["uri"]["port"]->ok() ? int(this->connection()["uri"]["port"]) : (_is_ssl ? 443 : 80), _is_ssl);
				}
				try {
					(*this->__sockets[_k]) << _req << flush;
					try {
						(*this->__sockets[_k]) >> _rep;
					}
					catch (zpt::SyntaxErrorException& _e) {
					}
					break;
				}
				catch(std::exception& _e) {
					this->__sockets[_k]->close();
				}
				_n_tries++;
			}
			while(_n_tries != 5);
		}
	}
	else {
		zpt::socketstream _socket;
		_socket.open(std::string(this->connection()["uri"]["domain"]), this->connection()["uri"]["port"]->ok() ? int(this->connection()["uri"]["port"]) : (_is_ssl ? 443 : 80), _is_ssl);
		_socket << _req << flush;
		try {
			_socket >> _rep;
		}
		catch (zpt::SyntaxErrorException& _e) { }
		_socket.close();
	}
	// zdbg(_rep);
	return _rep;
}

auto zpt::couchdb::Client::init_request(zpt::http::req _req) -> void {
	_req->header("Host", std::string(this->connection()["uri"]["authority"]));
	_req->header("Accept", "*/*");
	_req->header("Connection", "keep-alive");
	if (this->connection()["user"]->ok() && this->connection()["passwd"]->ok()) {
		_req->header("Authorization", std::string("Basic ") + zpt::base64::r_encode(std::string(this->connection()["user"]) + std::string(":") + std::string(this->connection()["passwd"])));
	}
}

auto zpt::couchdb::Client::create_database(std::string _collection) -> void {
	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::http::req _req;
	_req->method(zpt::ev::Put);
	_req->url(_db_name);
	
	zpt::http::rep _rep = this->send(_req);
	zpt::json _response(_rep->body());       
	assertz(_rep->status() == zpt::HTTP201 && bool(_response["ok"]) == true, std::string("could not create database:\n") + std::string(_rep), 500, 2001);
}

auto zpt::couchdb::Client::create_index(std::string _collection, zpt::json _fields) -> void {
	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection + std::string("/_index");
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::http::req _req;
	_req->method(zpt::ev::Post);
	_req->url(_db_name);
	_req->header("Content-Type", "application/json");

	for (auto _field : _fields->obj()) {
		std::string _body = std::string(zpt::json{ "index", { "fields", { zpt::array, _field.first } }, "name", (_field.first + std::string("-idx")) });
		_req->header("Content-Length", std::to_string(_body.length()));
		_req->body(_body);
		zpt::http::rep _rep = this->send(_req);
	}
}

auto zpt::couchdb::Client::insert(std::string _collection, std::string _href_prefix, zpt::json _document, zpt::json _opts) -> std::string {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	if (!_document["id"]->ok()) {
		_document << "id" << zpt::generate::r_uuid();
	}
	if (!_document["href"]->ok() && _href_prefix.length() != 0) {
		_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
	}

	_document << "_id" << _document["href"];
	
	zpt::json _exclude = (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts) : zpt::undefined);
	std::string _body = std::string(_document - _exclude);
	zpt::http::req _req;
	_req->method(zpt::ev::Post);
	_req->url(_db_name);
	_req->header("Content-Type", "application/json");
	_req->header("Content-Length", std::to_string(_body.length()));
	_req->body(_body);

	zpt::http::rep _rep = this->send(_req);	
	if (_rep->status() == zpt::HTTP404) {
		this->create_database(_collection);
		_rep = this->send(_req);
	}
	assertz(_rep->status() == zpt::HTTP201, std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") + std::string(_rep), int(_rep->status()), 1201); 
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
	return _document["id"]->str();
}

auto zpt::couchdb::Client::upsert(std::string _collection, std::string _href_prefix, zpt::json _document, zpt::json _opts) -> std::string {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	{
		if (_document["href"]->ok() || _document["id"]->ok()) {
			if (!_document["href"]->ok()) {
				_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
			}
			if (!_document["id"]->ok()) {
				zpt::json _split = zpt::split(_document["href"]->str(), "/");
				_document << "id" << _split->arr()->back();
			}
			std::string _url = _db_name + std::string("/") + zpt::url::r_encode(std::string(_document["href"]));
			zpt::json _upsert;
			zpt::http::rep _rep;
			zpt::http::req _req;
			_req->method(zpt::ev::Put);
			_req->url(_url);
			_req->header("Content-Type", "application/json");
			do {
				zpt::json _revision = this->get(_collection, std::string(_document["href"]));
				if (!_revision->ok()) {
					break;
				}
				_upsert = _revision | _document;
				if (!_upsert["id"]->ok()) {
					_upsert << "id" << zpt::generate::r_uuid();
				}
				if (!_upsert["href"]->ok() && _href_prefix.length() != 0) {
					_upsert << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _upsert["id"]->str());
				}
				_upsert << "_id" << _upsert["href"];

				zpt::json _exclude = (_opts["fields"]->is_array() ? _upsert - zpt::couchdb::get_fields(_opts) : zpt::undefined);
				std::string _body = std::string(_upsert - _exclude);
				_req->header("Content-Length", std::to_string(_body.length()));
				_req->body(_body);
				_rep = this->send(_req);
			}
			while(_rep->status() == zpt::HTTP409);
			if (_rep->status() == zpt::HTTP201) {	
				if (!bool(_opts["mutated-event"])) zpt::Connector::set(_collection, std::string(_upsert["href"]), _upsert, _opts);
				return _upsert["id"]->str();
			}
		}
	}	
	{
		if (!_document["id"]->ok()) {
			_document << "id" << zpt::generate::r_uuid();
		}
		if (!_document["href"]->ok() && _href_prefix.length() != 0) {
			_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
		}
		_document << "_id" << _document["href"];

		zpt::json _exclude = (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts) : zpt::undefined);
		std::string _body = std::string(_document - _exclude);
		zpt::http::req _req;
		_req->method(zpt::ev::Post);
		_req->url(_db_name);
		_req->header("Content-Type", "application/json");
		_req->header("Content-Length", std::to_string(_body.length()));
		_req->body(_body);

		zpt::http::rep _rep = this->send(_req);	
		if (_rep->status() == zpt::HTTP404) {
			this->create_database(_collection);
			_rep = this->send(_req);
		}
		assertz(_rep->status() == zpt::HTTP201, std::string("couldn't upsert document ") + std::string(_document["href"]) + std::string(": ") + _rep->body(), _rep->status(), 2002); 
		if (!bool(_opts["mutated-event"])) zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
	}	
	return _document["id"]->str();
}

auto zpt::couchdb::Client::save(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, std::string("'_document' must be of type JSObject"), 412, 0);
	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
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
		
		zpt::json _exclude = (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts) : zpt::undefined);
		std::string _body = std::string(_document - _exclude);
		_req->header("Content-Length", std::to_string(_body.length()));
		_req->body(_body);
		_rep = this->send(_req);
	}
	while(_rep->status() == zpt::HTTP409);
	assertz(_rep->status() == zpt::HTTP201, std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") + std::string(_rep), int(_rep->status()), 1201); 
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::save(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::couchdb::Client::set(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {
	assertz(_document->ok() && _document->type() == zpt::JSObject, std::string("'_document' must be of type JSObject"), 412, 0);
	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
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
		zpt::json _exclude = (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts) : zpt::undefined);
		std::string _body = std::string(_document - _exclude);
		_req->header("Content-Length", std::to_string(_body.length()));
		_req->body(_body);
		_rep = this->send(_req);
	}
	while(_rep->status() == zpt::HTTP409);
	assertz(_rep->status() == zpt::HTTP201, std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") + std::string(_rep), int(_rep->status()), 1201); 
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::couchdb::Client::set(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);
	size_t _size = 0;

	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::http::req _req;
	_req->method(zpt::ev::Put);
	_req->header("Content-Type", "application/json");
	zpt::json _result = this->query(_collection, _pattern);

	for (auto _revision : _result["elements"]->arr()) {
		std::string _url = _db_name + std::string("/") + zpt::url::r_encode(std::string(_revision["href"]));

		_document = _revision | _document;
		zpt::json _exclude = (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts) : zpt::undefined);
		std::string _body = std::string(_document - _exclude);
		_req->url(_url);
		_req->header("Content-Length", std::to_string(_body.length()));
		_req->body(_body);
		zpt::http::rep _rep = this->send(_req);
		if (_rep->status() == zpt::HTTP201) {
			_size++;
		}
	}
	
	if (!bool(_opts["mutated-event"]) && _size != 0) zpt::Connector::set(_collection, _pattern, _document, _opts);
	return _size;
}

auto zpt::couchdb::Client::unset(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {
 	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
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
		zpt::json _exclude = (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts) : zpt::undefined);
		std::string _body = std::string(_document - _exclude);
		_req->header("Content-Length", std::to_string(_body.length()));
		_req->body(_body);
		_rep = this->send(_req);
	}
	while(_rep->status() == zpt::HTTP409);
	assertz(_rep->status() == zpt::HTTP201, std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") + std::string(_rep), int(_rep->status()), 1201); 
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::unset(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::couchdb::Client::unset(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);
	size_t _size = 0;

	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::http::req _req;
	_req->method(zpt::ev::Put);
	_req->header("Content-Type", "application/json");
	zpt::json _result = this->query(_collection, _pattern);

	for (auto _revision : _result["elements"]->arr()) {
		std::string _url = _db_name + std::string("/") + zpt::url::r_encode(std::string(_revision["href"]));

		_document = _revision - _document;
		zpt::json _exclude = (_opts["fields"]->is_array() ? _document - zpt::couchdb::get_fields(_opts) : zpt::undefined);
		std::string _body = std::string(_document - _exclude);
		_req->url(_url);
		_req->header("Content-Length", std::to_string(_body.length()));
		_req->body(_body);
		zpt::http::rep _rep = this->send(_req);
		if (_rep->status() == zpt::HTTP201) {
			_size++;
		}
	}
	
	if (!bool(_opts["mutated-event"]) && _size != 0) zpt::Connector::unset(_collection, _pattern, _document, _opts);
	return _size;
}

auto zpt::couchdb::Client::remove(std::string _collection, std::string _href, zpt::json _opts) -> int {	
	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
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
	}
	while(_rep->status() == zpt::HTTP409);
	assertz(_rep->status() == zpt::HTTP200, std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") + std::string(_rep), int(_rep->status()), 1201); 
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::remove(_collection, _href, _opts + zpt::json{ "removed", _revision });
	return 1;
}

auto zpt::couchdb::Client::remove(std::string _collection, zpt::json _pattern, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);
	size_t _size = 0;

	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::http::req _req;
	_req->method(zpt::ev::Delete);
	zpt::json _result = this->query(_collection, _pattern);
	zpt::json _removed = zpt::json::array();

	for (auto _record : _result["elements"]->arr()) {
		std::string _url = _db_name + std::string("/") + zpt::url::r_encode(std::string(_record["href"]));
		_req->url(_url);
		_req->param("rev", std::string(_record["_rev"]));
		zpt::http::rep _rep = this->send(_req);
		if (_rep->status() == zpt::HTTP200) {
			_size++;
			_removed << _record;
		}
	}
	
	if (!bool(_opts["mutated-event"]) && _size != 0) zpt::Connector::remove(_collection, _pattern, _opts + zpt::json{ "removed", _removed });
	return _size;
}

auto zpt::couchdb::Client::get(std::string _collection, std::string _href, zpt::json _opts) -> zpt::json {
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
 	assertz(_href.length() != 0, "'href' parameter must not be empty", 0, 0);

	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	std::string _url = _db_name + std::string("/") + zpt::url::r_encode(_href);
	zpt::http::req _req;
	_req->method(zpt::ev::Get);
	_req->url(_url);
	zpt::http::rep _rep = this->send(_req);
	assertz(_rep->status() == zpt::HTTP200 || _rep->status() == zpt::HTTP404, std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") + std::string(_rep), int(_rep->status()), 1201); 

	if (!bool(_opts["mutated-event"])) zpt::Connector::get(_collection, _href, _opts);

	if (_rep->status() == zpt::HTTP200) {
		return zpt::json(_rep->body());
	}
 	return zpt::undefined;
}

auto zpt::couchdb::Client::query(std::string _collection, std::string _regexp, zpt::json _opts) -> zpt::json {
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
 	assertz(_regexp.length() != 0, "'_regexp' parameter must not be empty", 0, 0);
	return this->query(_collection, zpt::json(_regexp), _opts);
}

auto zpt::couchdb::Client::query(std::string _collection, zpt::json _regexp, zpt::json _opts) -> zpt::json {
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::json _query = zpt::couchdb::get_query(_regexp);
	if (!_query["selector"]->ok() || (_query["selector"]->is_object() && _query["selector"]->obj()->size() == 0)) {
		return this->all(_collection, _opts);
	}
	std::string _body = std::string(_query);
	zpt::http::req _req;
	size_t _size = 0;

	_req->method(zpt::ev::Post);
	_req->url(_db_name + std::string("/_find"));
	_req->header("Content-Type", "application/json");
	_req->header("Content-Length", std::to_string(_body.length()));
	_req->body(_body);

	zpt::http::rep _rep = this->send(_req);
	assertz(_rep->status() == zpt::HTTP200 || _rep->status() == zpt::HTTP404, std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") + std::string(_rep), int(_rep->status()), 1201); 

	zpt::json _result(_rep->body());
	zpt::json _return = zpt::json::array();
	if (_result["docs"]->is_array()) {
		_return = _result["docs"];
		_size = _result["total_rows"]->ok() ? size_t(_result["total_rows"]) : (_return->is_array() ? _return->arr()->size() : 0);
	}
	else if (_result["rows"]->is_array()) {
		_size = size_t(_result["total_rows"]);
		_return = _result->getPath("rows.*.doc");
	}

	if (!bool(_opts["mutated-event"])) zpt::Connector::query(_collection, _regexp, _opts);
	if (_size == 0) {
		return zpt::undefined;
	}
	return {
		"size", _size,
		"elements", _return
	};
}

auto zpt::couchdb::Client::all(std::string _collection, zpt::json _opts) -> zpt::json {
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
	std::string _db_name = std::string("/") + std::string(this->connection()["db"]) + std::string("_") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::http::req _req;		
	_req->method(zpt::ev::Get);
	_req->url(_db_name + std::string("/_all_docs"));
	_req->param("include_docs", "true");
	if (_opts["page_size"]->ok()) {
		_req->param("limit", std::string(_opts["page_size"]));
	}
	if (_opts["page_start_index"]->ok()) {
		_req->param("skip", std::string(_opts["page_start_index"]));
	}

	zpt::http::rep _rep = this->send(_req);
	assertz(_rep->status() == zpt::HTTP200, std::string("couchdb: error in request:\n") + std::string(_req) + std::string("\n") + std::string(_rep), int(_rep->status()), 1201); 

	zpt::json _result(_rep->body());
	zpt::json _return = zpt::json::array();
	size_t _size = 0;
	if (_result["docs"]->is_array()) {
		_size = size_t(_result["total_rows"]);
		_return = _result["docs"];
	}
	else if (_result["rows"]->is_array()) {
		_size = size_t(_result["total_rows"]);
		_return = _result->getPath("rows.*.doc");
	}
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::all(_collection, _opts);
	if (_size == 0) {
		return zpt::undefined;
	}
	return {
		"size", _size,
		"elements", _return
	};
}

extern "C" auto zpt_couchdb() -> int {
	return 1;
}
