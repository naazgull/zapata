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

zpt::redis::Client::Client(zpt::json _options, std::string _conf_path) : __options( _options), __redis_conf(_options->getPath(_conf_path)), __conf_path(_conf_path) {
	std::string _bind((std::string) this->__redis_conf["bind"]);
	std::string _address(_bind.substr(0, _bind.find(":")));
	uint _port = std::stoi(_bind.substr(_bind.find(":") + 1));
	this->connect({ "host", _address, "port", _port });
}

zpt::redis::Client::~Client() {
	redisFree(this->__conn);
}

auto zpt::redis::Client::name() -> std::string {
	return std::string("redis://") + ((std::string) this->__redis_conf["bind"]) + std::string("/") + ((std::string) this->__redis_conf["db"]);
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

auto zpt::redis::Client::connect(zpt::json _opts) -> void {
	this->__host.assign(std::string(_opts["host"]).data());
	this->__port = int(_opts["port"]);
	bool _success = true;
	do {
		_success = ((this->__conn = redisConnect(this->__host.data(), this->__port)) != nullptr);
		if (!_success) {
			sleep(1);		
		}
	}
	while(!_success);
	zpt::Connector::connect(_opts);
};

auto zpt::redis::Client::reconnect() -> void {
	redisFree(this->__conn);
	bool _success = true;
	do {
		sleep(1);
		_success = ((this->__conn = redisConnect(this->__host.data(), this->__port)) != nullptr);
	}
	while(!_success);
	zpt::Connector::reconnect();
}

auto zpt::redis::Client::insert(std::string _collection, std::string _href_prefix, zpt::json _document, zpt::json _opts) -> std::string {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _key(_collection);
	_key.insert(0, "/");
	_key.insert(0, (std::string) this->__redis_conf["db"]);

	if (!_document["id"]->ok()) {
		uuid _uuid;
		_uuid.make(UUID_MAKE_V1);
		_document << "id" << _uuid.string();
	}
	if (!_document["href"]->ok() && _href_prefix.length() != 0) {
		_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
	}

	redisReply* _reply = nullptr;
	bool _success = true;
	do {
		{
			std::lock_guard< std::mutex > _lock(this->__mtx);
			redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _document["href"]->str().data(), ((std::string) _document).data());
			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
		}
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
			this->reconnect();
		}
	}
	while(!_success);
	freeReplyObject(_reply);

	zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
	return _document["id"]->str();
}

auto zpt::redis::Client::save(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _key(_collection);
	_key.insert(0, "/");
	_key.insert(0, (std::string) this->__redis_conf["db"]);

	redisReply* _reply = nullptr;
	bool _success = true;
	do {
		{
			std::lock_guard< std::mutex > _lock(this->__mtx);
			redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _href.data(), ((std::string) _document).data());
			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
		}
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
			this->reconnect();
		}
	}
	while(!_success);
	freeReplyObject(_reply);

	zpt::Connector::save(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::redis::Client::set(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
 	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->__redis_conf["db"]);

 	zpt::json _record = this->get(_collection, _href);
 	zpt::json _new_record = _record + _document;

 	redisReply* _reply = nullptr;
 	bool _success = true;
  	do {
 		{
 			std::lock_guard< std::mutex > _lock(this->__mtx);
 			redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _href.data(), ((std::string) _new_record).data());
 			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
 		}
 		if (!_success) {
 			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
 			this->reconnect();
 		}
  	}
 	while(!_success);
 	freeReplyObject(_reply);	

	zpt::Connector::set(_collection, _href, _document, _opts);
 	return 1;
}

auto zpt::redis::Client::set(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);
 	assertz(_pattern["href"]->ok() && (_pattern->type() == zpt::JSString || _pattern->type() == zpt::JSArray), "'href' field must be of types string or array", 412, 0);
	if (_pattern["href"]->type() == zpt::JSString) {
		return this->set(_collection, _pattern["href"]->str(), _document, _opts);
	}
	else {
		int _changes = 0;
		for (auto _href : _pattern["href"]->arr()) {
			_changes += this->set(_collection, std::string(_href), _document, _opts);
		}
		return _changes;
	}
}

auto zpt::redis::Client::unset(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {
 	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->__redis_conf["db"]);

 	zpt::json _record = this->get(_collection, _href);
 	zpt::json _new_record = _record->clone();
 	for (auto _attribute : _document->obj()) {
 		_new_record >> _attribute.first;
 	}

 	redisReply* _reply = nullptr;
 	bool _success = true;
  	do {
 		{
 			std::lock_guard< std::mutex > _lock(this->__mtx);
 			redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _href.data(), ((std::string) _new_record).data());
 			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
 		}
 		if (!_success) {
 			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
 			this->reconnect();
 		}
  	}
 	while(!_success);
 	freeReplyObject(_reply);

	zpt::Connector::unset(_collection, _href, _document, _opts);
 	return 1;
}

auto zpt::redis::Client::unset(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);
 	assertz(_pattern["href"]->ok() && (_pattern->type() == zpt::JSString || _pattern->type() == zpt::JSArray), "'href' field must be of types string or array", 412, 0);
	if (_pattern["href"]->type() == zpt::JSString) {
		return this->unset(_collection, _pattern["href"]->str(), _document, _opts);
	}
	else {
		int _changes = 0;
		for (auto _href : _pattern["href"]->arr()) {
			_changes += this->unset(_collection, std::string(_href), _document, _opts);
		}
		return _changes;
	}
}

auto zpt::redis::Client::remove(std::string _collection, std::string _href, zpt::json _opts) -> int {	
 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->__redis_conf["db"]);

 	redisReply* _reply = nullptr;
 	bool _success = true;
  	do {
 		{
 			std::lock_guard< std::mutex > _lock(this->__mtx);
 			redisAppendCommand(this->__conn, "HDEL %s %s", _key.data(), _href.data());
 			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
 		}
 		if (!_success) {
 			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
 			this->reconnect();
 		}
  	}
 	while(!_success);
 	if(!_success) {
 		zlog(std::string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zpt::emergency);
 		exit(-10);
 	}
 	freeReplyObject(_reply);

	zpt::Connector::remove(_collection, _href, _opts);
 	return 1;
}

auto zpt::redis::Client::remove(std::string _collection, zpt::json _pattern, zpt::json _opts) -> int {
 	assertz(_pattern->ok() && _pattern->type() == zpt::JSObject, "'_pattern' must be of type JSObject", 412, 0);
 	assertz(_pattern["href"]->ok() && (_pattern->type() == zpt::JSString || _pattern->type() == zpt::JSArray), "'href' field must be of types string or array", 412, 0);
	if (_pattern["href"]->type() == zpt::JSString) {
		return this->remove(_collection, _pattern["href"]->str(), _opts);
	}
	else {
		int _changes = 0;
		for (auto _href : _pattern["href"]->arr()) {
			_changes += this->remove(_collection, std::string(_href), _opts);
		}
		return _changes;
	}
}

auto zpt::redis::Client::get(std::string _collection, std::string _href, zpt::json _opts) -> zpt::json {
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
 	assertz(_href.length() != 0, "'href' parameter must not be empty", 0, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->__redis_conf["db"]);

 	redisReply* _reply = nullptr;
 	bool _success = true;
  	do {
 		{
 			std::lock_guard< std::mutex > _lock(this->__mtx);
 			redisAppendCommand(this->__conn, "HGET %s %s", _key.data(), _href.data());
 			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK) && (_reply != nullptr);
 		}
 		if (!_success) {
 			if (_reply != nullptr) {
 				freeReplyObject(_reply);
 			}
 			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
 			this->reconnect();
 		}
  	}
 	while(!_success);
 	if(!_success) {
 		zlog(std::string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zpt::emergency);
 		exit(-10);
 	}
	
 	int _type = _reply->type;
 	switch(_type) {
 		case REDIS_REPLY_STATUS:
 		case REDIS_REPLY_ERROR: {
 			std::string _status_text(_reply->str, _reply->len);
 			freeReplyObject(_reply);
 			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_STRING, string("Redis server replied with an error/status: ") + _status_text, 0, 0); 
 		}
 		case REDIS_REPLY_INTEGER: {
 			freeReplyObject(_reply);
 			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_STRING, string("Redis server replied something else: REDIS_REPLY_INTEGER"), 0, 0); 			
 		}
 		case REDIS_REPLY_ARRAY: {
 			freeReplyObject(_reply);
 			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_STRING, string("Redis server replied something else: REDIS_REPLY_ARRAY"), 0, 0); 
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
 				assertz(_reply != nullptr, string("something went wrong with the Redis server, got non JSON value:\n") + _data, 0, 0);
 			}
 		}
 		default : {
 			zlog("\nnone of the above", zpt::notice);
 			break;
 		}
 	}
 	freeReplyObject(_reply);
	
	zpt::Connector::get(_collection, _href, _opts);
 	return zpt::undefined;
}

auto zpt::redis::Client::query(std::string _collection, std::string _regexp, zpt::json _opts) -> zpt::json {
 	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
 	assertz(_regexp.length() != 0, "'_regexp' parameter must not be empty", 0, 0);

 	std::string _key(_collection);
 	_key.insert(0, "/");
 	_key.insert(0, (std::string) this->__redis_conf["db"]);

 	int _cursor = 0;
 	redisReply* _reply = nullptr;
 	zpt::JSONArr _return;

 	do {
 		bool _success = true;
 	 		do {
 			{
 				std::lock_guard< std::mutex > _lock(this->__mtx);
 				redisAppendCommand(this->__conn, "HSCAN %s %i MATCH %s", _key.data(), _cursor, _regexp.data());
 				_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK) && (_reply != nullptr);
 			}
 			if (!_success) {
 				if (_reply != nullptr) {
 					freeReplyObject(_reply);
 				}
 				zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
 				this->reconnect();
 			}
 	 		}
 		while(!_success);
 		if(!_success) {
 			zlog(std::string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zpt::emergency);
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

	zpt::Connector::query(_collection, _regexp, _opts);
	return {
		"size", _return->size(),
		"elements", _return
	};
}

auto zpt::redis::Client::query(std::string _collection, zpt::json _regexp, zpt::json _opts) -> zpt::json {
	if (_regexp["query"]->ok()) {
		return this->query(_collection, _regexp["query"]->str(), _opts);
	}
	zpt::Connector::query(_collection, _regexp, _opts);	
	return zpt::undefined;
}

auto zpt::redis::Client::all(std::string _collection, zpt::json _opts) -> zpt::json {
	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);

	std::string _key(_collection);
	_key.insert(0, "/");
	_key.insert(0, (std::string) this->__redis_conf["db"]);

	redisReply* _reply = nullptr;
	bool _success = true;
	do {
		{
			std::lock_guard< std::mutex > _lock(this->__mtx);
			redisAppendCommand(this->__conn, "HGETALL %s", _key.data());
			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK) && (_reply != nullptr);
		}
		if (!_success) {
			if (_reply != nullptr) {
				freeReplyObject(_reply);
			}
			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
			this->reconnect();
		}
	}
	while(!_success);
	
	int _type = _reply->type;
	switch(_type) {
		case REDIS_REPLY_STATUS:
		case REDIS_REPLY_ERROR: {
			std::string _status_text(_reply->str, _reply->len);
			freeReplyObject(_reply);
			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, string("Redis server replied with an error/status: ") + _status_text, 0, 0); 
		}
		case REDIS_REPLY_INTEGER: {
			freeReplyObject(_reply);
			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, string("Redis server replied something else: REDIS_REPLY_INTEGER"), 0, 0); 			
		}
		case REDIS_REPLY_STRING: {
			std::string _string_text(_reply->str, _reply->len);
			freeReplyObject(_reply);
			assertz(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY, string("Redis server replied something else: REDIS_REPLY_STRING > ") + _string_text, 0, 0); 
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

	zpt::Connector::all(_collection, _opts);	
	return zpt::undefined;
}
