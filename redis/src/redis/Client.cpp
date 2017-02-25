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

#include <zapata/redis/Client.h>

zpt::redis::ClientPtr::ClientPtr(zpt::redis::Client * _target) : std::shared_ptr<zpt::redis::Client>(_target) {
}

zpt::redis::ClientPtr::ClientPtr(zpt::json _options, std::string _conf_path) : std::shared_ptr<zpt::redis::Client>(new zpt::redis::Client(_options, _conf_path)) {
}

zpt::redis::ClientPtr::~ClientPtr() {
}

zpt::redis::Client::Client(zpt::json _options, std::string _conf_path) : __options( _options), __conn(nullptr) {
	try {
		std::string _bind((std::string) _options->getPath(_conf_path)["bind"]);
		std::string _address(_bind.substr(0, _bind.find(":")));
		uint _port = std::stoi(_bind.substr(_bind.find(":") + 1));
		this->connection(_options->getPath(_conf_path) + zpt::json({ "host", _address, "port", _port }));
	}
	catch(std::exception& _e) {
		assertz(false, std::string("could not connect to Redis server: ") + _e.what(), 500, 0);
	}
}

zpt::redis::Client::~Client() {
	if (this->__conn != nullptr) {
		redisFree(this->__conn);
	}
}

auto zpt::redis::Client::name() -> std::string {
	return std::string("redis://") + ((std::string) this->connection()["bind"]) + std::string("/") + ((std::string) this->connection()["db"]);
}

auto zpt::redis::Client::options() -> zpt::json{
	return this->__options;
}

auto zpt::redis::Client::events(zpt::ev::emitter _emitter) -> void {
	this->__events = _emitter;
}

auto zpt::redis::Client::events() -> zpt::ev::emitter {
	return this->__events;
}

auto zpt::redis::Client::mutations(zpt::mutation::emitter _emitter) -> void {
}

auto zpt::redis::Client::mutations() -> zpt::mutation::emitter {
	return this->__events->mutations();
}

auto zpt::redis::Client::connect() -> void {
	this->__host.assign(std::string(this->connection()["host"]).data());
	this->__port = int(this->connection()["port"]);
	assertz((this->__conn = redisConnect(this->__host.data(), this->__port)) != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0);
	zpt::Connector::connect();
};

auto zpt::redis::Client::reconnect() -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0);
	redisFree(this->__conn);
	assertz((this->__conn = redisConnect(this->__host.data(), this->__port)) != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0);
	zpt::Connector::reconnect();
}

auto zpt::redis::Client::insert(std::string _collection, std::string _href_prefix, zpt::json _document, zpt::json _opts) -> std::string {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _key(_collection);
	_key.insert(0, "/");
	_key.insert(0, (std::string) this->connection()["db"]);

	if (!_document["id"]->ok()) {
		_document << "id" << zpt::generate::r_uuid();
	}
	if (!_document["href"]->ok() && _href_prefix.length() != 0) {
		_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
	}

	redisReply* _reply = nullptr;
	bool _success = true;
	// do {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _document["href"]->str().data(), ((std::string) _document).data());
			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK); }
	// 	if (!_success) {
	// 		zlog("disconnected from Redis server, going to reconnect...", zpt::critical);
	// 		this->reconnect();
	// 	}
	// }
	// while(!_success);
	freeReplyObject(_reply);

	if (!bool(_opts["mutated-event"])) zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
	return _document["id"]->str();
}

auto zpt::redis::Client::save(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _key(_collection);
	_key.insert(0, "/");
	_key.insert(0, (std::string) this->connection()["db"]);

	redisReply* _reply = nullptr;
	bool _success = true;
	// do {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _href.data(), ((std::string) _document).data());
			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK); }
	// 	if (!_success) {
	// 		zlog("disconnected from Redis server, going to reconnect...", zpt::critical);
	// 		this->reconnect();
	// 	}
	// }
	// while(!_success);
	freeReplyObject(_reply);

	if (!bool(_opts["mutated-event"])) zpt::Connector::save(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::redis::Client::set(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
 	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

 	zpt::json _record = this->get(_collection, _href);
 	zpt::json _new_record = _record + _document;

 	redisReply* _reply = nullptr;
 	bool _success = true;
  	// do {
 		{ std::lock_guard< std::mutex > _lock(this->__mtx);
 			redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _href.data(), ((std::string) _new_record).data());
 			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK); }
 	// 	if (!_success) {
 	// 		zlog("disconnected from Redis server, going to reconnect...", zpt::critical);
 	// 		this->reconnect();
 	// 	}
  	// }
 	// while(!_success);
 	freeReplyObject(_reply);	

	if (!bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _href, _document, _opts);
 	return 1;
}

auto zpt::redis::Client::set(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

	zpt::json _selected = this->query(_collection, zpt::redis::to_regex(_pattern), _opts);
	for (auto _record : _selected["elements"]->arr()) {
		zpt::json _new_record = _record + _document;

		redisReply* _reply = nullptr;
		bool _success = true;
		// do {
			{ std::lock_guard< std::mutex > _lock(this->__mtx);
				redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _record["href"]->str().data(), ((std::string) _new_record).data());
				_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK); }
		// 	if (!_success) {
		// 		zlog("disconnected from Redis server, going to reconnect...", zpt::critical);
		// 		this->reconnect();
		// 	}
		// }
		// while(!_success);
		freeReplyObject(_reply);
	}
	
	if (!bool(_opts["mutated-event"]) && int(_selected["size"]) != 0) zpt::Connector::set(_collection, _pattern, _document, _opts);
	return int(_selected["size"]);
}

auto zpt::redis::Client::unset(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
 	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

 	zpt::json _record = this->get(_collection, _href);
 	zpt::json _new_record = _record->clone();
 	for (auto _attribute : _document->obj()) {
 		_new_record >> _attribute.first;
 	}

 	redisReply* _reply = nullptr;
 	bool _success = true;
  	// do {
 		{ std::lock_guard< std::mutex > _lock(this->__mtx);
 			redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _href.data(), ((std::string) _new_record).data());
 			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK); }
 	// 	if (!_success) {
 	// 		zlog("disconnected from Redis server, going to reconnect...", zpt::critical);
 	// 		this->reconnect();
 	// 	}
  	// }
 	// while(!_success);
 	freeReplyObject(_reply);

	if (!bool(_opts["mutated-event"])) zpt::Connector::unset(_collection, _href, _document, _opts);
 	return 1;
}

auto zpt::redis::Client::unset(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

	zpt::json _selected = this->query(_collection, zpt::redis::to_regex(_pattern), _opts);
	for (auto _record : _selected["elements"]->arr()) {
		zpt::json _new_record = _record->clone();
		for (auto _attribute : _document->obj()) {
			_new_record >> _attribute.first;
		}

		redisReply* _reply = nullptr;
		bool _success = true;
		// do {
			{ std::lock_guard< std::mutex > _lock(this->__mtx);
				redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _record["href"]->str().data(), ((std::string) _new_record).data());
				_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK); }
		// 	if (!_success) {
		// 		zlog("disconnected from Redis server, going to reconnect...", zpt::critical);
		// 		this->reconnect();
		// 	}
		// }
		// while(!_success);
		freeReplyObject(_reply);
	}
	
	if (!bool(_opts["mutated-event"]) && int(_selected["size"]) != 0) zpt::Connector::unset(_collection, _pattern, _document, _opts);
	return int(_selected["size"]);
}

auto zpt::redis::Client::remove(std::string _collection, std::string _href, zpt::json _opts) -> int {	
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

 	redisReply* _reply = nullptr;
 	bool _success = true;
  	// do {
 		{ std::lock_guard< std::mutex > _lock(this->__mtx);
 			redisAppendCommand(this->__conn, "HDEL %s %s", _key.data(), _href.data());
 			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK); }
 	// 	if (!_success) {
 	// 		zlog("disconnected from Redis server, going to reconnect...", zpt::critical);
 	// 		this->reconnect();
 	// 	}
  	// }
 	// while(!_success);
 	freeReplyObject(_reply);

	if (!bool(_opts["mutated-event"])) zpt::Connector::remove(_collection, _href, _opts);
 	return 1;
}

auto zpt::redis::Client::remove(std::string _collection, zpt::json _pattern, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);
 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

	zpt::json _selected = this->query(_collection, zpt::redis::to_regex(_pattern), _opts);
	for (auto _record : _selected["elements"]->arr()) {
		redisReply* _reply = nullptr;
		bool _success = true;
		// do {
			{ std::lock_guard< std::mutex > _lock(this->__mtx);
				redisAppendCommand(this->__conn, "HDEL %s %s", _key.data(), _record["href"]->str().data());
				_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK); }
		// 	if (!_success) {
		// 		zlog("disconnected from Redis server, going to reconnect...", zpt::critical);
		// 		this->reconnect();
		// 	}
		// }
		// while(!_success);
		freeReplyObject(_reply);
		if (!bool(_opts["mutated-event"])) zpt::Connector::remove(_collection, _record["href"]->str(), _opts);
	}
	
	return int(_selected["size"]);
}

auto zpt::redis::Client::get(std::string _collection, std::string _href, zpt::json _opts) -> zpt::json {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
 	assertz(_href.length() != 0, "'href' parameter must not be empty", 0, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

 	redisReply* _reply = nullptr;
 	bool _success = true;
  	// do {
 		{ std::lock_guard< std::mutex > _lock(this->__mtx);
 			redisAppendCommand(this->__conn, "HGET %s %s", _key.data(), _href.data());
 			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK) && (_reply != nullptr);	}
 	// 	if (!_success) {
 	// 		if (_reply != nullptr) {
 	// 			freeReplyObject(_reply);
 	// 		}
 	// 		zlog("disconnected from Redis server, going to reconnect...", zpt::critical);
 	// 		this->reconnect();
 	// 	}
  	// }
 	// while(!_success);
 	if(!_success) {
 		zlog(std::string("couldn't connect to ") + this->__host + std::string(":") + std::to_string(this->__port) + std::string(", after several attempts.\nEXITING since can't vouche for internal state."), zpt::emergency);
 		exit(-10);
 	}
	
 	int _type = _reply->type;
 	switch(_type) {
 		case REDIS_REPLY_STATUS:
 		case REDIS_REPLY_ERROR: {
 			std::string _status_text(_reply->str, _reply->len);
 			freeReplyObject(_reply);
 			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_STRING, std::string("Redis server replied with an error/status: ") + _status_text, 0, 0); 
 		}
 		case REDIS_REPLY_INTEGER: {
 			freeReplyObject(_reply);
 			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_STRING, std::string("Redis server replied something else: REDIS_REPLY_INTEGER"), 0, 0); 			
 		}
 		case REDIS_REPLY_ARRAY: {
 			freeReplyObject(_reply);
 			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_STRING, std::string("Redis server replied something else: REDIS_REPLY_ARRAY"), 0, 0); 
 		}
 		case REDIS_REPLY_NIL: {
 			freeReplyObject(_reply);
 			return zpt::undefined;
 		}
 		case REDIS_REPLY_STRING: {
 			std::string _data(_reply->str, _reply->len);
 			try {
 				zpt::json _return = (zpt::json) zpt::json(_data);
 				freeReplyObject(_reply);
 				return _return;
 			}
 			catch(zpt::SyntaxErrorException& _e) {
 				freeReplyObject(_reply);
 				assertz(_reply != nullptr, std::string("something went wrong with the Redis server, got non JSON value:\n") + _data, 0, 0);
 			}
 		}
 		default : {
 			zlog("\nnone of the above", zpt::notice);
 			break;
 		}
 	}
 	freeReplyObject(_reply);
	
	if (!bool(_opts["mutated-event"])) zpt::Connector::get(_collection, _href, _opts);
 	return zpt::undefined;
}

auto zpt::redis::Client::query(std::string _collection, std::string _regexp, zpt::json _opts) -> zpt::json {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
 	assertz(_regexp.length() != 0, "'_regexp' parameter must not be empty", 0, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->connection()["db"]);

 	int _cursor = 0;
 	redisReply* _reply = nullptr;
 	zpt::JSONArr _return;

 	do {
 		bool _success = true;
		// do {
 			{ std::lock_guard< std::mutex > _lock(this->__mtx);
 				redisAppendCommand(this->__conn, "HSCAN %s %i MATCH %s", _key.data(), _cursor, _regexp.data());
 				_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK) && (_reply != nullptr); }
 		// 	if (!_success) {
 		// 		if (_reply != nullptr) {
 		// 			freeReplyObject(_reply);
 		// 		}
 		// 		zlog("disconnected from Redis server, going to reconnect...", zpt::critical);
 		// 		this->reconnect();
 		// 	}
		// }
 		// while(!_success);
		
 		int _type = _reply->type;
 		switch(_type) {
 			case REDIS_REPLY_STATUS:
 			case REDIS_REPLY_ERROR: {
 				_cursor = 0;
 				std::string _status_text(_reply->str, _reply->len);
 				freeReplyObject(_reply);
 				assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, std::string("Redis server replied with an error/status: ") + _status_text, 0, 0); 
 				break;
 			}
 			case REDIS_REPLY_INTEGER: {
 				_cursor = 0;
 				freeReplyObject(_reply);
 				assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, std::string("Redis server replied something else: REDIS_REPLY_INTEGER"), 0, 0); 			
 				break;
 			}
 			case REDIS_REPLY_STRING: {
 				_cursor = 0;
 				std::string _string_text(_reply->str, _reply->len);
 				freeReplyObject(_reply);
 				assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, std::string("Redis server replied something else: REDIS_REPLY_STRING > ") + _string_text, 0, 0); 
 				break;
 			}
 			case REDIS_REPLY_NIL: {
 				_cursor = 0;
 				freeReplyObject(_reply);
				break;
			}
			case REDIS_REPLY_ARRAY: {
				_cursor = std::stoi(std::string(_reply->element[0]->str, _reply->element[0]->len));
				for (size_t _i = 0; _i < _reply->element[1]->elements; _i += 2) {
					std::string _json(_reply->element[1]->element[_i + 1]->str, _reply->element[1]->element[_i + 1]->len);
					_return << (zpt::json) zpt::json(_json);
				}
				freeReplyObject(_reply);
				break;
			}
			default : {
				_cursor = 0;
				freeReplyObject(_reply);
				zlog("\nnone of the above", zpt::notice);
				break;
			}
		}
	}
	while (_cursor != 0);

	if (!bool(_opts["mutated-event"])) zpt::Connector::query(_collection, _regexp, _opts);
	return {
		"size", _return->size(),
		"elements", _return
	};
}

auto zpt::redis::Client::query(std::string _collection, zpt::json _regexp, zpt::json _opts) -> zpt::json {
 	assertz(_regexp->ok() && _regexp->type() == zpt::JSObject, "'_regexp' must be of type JSObject", 412, 0);
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
	return this->query(_collection, zpt::redis::to_regex(_regexp), _opts);
}

auto zpt::redis::Client::all(std::string _collection, zpt::json _opts) -> zpt::json {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn != nullptr, std::string("connection to Redis at ") + this->name() + std::string(" has not been established."), 500, 0); }
	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);

	std::string _key(_collection);
	_key.insert(0, "/");
	_key.insert(0, (std::string) this->connection()["db"]);

	redisReply* _reply = nullptr;
	bool _success = true;
	// do {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			redisAppendCommand(this->__conn, "HGETALL %s", _key.data());
			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK) && (_reply != nullptr); }
	// 	if (!_success) {
	// 		if (_reply != nullptr) {
	// 			freeReplyObject(_reply);
	// 		}
	// 		zlog("disconnected from Redis server, going to reconnect...", zpt::critical);
	// 		this->reconnect();
	// 	}
	// }
	// while(!_success);
	
	int _type = _reply->type;
	switch(_type) {
		case REDIS_REPLY_STATUS:
		case REDIS_REPLY_ERROR: {
			std::string _status_text(_reply->str, _reply->len);
			freeReplyObject(_reply);
			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, std::string("Redis server replied with an error/status: ") + _status_text, 0, 0); 
		}
		case REDIS_REPLY_INTEGER: {
			freeReplyObject(_reply);
			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, std::string("Redis server replied something else: REDIS_REPLY_INTEGER"), 0, 0); 			
		}
		case REDIS_REPLY_STRING: {
			std::string _string_text(_reply->str, _reply->len);
			freeReplyObject(_reply);
			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, std::string("Redis server replied something else: REDIS_REPLY_STRING > ") + _string_text, 0, 0); 
		}
		case REDIS_REPLY_NIL: {
			freeReplyObject(_reply);
			return zpt::undefined;
		}
		case REDIS_REPLY_ARRAY: {
			zpt::JSONArr _return;
			for (size_t _i = 0; _i < _reply->elements; _i += 2) {
				std::string _json(_reply->element[_i + 1]->str, _reply->element[_i + 1]->len);
				_return << (zpt::json) zpt::json(_json);
			}
			freeReplyObject(_reply);
			return {
				"size", _return->size(),
				"elements", _return
			};
		}
		default : {
			zlog("\nnone of the above", zpt::notice);
			break;
		}
	}
	freeReplyObject(_reply);

	if (!bool(_opts["mutated-event"])) zpt::Connector::all(_collection, _opts);	
	return zpt::undefined;
}

extern "C" auto zpt_redis() -> int {
	return 1;
}
