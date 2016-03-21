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

#include <zapata/redis/Collection.h>

zapata::redis::CollectionPtr::CollectionPtr(zapata::JSONObj& _options) : std::shared_ptr<zapata::redis::Collection>(new zapata::redis::Collection(_options)) {
}

zapata::redis::CollectionPtr::~CollectionPtr() {
}

zapata::redis::Collection::Collection(zapata::JSONObj& _options) : __options( _options) {

}

zapata::redis::Collection::~Collection() {
	redisFree(this->__conn);
	pthread_mutexattr_destroy(this->__attr);
	pthread_mutex_destroy(this->__mtx);
	delete this->__mtx;
	delete this->__attr;
}

zapata::JSONObj& zapata::redis::Collection::options() {
	return this->__options;
}

void zapata::redis::Collection::connect(string _host, uint _port) {
	this->__mtx = new pthread_mutex_t();
	this->__attr = new pthread_mutexattr_t();
	pthread_mutexattr_init(this->__attr);
	pthread_mutex_init(this->__mtx, this->__attr);

	this->__host.assign(_host.data());
	this->__port = _port;
	bool _success = true;
	short _retry = 0;
	do {
		_success = ((this->__redis = redisConnect(this->__host.data(), this->__port)) != nullptr);
		_retry++;
		if (!_success) {
			sleep(1);		
		}
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), muzzley::emergency);
		exit(-10);
	}
};

void zapata::redis::Collection::reconnect() {
	redisFree(this->__redis);
	bool _success = true;
	short _retry = 0;
	do {
		sleep(1);
		_success = ((this->__redis = redisConnect(this->__host.data(), this->__port)) != nullptr);
		_retry++;
	}
	while(!_success && _retry != 10);
}

std::string zapata::redis::Collection::insert(std::string _collection, std::string _id_prefix, zapata::JSONPtr _document) {	
	assertz(_document->ok() && _document->type() == zapata::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__options["mongo"]["db"]);

	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	_document << "id" << _uuid.string();
	_document << "_id" << (_id_prefix + (_id_prefix.back() != '/' ? string("/") : string("")) + _document["id"]->str());

	mongo::BSONObjBuilder _mongo_document;
	zapata::redis::tomongo(_document, _mongo_document);
	this->__conn->insert(_full_collection, _mongo_document.obj());

	return _document["id"]->str();
}

int zapata::redis::Collection::update(std::string _collection, zapata::JSONPtr _pattern, zapata::JSONPtr _document) {	
	assertz(_document->ok() && _document->type() == zapata::JSObject, "'_document' must be of type JSObject", 412, 0);
	assertz(_pattern->ok() && _pattern->type() == zapata::JSObject, "'_pattern' must be of type JSObject", 412, 0);

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__options["mongo"]["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::redis::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);

	_document = zapata::make_ptr(JSON("$set" << _document));
	mongo::BSONObjBuilder _mongo_document;
	zapata::redis::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, _filter, _mongo_document.obj(), false, true);

	return _size;
}

int zapata::redis::Collection::unset(std::string _collection, zapata::JSONPtr _pattern, zapata::JSONPtr _document) {
	assertz(_document->ok() && _document->type() == zapata::JSObject, "'_document' must be of type JSObject", 412, 0);
	assertz(_pattern->ok() && _pattern->type() == zapata::JSObject, "'_pattern' must be of type JSObject", 412, 0);

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__options["mongo"]["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::redis::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);

	_document = zapata::make_ptr(JSON("$unset" << _document));
	mongo::BSONObjBuilder _mongo_document;
	zapata::redis::tomongo(_document, _mongo_document);
	this->__conn->update(_full_collection, _filter, _mongo_document.obj(), false, true);

	return _size;
}

int zapata::redis::Collection::remove(std::string _collection, zapata::JSONPtr _pattern) {
	assertz(_pattern->ok() && _pattern->type() == zapata::JSObject, "'_pattern' must be of type JSObject", 412, 0);

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__options["mongo"]["db"]);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::redis::get_query(_pattern, _query_b, _order_b, _page_size, _page_start_index);
	mongo::Query _filter(_query_b.done());

	unsigned long _size = this->__conn->count(_full_collection, _filter.obj, (int) mongo::QueryOption_SlaveOk);
	this->__conn->remove(_full_collection, _filter);

	return _size;
}

zapata::JSONPtr zapata::redis::Collection::query(std::string _collection, zapata::JSONPtr _pattern) {
	zapata::JSONArr _return;

	std::string _full_collection(_collection);
	_full_collection.insert(0, ".");
	_full_collection.insert(0, (string) this->__options["mongo"]["db"]);

	assertz(_full_collection.length() != 0, "'_full_collection' parameter must not be empty", 0, 0);
	assertz(_regexp.length() != 0, "'_regexp' parameter must not be empty", 0, 0);
	string _command("HSCAN %s %i MATCH %s");
	int _cursor = 0;
	redisReply* _reply = nullptr;
	muzzley::JSONObj _return;
	size_t _size = 0;

	do {
		bool _success = true;
		short _retry = 0;
		do {
			pthread_mutex_lock(this->__mtx);
			redisAppendCommand(this->__redis, _command.data(), _full_collection.data(), _cursor, _regexp.data());
			_success = (redisGetReply(this->__redis, (void**) & _reply) == REDIS_OK) && (_reply != nullptr);
			pthread_mutex_unlock(this->__mtx);
			if (!_success) {
				mzlog("disconnected from Redis server, going to reconnect...", muzzley::warning);
				this->reconnect();
			}
			_retry++;
		}
		while(!_success && _retry != 10);
		if(!_success) {
			mzlog(string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), muzzley::emergency);
			exit(-10);
		}
		
		int _type = _reply->type;
		switch(_type) {
			case REDIS_REPLY_STATUS:
			case REDIS_REPLY_ERROR: {
				_cursor = 0;
				std::string _status_text(_reply->str, _reply->len);
				freeReplyObject(_reply);
				assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, string("Redis server replied with an error/status: ") + _status_text, 0, 0); 
				break;
			}
			case REDIS_REPLY_INTEGER: {
				_cursor = 0;
				freeReplyObject(_reply);
				assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, string("Redis server replied something else: REDIS_REPLY_INTEGER"), 0, 0); 			
				break;
			}
			case REDIS_REPLY_STRING: {
				_cursor = 0;
				std::string _string_text(_reply->str, _reply->len);
				freeReplyObject(_reply);
				assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, string("Redis server replied something else: REDIS_REPLY_STRING > ") + _string_text, 0, 0); 
				break;
			}
			case REDIS_REPLY_NIL: {
				_cursor = 0;
				freeReplyObject(_reply);
				break;
			}
			case REDIS_REPLY_ARRAY: {
				_cursor = std::stoi(string(_reply->element[0]->str, _reply->element[0]->len));
				for (size_t _i = 0; _i < _reply->element[1]->elements; _i += 2) {
					std::string _full_collection(_reply->element[1]->element[_i]->str, _reply->element[1]->element[_i]->len);
					std::string _json(_reply->element[1]->element[_i + 1]->str, _reply->element[1]->element[_i + 1]->len);
					_return << _full_collection << muzzley::fromstr(_json);
				}
				freeReplyObject(_reply);
				break;
			}
			default : {
				_cursor = 0;
				freeReplyObject(_reply);
				mzlog("\nnone of the above", muzzley::notice);
				break;
			}
		}
	}
	while (_cursor != 0);

	if (_return->size() == 0) {
		return zapata::undefined;
	}
	return zapata::make_ptr(JSON(
		"size" << _size <<
		"elements" << _return
	));
}
