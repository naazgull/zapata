/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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

zpt::couchdb::Client::Client(zpt::json _options, std::string _conf_path) : __options( _options), __socket(new zpt::socketstream()) {
	try {
		zpt::json _uri = zpt::uri::parse((std::string) _options->getPath(_conf_path)["bind"]);
		if (_uri["scheme"] == zpt::json::string("zpt")) {
			_uri << "scheme" << "http";
		}
		this->connection(_options->getPath(_conf_path) + zpt::json{ "uri", _uri });
		zdbg(this->connection());
	}
	catch(std::exception& _e) {
		assertz(false, std::string("could not connect to CouchDB server: ") + _e.what(), 500, 0);
	}
}

zpt::couchdb::Client::~Client() {
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

auto zpt::couchdb::Client::mutations(zpt::mutation::emitter _emitter) -> void {
}

auto zpt::couchdb::Client::mutations() -> zpt::mutation::emitter {
	return this->__events->mutations();
}

auto zpt::couchdb::Client::connect() -> void {
	zpt::Connector::connect();
}

auto zpt::couchdb::Client::reconnect() -> void {
	zpt::Connector::reconnect();
}

auto zpt::couchdb::Client::socket() -> zpt::socketstream_ptr {
	if (!this->__socket->is_open()) {
		zdbg(this->connection()["uri"]);
		this->__socket->open(std::string(this->connection()["uri"]["domain"]), this->connection()["uri"]["port"]->ok() ? int(this->connection()["uri"]["port"]) : 80);
	}
	return this->__socket;
}

auto zpt::couchdb::Client::send() -> zpt::http::rep {
	zpt::http::rep _r;
	return _r;
}

auto zpt::couchdb::Client::insert(std::string _collection, std::string _href_prefix, zpt::json _document, zpt::json _opts) -> std::string {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	if (!_document["id"]->ok()) {
		_document << "id" << zpt::generate::r_uuid();
	}
	if (!_document["href"]->ok() && _href_prefix.length() != 0) {
		_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
	}

	_document << "_id" << _document["id"];

	std::string _body = std::string(_document);

	zpt::http::req _req;
	zpt::http::req _rep;
	_req->method(zpt::ev::Post);
	_req->url(_href_prefix);
	_req->header("Host", std::string(this->connection()["uri"]["authority"]));
	_req->header("Accept", "*/*");
	_req->header("User-Agent", "zapata HTTP");
	_req->header("Content-Type", "application/json");
	_req->header("Content-Length", std::to_string(_body.length()));
	_req->body(_document);
	
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		zpt::socketstream_ptr _socket = this->socket();
		(*_socket) << _req << flush;
		zdbg(_req);
		(*_socket) >> _rep; }

	zdbg(_rep);
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
	return _document["id"]->str();
}

auto zpt::couchdb::Client::save(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
	std::string _key(_collection);
	_key.insert(0, "/");
	_key.insert(0, (std::string) this->connection()["db"]);
	
	assertz(_document->ok() && _document->type() == zpt::JSObject, std::string(_key) + std::string(" > ") + _href + std::string(": '_document' must be of type JSObject"), 412, 0);

	if (!bool(_opts["mutated-event"])) zpt::Connector::save(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::couchdb::Client::set(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
 	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

 	zpt::json _record = this->get(_collection, _href);
	for (auto _field : _document->obj()) {
		_record <<  _field.first << _field.second;
	}

	if (!bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _href, _document, _opts);
 	return 1;
}

auto zpt::couchdb::Client::set(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);
	size_t _size = 0;
	
	if (!bool(_opts["mutated-event"]) && _size != 0) zpt::Connector::set(_collection, _pattern, _document, _opts);
	return _size;
}

auto zpt::couchdb::Client::unset(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {
 	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

	if (!bool(_opts["mutated-event"])) zpt::Connector::unset(_collection, _href, _document, _opts);
 	return 1;
}

auto zpt::couchdb::Client::unset(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);
	size_t _size = 0;

	if (!bool(_opts["mutated-event"]) && _size != 0) zpt::Connector::unset(_collection, _pattern, _document, _opts);
	return _size;
}

auto zpt::couchdb::Client::remove(std::string _collection, std::string _href, zpt::json _opts) -> int {	
 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

	if (!bool(_opts["mutated-event"])) zpt::Connector::remove(_collection, _href, _opts);
 	return 1;
}

auto zpt::couchdb::Client::remove(std::string _collection, zpt::json _pattern, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);
 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);
	size_t _size = 0;
	
	return _size;
}

auto zpt::couchdb::Client::get(std::string _collection, std::string _href, zpt::json _opts) -> zpt::json {
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
 	assertz(_href.length() != 0, "'href' parameter must not be empty", 0, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::get(_collection, _href, _opts);
 	return zpt::undefined;
}

auto zpt::couchdb::Client::query(std::string _collection, std::string _regexp, zpt::json _opts) -> zpt::json {
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
 	assertz(_regexp.length() != 0, "'_regexp' parameter must not be empty", 0, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

	zpt::JSONArr _return;

	if (!bool(_opts["mutated-event"])) zpt::Connector::query(_collection, _regexp, _opts);
	return {
		"size", _return->size(),
		"elements", _return
	};
}

auto zpt::couchdb::Client::query(std::string _collection, zpt::json _regexp, zpt::json _opts) -> zpt::json {
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

	zpt::JSONArr _return;

	if (!bool(_opts["mutated-event"])) zpt::Connector::query(_collection, _regexp, _opts);
	return {
		"size", _return->size(),
		"elements", _return
	};
}

auto zpt::couchdb::Client::all(std::string _collection, zpt::json _opts) -> zpt::json {
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

	zpt::JSONArr _return;

	if (!bool(_opts["mutated-event"])) zpt::Connector::all(_collection, _opts);
	return {
		"size", _return->size(),
		"elements", _return
	};
}

extern "C" auto zpt_couchdb() -> int {
	return 1;
}
