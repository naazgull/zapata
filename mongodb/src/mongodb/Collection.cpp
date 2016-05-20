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

#include <zapata/mongodb/Collection.h>

zapata::mongodb::CollectionPtr::CollectionPtr(zapata::mongodb::Collection * _target) : std::shared_ptr<zapata::mongodb::Collection>(_target) {
}

zapata::mongodb::CollectionPtr::CollectionPtr(zapata::JSONPtr _options) : std::shared_ptr<zapata::mongodb::Collection>(new zapata::mongodb::Collection(_options)) {
}

zapata::mongodb::CollectionPtr::~CollectionPtr() {
}

zapata::mongodb::Collection::Collection(zapata::JSONPtr _options) : __options( _options), __conn((string) _options["mongodb"]["bind"]), __broadcast(true), __addons(new zapata::Addons(_options)) {
	if (this->__options["mongodb"]["user"]->ok()) {
		this->__conn->auth(BSON("mechanism" << "MONGODB-CR" << "user" << (string) this->__options["mongodb"]["user"] << "pwd" << (string) this->__options["mongodb"]["passwd"] << "db" << (string) this->__options["mongodb"]["db"]));
	}
	this->__conn->setWriteConcern((mongo::WriteConcern) 2);

}

zapata::mongodb::Collection::~Collection() {
	this->__conn.done();
}

zapata::JSONPtr zapata::mongodb::Collection::options() {
	return this->__options;
}

std::string zapata::mongodb::Collection::name() {
	return string("mongodb://") + ((string) this->__options["mongodb"]["bind"]) + string(":") + ((string) this->__options["mongodb"]["port"]) + string("/") + ((string) this->__options["mongodb"]["db"]);
}

bool& zapata::mongodb::Collection::broadcast() {
	return this->__broadcast;
}

zapata::EventEmitterPtr zapata::mongodb::Collection::addons() {
	return this->__addons;
}

std::string zapata::mongodb::Collection::insert(std::string _collection, std::string _id_prefix, zapata::JSONPtr _document) {	
	assertz(_document->ok() && _document->type() == zapata::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__options["mongodb"]["db"]);

	if (!_document["id"]->ok()) {
		uuid _uuid;
		_uuid.make(UUID_MAKE_V1);
		_document << "id" << _uuid.string();
	}
	_document << "_id" << (_id_prefix + (_id_prefix.back() != '/' ? string("/") : string("")) + _document["id"]->str());
	_document << "href" << _document["_id"];

	mongo::BSONObjBuilder _mongo_document;
	zapata::mongodb::tomongo(_document, _mongo_document);
	this->__conn->insert(_full_collection, _mongo_document.obj());
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	if (this->__broadcast) {
		this->addons()->trigger(zapata::ev::Post, string("INSERT ") + _collection, _document);
	}
	return _document["id"]->str();
}

int zapata::mongodb::Collection::update(std::string _collection, zapata::JSONPtr _pattern, zapata::JSONPtr _document) {	
	assertz(_document->ok() && _document->type() == zapata::JSObject, "'_document' must be of type JSObject", 412, 0);
	if (!_pattern->ok()) {
		_pattern = zapata::mkobj();
	}

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__options["mongodb"]["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	_document = zapata::mkptr(JSON("$set" << _document));
	mongo::BSONObjBuilder _mongo_document;
	zapata::mongodb::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, _filter, _mongo_document.obj(), false, true);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	if (this->__broadcast) {
		this->addons()->trigger(zapata::ev::Post, string("UPDATE ") + _collection, _pattern);
	}
	return _size;
}

int zapata::mongodb::Collection::unset(std::string _collection, zapata::JSONPtr _pattern, zapata::JSONPtr _document) {
	assertz(_document->ok() && _document->type() == zapata::JSObject, "'_document' must be of type JSObject", 412, 0);
	if (!_pattern->ok()) {
		_pattern = zapata::mkobj();
	}

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__options["mongodb"]["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	_document = zapata::mkptr(JSON("$unset" << _document));
	mongo::BSONObjBuilder _mongo_document;
	zapata::mongodb::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, _filter, _mongo_document.obj(), false, true);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	if (this->__broadcast) {
		this->addons()->trigger(zapata::ev::Post, string("UPDATE ") + _collection, _pattern);
	}
	return _size;
}

int zapata::mongodb::Collection::remove(std::string _collection, zapata::JSONPtr _pattern) {
	if (!_pattern->ok()) {
		_pattern = zapata::mkobj();
	}

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__options["mongodb"]["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	this->__conn->remove(_full_collection, _filter);
	assertz(this->__conn->getLastError().length() == 0, string("mongodb operation returned an error: ") + this->__conn->getLastError(), 500, 0);

	if (this->__broadcast) {
		this->addons()->trigger(zapata::ev::Post, string("DELETE ") + _collection, _pattern);
	}
	return _size;
}

zapata::JSONPtr zapata::mongodb::Collection::query(std::string _collection, zapata::JSONPtr _pattern) {
	if (!_pattern->ok()) {
		_pattern = zapata::mkobj();
	}

	zapata::JSONArr _return;

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__options["mongodb"]["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::mongodb::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);

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
		zapata::JSONObj _obj;
		zapata::mongodb::frommongo(_record, _obj);
		_return << _obj;
	}

	if (_return->size() == 0) {
		return zapata::undefined;
	}
	return zapata::mkptr(JSON(
		"size" << _size <<
		"elements" << _return
	));
}
