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

zpt::mongodb::Client::Client(zpt::json _options, std::string _conf_path) : __options( _options), __mongodb_conf(_options->getPath(_conf_path)), __conf_path(_conf_path), __conn((std::string) __mongodb_conf["bind"]) {
	if (this->__mongodb_conf["user"]->ok()) {
		this->__conn->auth(BSON("mechanism" << "MONGODB-CR" << "user" << (std::string) this->__mongodb_conf["user"] << "pwd" << (std::string) this->__mongodb_conf["passwd"] << "db" << (std::string) this->__mongodb_conf["db"]));
	}
	this->__conn->setWriteConcern((mongo::WriteConcern) 2);
}

zpt::mongodb::Client::~Client() {
	this->__conn.done();
}

auto zpt::mongodb::Client::name() -> std::string {
	return std::string("mongodb://") + ((std::string) this->__mongodb_conf["bind"]) + std::string(":") + ((std::string) this->__mongodb_conf["port"]) + std::string("/") + ((std::string) this->__mongodb_conf["db"]);
}

auto zpt::mongodb::Client::options() -> zpt::json {
	return this->__options;
}

auto zpt::mongodb::Client::events(zpt::ev::emitter _emitter) -> void {
	this->__events = _emitter;
}

auto zpt::mongodb::Client::events() -> zpt::ev::emitter {
	return this->__events;
}

auto zpt::mongodb::Client::mutations(zpt::mutation::emitter _emitter) -> void {
}

auto zpt::mongodb::Client::mutations() -> zpt::mutation::emitter {
	return this->__events->mutations();
}

auto zpt::mongodb::Client::connect(zpt::json _opts) -> void {	
}

auto zpt::mongodb::Client::reconnect() -> void {
}

auto zpt::mongodb::Client::insert(std::string _collection, std::string _href_prefix, zpt::json _document, zpt::json _opts) -> std::string {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn->resetError();

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (std::string) this->__mongodb_conf["db"]);

	if (!_document["id"]->ok()) {
		uuid _uuid;
		_uuid.make(UUID_MAKE_V1);
		_document << "id" << _uuid.string();
	}
	if (!_document["href"]->ok() && _href_prefix.length() != 0) {
		_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
	}
	_document << "_id" << _document["href"];

	mongo::BSONObjBuilder _mongo_document;
	zpt::mongodb::tomongo(_document, _mongo_document);
	this->__conn->insert(_full_collection, _mongo_document.obj());
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
	return _document["id"]->str();
}

auto zpt::mongodb::Client::save(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn->resetError();

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (std::string) this->__mongodb_conf["db"]);

	mongo::BSONObjBuilder _mongo_document;
	zpt::mongodb::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, BSON( "_id" << _href ), _mongo_document.obj(), false, false);
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	zpt::Connector::save(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::mongodb::Client::set(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn->resetError();

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (std::string) this->__mongodb_conf["db"]);

	_document = { "$set", _document };
	mongo::BSONObjBuilder _mongo_document;
	zpt::mongodb::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, BSON( "_id" << _href ), _mongo_document.obj(), false, false);
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	zpt::Connector::set(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::mongodb::Client::set(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn->resetError();
	if (!_pattern->ok()) {
		_pattern = zpt::json::object();
	}

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (std::string) this->__mongodb_conf["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zpt::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	_document = { "$set", _document };
	mongo::BSONObjBuilder _mongo_document;
	zpt::mongodb::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, _filter, _mongo_document.obj(), false, bool(_opts["multi"]));
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	zpt::Connector::set(_collection, _pattern, _document, _opts);
	return _size;
}

auto zpt::mongodb::Client::unset(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn->resetError();

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (std::string) this->__mongodb_conf["db"]);

	_document = { "$unset", _document };
	mongo::BSONObjBuilder _mongo_document;
	zpt::mongodb::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, BSON( "_id" << _href ), _mongo_document.obj(), false, false);
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	zpt::Connector::unset(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::mongodb::Client::unset(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn->resetError();
	if (!_pattern->ok()) {
		_pattern = zpt::json::object();
	}

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (std::string) this->__mongodb_conf["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zpt::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	_document = { "$unset", _document };
	mongo::BSONObjBuilder _mongo_document;
	zpt::mongodb::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, _filter, _mongo_document.obj(), false, bool(_opts["multi"]));
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	zpt::Connector::unset(_collection, _pattern, _document, _opts);
	return _size;
}

auto zpt::mongodb::Client::remove(std::string _collection, std::string _href, zpt::json _opts) -> int {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn->resetError();

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (std::string) this->__mongodb_conf["db"]);

	this->__conn->remove(_full_collection, BSON( "_id" << _href ));
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	zpt::Connector::remove(_collection, _href, _opts);
	return 1;
}

auto zpt::mongodb::Client::remove(std::string _collection, zpt::json _pattern, zpt::json _opts) -> int {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn->resetError();
	if (!_pattern->ok()) {
		_pattern = zpt::json::object();
	}

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (std::string) this->__mongodb_conf["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zpt::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	this->__conn->remove(_full_collection, _filter);
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	zpt::Connector::remove(_collection, _pattern, _opts);
	return _size;
}

auto zpt::mongodb::Client::get(std::string _collection, std::string _topic, zpt::json _opts) -> zpt::json {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn->resetError();

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (std::string) this->__mongodb_conf["db"]);

	std::unique_ptr<mongo::DBClientCursor> _result = this->__conn->query(_full_collection, BSON( "_id" << _topic ), 0, 0, nullptr, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	if (_result->more()) {
		mongo::BSONObj _record = _result->next();
		zpt::json _obj = zpt::json::object();
		zpt::mongodb::frommongo(_record, _obj);
		return _obj;
	}

	return zpt::undefined;
}

auto zpt::mongodb::Client::query(std::string _collection, std::string _pattern, zpt::json _opts) -> zpt::json {
	return this->query(_collection, zpt::json(_pattern), _opts);
}

auto zpt::mongodb::Client::query(std::string _collection, zpt::json _pattern, zpt::json _opts) -> zpt::json {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn->resetError();
	if (!_pattern->ok()) {
		_pattern = zpt::json::object();
	}

	zpt::JSONArr _elements;

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (std::string) this->__mongodb_conf["db"]);

	size_t _page_size = size_t(_opts["page-size"]);
	size_t _page_start_index = size_t(_opts["page-start-index"]);
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zpt::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);

	if (_opts["order-by"]->ok()) {
		std::istringstream lss(((std::string) _opts["order-by"]).data());
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
					oss << _part << flush;

					_order_b.append(oss.str(), _dir);
				}
			}
		}
	}

	mongo::Query _query(_query_b.done());
	unsigned long _size = this->__conn->count(_full_collection, _query.obj, (int) mongo::QueryOption_SlaveOk);
	mongo::BSONObj _order = _order_b.done();
	if (!_order.isEmpty()) {
		_query.sort(_order);
	}
	
	std::unique_ptr<mongo::DBClientCursor> _result = this->__conn->query(_full_collection, _query, _page_size, _page_start_index, nullptr, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

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

auto zpt::mongodb::Client::all(std::string _collection, zpt::json _opts) -> zpt::json {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn->resetError();
	zpt::JSONArr _elements;

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (std::string) this->__mongodb_conf["db"]);

	size_t _page_size = size_t(_opts["page-size"]);
	size_t _page_start_index = size_t(_opts["page-start-index"]);
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;

	if (_opts["order-by"]->ok()) {
		std::istringstream lss(((std::string) _opts["order-by"]).data());
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
					oss << _part << flush;

					_order_b.append(oss.str(), _dir);
				}
			}
		}
	}

	mongo::Query _query(_query_b.done());
	unsigned long _size = this->__conn->count(_full_collection, _query.obj, (int) mongo::QueryOption_SlaveOk);
	mongo::BSONObj _order = _order_b.done();
	if (!_order.isEmpty()) {
		_query.sort(_order);
	}
	
	std::unique_ptr<mongo::DBClientCursor> _result = this->__conn->query(_full_collection, _query, _page_size, _page_start_index, nullptr, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, std::string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

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
