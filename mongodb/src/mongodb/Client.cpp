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

#include <zapata/mongodb/Client.h>

zpt::mongodb::ClientPtr::ClientPtr(zpt::mongodb::Client * _target) : std::shared_ptr<zpt::mongodb::Client>(_target) {
}

zpt::mongodb::ClientPtr::ClientPtr(zpt::json _options, std::string _conf_path) : std::shared_ptr<zpt::mongodb::Client>(new zpt::mongodb::Client(_options, _conf_path)) {
}

zpt::mongodb::ClientPtr::~ClientPtr() {
}

zpt::mongodb::Client::Client(zpt::json _options, std::string _conf_path) : __options( _options), __mongodb_conf(_options->getPath(_conf_path)), __conf_path(_conf_path), __conn((string) __mongodb_conf["bind"]), __broadcast(true), __addons(new zpt::Addons(_options)) {
	if (this->__mongodb_conf["user"]->ok()) {
		this->__conn->auth(BSON("mechanism" << "MONGODB-CR" << "user" << (string) this->__mongodb_conf["user"] << "pwd" << (string) this->__mongodb_conf["passwd"] << "db" << (string) this->__mongodb_conf["db"]));
	}
	this->__conn->setWriteConcern((mongo::WriteConcern) 2);
}

zpt::mongodb::Client::~Client() {
	this->__conn.done();
}

zpt::json zpt::mongodb::Client::options() {
	return this->__options;
}

std::string zpt::mongodb::Client::name() {
	return string("mongodb://") + ((string) this->__mongodb_conf["bind"]) + string(":") + ((string) this->__mongodb_conf["port"]) + string("/") + ((string) this->__mongodb_conf["db"]);
}

bool& zpt::mongodb::Client::broadcast() {
	return this->__broadcast;
}

zpt::ev::emitter zpt::mongodb::Client::addons() {
	return this->__addons;
}

std::string zpt::mongodb::Client::insert(std::string _collection, std::string _id_prefix, zpt::json _document) {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__mongodb_conf["db"]);

	if (!_document["id"]->ok()) {
		uuid _uuid;
		_uuid.make(UUID_MAKE_V1);
		_document << "id" << _uuid.string();
	}
	if (!_document["_id"]->ok() && _id_prefix.length() != 0) {
		_document << "_id" << (_id_prefix + (_id_prefix.back() != '/' ? string("/") : string("")) + _document["id"]->str());
	}
	_document << "href" << _document["_id"];

	mongo::BSONObjBuilder _mongo_document;
	zpt::mongodb::tomongo(_document, _mongo_document);
	this->__conn->insert(_full_collection, _mongo_document.obj());
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	if (this->__broadcast && this->addons().get() != nullptr) {
		this->addons()->trigger(zpt::ev::Post, string("INSERT ") + _collection, _document);
	}
	return _document["id"]->str();
}

int zpt::mongodb::Client::save(std::string _collection, zpt::json _pattern, zpt::json _document) {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	if (!_pattern->ok()) {
		_pattern = zpt::json::object();
	}

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__mongodb_conf["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zpt::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	mongo::BSONObjBuilder _mongo_document;
	zpt::mongodb::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, _filter, _mongo_document.obj(), false, false);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	if (this->__broadcast) {
		this->addons()->trigger(zpt::ev::Post, string("UPDATE ") + _collection, _pattern);
	}
	return _size;
}

int zpt::mongodb::Client::set(std::string _collection, zpt::json _pattern, zpt::json _document) {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	if (!_pattern->ok()) {
		_pattern = zpt::json::object();
	}

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__mongodb_conf["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zpt::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	_document = { "$set", _document };
	mongo::BSONObjBuilder _mongo_document;
	zpt::mongodb::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, _filter, _mongo_document.obj(), false, true);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	if (this->__broadcast) {
		this->addons()->trigger(zpt::ev::Post, string("UPDATE ") + _collection, _pattern);
	}
	return _size;
}

int zpt::mongodb::Client::unset(std::string _collection, zpt::json _pattern, zpt::json _document) {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	if (!_pattern->ok()) {
		_pattern = zpt::json::object();
	}

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__mongodb_conf["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zpt::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	_document = { "$unset", _document };
	mongo::BSONObjBuilder _mongo_document;
	zpt::mongodb::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, _filter, _mongo_document.obj(), false, true);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	if (this->__broadcast) {
		this->addons()->trigger(zpt::ev::Post, string("UPDATE ") + _collection, _pattern);
	}
	return _size;
}

int zpt::mongodb::Client::remove(std::string _collection, zpt::json _pattern) {
	if (!_pattern->ok()) {
		_pattern = zpt::json::object();
	}

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__mongodb_conf["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zpt::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	this->__conn->remove(_full_collection, _filter);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	if (this->__broadcast) {
		this->addons()->trigger(zpt::ev::Post, string("DELETE ") + _collection, _pattern);
	}
	return _size;
}

zpt::json zpt::mongodb::Client::query(std::string _collection, zpt::json _pattern) {
	if (!_pattern->ok()) {
		_pattern = zpt::json::object();
	}

	zpt::JSONArr _elements;

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__mongodb_conf["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zpt::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);

	mongo::Query _query(_query_b.done());
	unsigned long _size = this->__conn->count(_full_collection, _query.obj, (int) mongo::QueryOption_SlaveOk);
	mongo::BSONObj _order = _order_b.done();
	if (!_order.isEmpty()) {
		_query.sort(_order);
	}
	
	std::unique_ptr<mongo::DBClientCursor> _result = this->__conn->query(_full_collection, _query, _page_size, _page_start_index, nullptr, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	while(_result->more()) {
		mongo::BSONObj _record = _result->next();
		zpt::JSONObj _obj;
		zpt::mongodb::frommongo(_record, _obj);
		_elements << _obj;
	}

	if (_elements->size() == 0) {
		return zpt::undefined;
	}
	zpt::json _return = {
		"size", _size, 
		"elements", _elements
	};
	if (_page_size != 0) {
		_return << "links" << zpt::json(
			{
				"next", (std::string("?page-size=") + std::to_string(_page_size) + std::string("&page-start-index=") + std::to_string(_page_start_index + _page_size)),
				"prev", (std::string("?page-size=") + std::to_string(_page_size) + std::string("&page-start-index=") + std::to_string(_page_size < _page_start_index ? _page_start_index - _page_size : 0))
			}
		);
	}
	return _return;
}
