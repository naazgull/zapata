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

zpt::redis::ClientPtr::ClientPtr(zpt::JSONPtr _options, std::string _conf_path) : std::shared_ptr<zpt::redis::Client>(new zpt::redis::Client(_options, _conf_path)) {
}

zpt::redis::ClientPtr::~ClientPtr() {
}

zpt::redis::Client::Client(zpt::JSONPtr _options, std::string _conf_path) : __options( _options), __redis_conf(_options->getPath(_conf_path)), __conf_path(_conf_path) {
	std::string _bind((std::string) this->__redis_conf["bind"]);
	std::string _address(_bind.substr(0, _bind.find(":")));
	uint _port = std::stoi(_bind.substr(_bind.find(":") + 1));
	this->connect(_address, _port);
}

zpt::redis::Client::~Client() {
	redisFree(this->__conn);
	pthread_mutexattr_destroy(this->__attr);
	pthread_mutex_destroy(this->__mtx);
	delete this->__mtx;
	delete this->__attr;
}

zpt::JSONPtr zpt::redis::Client::options() {
	return this->__options;
}

std::string zpt::redis::Client::name() {
	return std::string("redis://") + ((std::string) this->__redis_conf["bind"]) + std::string("/") + ((std::string) this->__redis_conf["db"]);
}

void zpt::redis::Client::connect(std::string _host, uint _port) {
	zlog(std::string("connecting to ") + _host + std::string(":") + std::to_string(_port), zpt::debug);
	this->__mtx = new pthread_mutex_t();
	this->__attr = new pthread_mutexattr_t();
	pthread_mutexattr_init(this->__attr);
	pthread_mutex_init(this->__mtx, this->__attr);

	this->__host.assign(_host.data());
	this->__port = _port;
	bool _success = true;
	short _retry = 0;
	do {
		_success = ((this->__conn = redisConnect(this->__host.data(), this->__port)) != nullptr);
		_retry++;
		if (!_success) {
			sleep(1);		
		}
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(std::string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zpt::emergency);
		exit(-10);
	}
};

void zpt::redis::Client::reconnect() {
	redisFree(this->__conn);
	bool _success = true;
	short _retry = 0;
	do {
		sleep(1);
		_success = ((this->__conn = redisConnect(this->__host.data(), this->__port)) != nullptr);
		_retry++;
	}
	while(!_success && _retry != 10);
}

std::string zpt::redis::Client::insert(std::string _collection, std::string _id_prefix, zpt::JSONPtr _document) {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (std::string) this->__redis_conf["db"]);

	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	_document << "id" << _uuid.string();
	_document << "_id" << (_id_prefix + (_id_prefix.back() != '/' ? string("/") : string("")) + _document["id"]->str());
	_document << "href" << _document["_id"];

	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _document["_id"]->str().data(), ((std::string) _document).data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(std::string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zpt::emergency);
		exit(-10);
	}
	freeReplyObject(_reply);	

	return _document["id"]->str();
}

int zpt::redis::Client::save(std::string _collection, std::string _url, zpt::JSONPtr _document) {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (std::string) this->__redis_conf["db"]);

	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _url.data(), ((std::string) _document).data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(std::string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zpt::emergency);
		exit(-10);
	}
	freeReplyObject(_reply);	

	return 1;
}

int zpt::redis::Client::set(std::string _collection, std::string _url, zpt::JSONPtr _document) {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (std::string) this->__redis_conf["db"]);

	zpt::JSONPtr _record = this->get(_collection, _url);
	zpt::JSONPtr _new_record = _record + _document;

	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _url.data(), ((std::string) _new_record).data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(std::string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zpt::emergency);
		exit(-10);
	}
	freeReplyObject(_reply);	

	return 1;
}

int zpt::redis::Client::unset(std::string _collection, std::string _url, zpt::JSONPtr _document) {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (std::string) this->__redis_conf["db"]);

	zpt::JSONPtr _record = this->get(_collection, _url);
	zpt::JSONPtr _new_record = _record->clone();
	for (auto _attribute : _document->obj()) {
		_new_record >> _attribute.first;
	}

	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _url.data(), ((std::string) _new_record).data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(std::string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zpt::emergency);
		exit(-10);
	}
	freeReplyObject(_reply);

	return 1;
}

int zpt::redis::Client::remove(std::string _collection, std::string _url) {	
	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (std::string) this->__redis_conf["db"]);

	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, "HDEL %s %s", _key.data(), _url.data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(std::string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zpt::emergency);
		exit(-10);
	}
	freeReplyObject(_reply);

	return 1;
}

zpt::JSONPtr zpt::redis::Client::get(std::string _collection, std::string _url) {	
	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
	assertz(_url.length() != 0, "'_url' parameter must not be empty", 0, 0);

	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (std::string) this->__redis_conf["db"]);

	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, "HGET %s %s", _key.data(), _url.data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK) && (_reply != nullptr);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			if (_reply != nullptr) {
				freeReplyObject(_reply);
			}
			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
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
				zpt::JSONPtr _return = (zpt::JSONPtr) zpt::json(_data);
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
	return zpt::undefined;
}

zpt::JSONPtr zpt::redis::Client::query(std::string _collection, std::string _regexp) {
	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);
	assertz(_regexp.length() != 0, "'_regexp' parameter must not be empty", 0, 0);

	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (std::string) this->__redis_conf["db"]);

	int _cursor = 0;
	redisReply* _reply = nullptr;
	zpt::JSONArr _return;

	do {
		bool _success = true;
		short _retry = 0;
		do {
			pthread_mutex_lock(this->__mtx);
			redisAppendCommand(this->__conn, "HSCAN %s %i MATCH %s", _key.data(), _cursor, _regexp.data());
			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK) && (_reply != nullptr);
			pthread_mutex_unlock(this->__mtx);
			if (!_success) {
				if (_reply != nullptr) {
					freeReplyObject(_reply);
				}
				zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
				this->reconnect();
			}
			_retry++;
		}
		while(!_success && _retry != 10);
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
					_return << (zpt::JSONPtr) zpt::json(_json);
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

	return JPTR(
		"size" << _return->size() <<
		"elements" << _return
	);
}

zpt::JSONPtr zpt::redis::Client::all(std::string _collection) {
	assertz(_collection.length() != 0, "'_collection' parameter must not be empty", 0, 0);

	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (std::string) this->__redis_conf["db"]);

	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, "HGETALL %s", _key.data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK) && (_reply != nullptr);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			if (_reply != nullptr) {
				freeReplyObject(_reply);
			}
			zlog("disconnected from Redis server, going to reconnect...", zpt::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
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
				_return << (zpt::JSONPtr) zpt::json(_json);
			}
			freeReplyObject(_reply);
			return JPTR(
				"size" << _return->size() <<
				"elements" << _return
			);
		}
		default : {
			zlog("\nnone of the above", zpt::notice);
			break;
		}
	}

	freeReplyObject(_reply);
	return zpt::undefined;
}
