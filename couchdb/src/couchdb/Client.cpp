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
	zpt::socketstream_ptr _socket(new zpt::socketstream());
	_socket->open(std::string(this->connection()["uri"]["domain"]), this->connection()["uri"]["port"]->ok() ? int(this->connection()["uri"]["port"]) : 80);
	return _socket;

}

auto zpt::couchdb::Client::send(zpt::http::req _req) -> zpt::http::rep {
	zpt::http::rep _rep;
	this->init_request(_req);
	zpt::socketstream_ptr _socket = this->socket();
	(*_socket) << _req << flush;
	try {
		(*_socket) >> _rep;
	}
	catch (zpt::SyntaxErrorException& _e) {
	}
	_socket->close();
	return _rep;
}

auto zpt::couchdb::Client::init_request(zpt::http::req _req) -> void {
	_req->header("Host", std::string(this->connection()["uri"]["authority"]));
	_req->header("Accept", "*/*");
	if (this->connection()["user"]->ok() && this->connection()["passwd"]->ok()) {
		_req->header("Authorization", std::string("Basic ") + zpt::base64::r_encode(std::string(this->connection()["user"]) + std::string(":") + std::string(this->connection()["passwd"])));
	}
}

auto zpt::couchdb::Client::create_database(std::string _collection) -> void {
	std::string _db_name = std::string("/") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::http::req _req;
	_req->method(zpt::ev::Put);
	_req->url(_db_name);
	
	zpt::http::rep _rep = this->send(_req);
	zpt::json _response(_rep->body());       
	assertz(_rep->status() == zpt::HTTP201 && bool(_response["ok"]) == true, std::string("could not create database:\n") + std::string(_rep), 500, 2001);
}

auto zpt::couchdb::Client::create_index(std::string _collection, zpt::json _fields) -> void {
	std::string _db_name = std::string("/") + _collection + std::string("/_index");
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
	std::string _db_name = std::string("/") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	if (!_document["id"]->ok()) {
		_document << "id" << zpt::generate::r_uuid();
	}
	if (!_document["href"]->ok() && _href_prefix.length() != 0) {
		_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
	}

	_document << "_id" << _document["href"];

	std::string _body = std::string(_document);
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
	assertz(_rep->status() == zpt::HTTP201, std::string("couldn't insert document:\n") + std::string(_rep), _rep->status(), 2002); 
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
	return _document["id"]->str();
}

auto zpt::couchdb::Client::save(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, std::string("'_document' must be of type JSObject"), 412, 0);
	std::string _db_name = std::string("/") + _collection;
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
		
		std::string _body = std::string(_document);
		_req->header("Content-Length", std::to_string(_body.length()));
		_req->body(_body);
		_rep = this->send(_req);
	}
	while(_rep->status() == zpt::HTTP409);
	assertz(_rep->status() == zpt::HTTP201, std::string("couldn't save document:\n") + std::string(_rep), _rep->status(), 2002); 
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::save(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::couchdb::Client::set(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, std::string("'_document' must be of type JSObject"), 412, 0);
	std::string _db_name = std::string("/") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	std::string _url = _db_name + std::string("/") + zpt::url::r_encode(_href);
	zpt::http::req _req;
	_req->method(zpt::ev::Put);
	_req->url(_url);
	_req->header("Content-Type", "application/json");
	zpt::http::rep _rep;
	do {
		zpt::json _revision = this->get(_collection, _href);		
		std::string _body = std::string(zpt::json(_revision + _document));
		_req->header("Content-Length", std::to_string(_body.length()));
		_req->body(_body);
		_rep = this->send(_req);
	}
	while(_rep->status() == zpt::HTTP409);
	assertz(_rep->status() == zpt::HTTP201, std::string("couldn't set document fields:\n") + std::string(_rep), _rep->status(), 2002); 
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::couchdb::Client::set(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);
	size_t _size = 0;

	std::string _db_name = std::string("/") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::http::req _req;
	_req->method(zpt::ev::Put);
	_req->header("Content-Type", "application/json");
	zpt::json _result = this->query(_collection, _pattern);

	for (auto _record : _result["elements"]->arr()) {
		std::string _url = _db_name + std::string("/") + zpt::url::r_encode(std::string(_record["href"]));
		std::string _body = std::string(zpt::json(_record + _document));
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
	std::string _db_name = std::string("/") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	std::string _url = _db_name + std::string("/") + zpt::url::r_encode(_href);
	zpt::http::req _req;
	_req->method(zpt::ev::Put);
	_req->url(_url);
	_req->header("Content-Type", "application/json");
	zpt::http::rep _rep;
	do {
		zpt::json _revision = this->get(_collection, _href);		
		std::string _body = std::string(zpt::json(_revision - _document));
		_req->header("Content-Length", std::to_string(_body.length()));
		_req->body(_body);
		_rep = this->send(_req);
	}
	while(_rep->status() == zpt::HTTP409);
	assertz(_rep->status() == zpt::HTTP201, std::string("couldn't unset document fields:\n") + std::string(_rep), _rep->status(), 2002); 
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::unset(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::couchdb::Client::unset(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);
	size_t _size = 0;

	std::string _db_name = std::string("/") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::http::req _req;
	_req->method(zpt::ev::Put);
	_req->header("Content-Type", "application/json");
	zpt::json _result = this->query(_collection, _pattern);

	for (auto _record : _result["elements"]->arr()) {
		std::string _url = _db_name + std::string("/") + zpt::url::r_encode(std::string(_record["href"]));
		std::string _body = std::string(zpt::json(_record - _document));
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
	std::string _db_name = std::string("/") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	std::string _url = _db_name + std::string("/") + zpt::url::r_encode(_href);
	zpt::http::req _req;
	_req->method(zpt::ev::Delete);
	_req->url(_url);
	zpt::http::rep _rep;
	do {
		zpt::json _revision = this->get(_collection, _href);
		_req->param("rev", std::string(_revision["_rev"]));
		_rep = this->send(_req);
	}
	while(_rep->status() == zpt::HTTP409);
	assertz(_rep->status() == zpt::HTTP200, std::string("couldn't remove document:\n") + std::string(_rep), _rep->status(), 2002); 
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::remove(_collection, _href, _opts);
	return 1;
}

auto zpt::couchdb::Client::remove(std::string _collection, zpt::json _pattern, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);
	size_t _size = 0;

	std::string _db_name = std::string("/") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::http::req _req;
	_req->method(zpt::ev::Delete);
	zpt::json _result = this->query(_collection, _pattern);

	for (auto _record : _result["elements"]->arr()) {
		std::string _url = _db_name + std::string("/") + zpt::url::r_encode(std::string(_record["href"]));
		_req->url(_url);
		_req->param("rev", std::string(_record["_rev"]));
		zpt::http::rep _rep = this->send(_req);
		if (_rep->status() == zpt::HTTP200) {
			_size++;
		}
	}
	
	if (!bool(_opts["mutated-event"]) && _size != 0) zpt::Connector::remove(_collection, _pattern, _opts);
	return _size;
}

auto zpt::couchdb::Client::get(std::string _collection, std::string _href, zpt::json _opts) -> zpt::json {
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
 	assertz(_href.length() != 0, "'href' parameter must not be empty", 0, 0);

	std::string _db_name = std::string("/") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	std::string _url = _db_name + std::string("/") + zpt::url::r_encode(_href);
	zpt::http::req _req;
	_req->method(zpt::ev::Get);
	_req->url(_url);
	zpt::http::rep _rep = this->send(_req);

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
	std::string _db_name = std::string("/") + _collection;
	std::transform(_db_name.begin(), _db_name.end(), _db_name.begin(), ::tolower);

	zpt::json _query = zpt::couchdb::get_query(_regexp);
	std::string _body = std::string(_query);
	zpt::http::req _req;
	size_t _size = 0;
	if (_query["selector"]->is_object() && _query["selector"]->obj()->size() != 0) {
		zpt::http::req _req_count;
		_req_count->method(zpt::ev::Get);
		_req_count->url(_db_name + std::string("/_all_docs"));
		_req_count->param("limit", "0");
		zpt::http::rep _rep_count = this->send(_req_count);
		zpt::json _count(_rep_count->body());
		_size = size_t(_count["total_rows"]);
		
		_req->method(zpt::ev::Post);
		_req->url(_db_name + std::string("/_find"));
		_req->header("Content-Type", "application/json");
		_req->header("Content-Length", std::to_string(_body.length()));
		_req->body(_body);
	}
	else {
		_req->method(zpt::ev::Get);
		_req->url(_db_name + std::string("/_all_docs"));
		_req->param("include_docs", "true");
		if (_query->is_object()) {
			for (auto _param : _query->obj()) {
				_req->param(_param.first, std::string(_param.second));
			}
		}
	}

	zpt::http::rep _rep = this->send(_req);
	zpt::json _result(_rep->body());
	zpt::JSONArr _return;
	if (_result["docs"]->is_array()) {
		_return = _result["docs"]->arr();
	}
	else if (_result["rows"]->is_array()) {
		_size = size_t(_result["total_rows"]);
		_return = _result["rows"]->arr();
	}
	else {
		_return = zpt::json::array();
	}

	if (!bool(_opts["mutated-event"])) zpt::Connector::query(_collection, _regexp, _opts);
	return {
		"size", _size,
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
