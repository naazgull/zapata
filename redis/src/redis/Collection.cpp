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

zapata::redis::CollectionPtr::CollectionPtr(zapata::redis::Collection * _target) : std::shared_ptr<zapata::redis::Collection>(_target) {
}

zapata::redis::CollectionPtr::CollectionPtr(zapata::JSONPtr _options) : std::shared_ptr<zapata::redis::Collection>(new zapata::redis::Collection(_options)) {
}

zapata::redis::CollectionPtr::~CollectionPtr() {
}

zapata::redis::Collection::Collection(zapata::JSONPtr _options) : __options( _options) {
	this->connect((string) _options["redis"]["bind"], (uint) _options["redis"]["port"]);
}

zapata::redis::Collection::~Collection() {
	redisFree(this->__conn);
	pthread_mutexattr_destroy(this->__attr);
	pthread_mutex_destroy(this->__mtx);
	delete this->__mtx;
	delete this->__attr;
}

zapata::JSONPtr zapata::redis::Collection::options() {
	return this->__options;
}

std::string zapata::redis::Collection::name() {
	return string("redis://") + ((string) this->__options["redis"]["bind"]) + string(":") + ((string) this->__options["redis"]["port"]) + string("/") + ((string) this->__options["redis"]["db"]);
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
		_success = ((this->__conn = redisConnect(this->__host.data(), this->__port)) != nullptr);
		_retry++;
		if (!_success) {
			sleep(1);		
		}
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zapata::emergency);
		exit(-10);
	}
};

void zapata::redis::Collection::reconnect() {
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

zapata::JSONPtr zapata::redis::Collection::insert(std::string _collection, std::string _id_prefix, zapata::JSONPtr _document) {	
	assertz(_document->ok() && _document->type() == zapata::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (string) this->__options["redis"]["db"]);

	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	_document << "id" << _uuid.string();
	_document << "_id" << (_id_prefix + (_id_prefix.back() != '/' ? string("/") : string("")) + _document["id"]->str());
	_document << "href" << _document["id"];

	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _document["_id"]->str().data(), ((string) _document).data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zapata::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zapata::emergency);
		exit(-10);
	}
	freeReplyObject(_reply);	

	return _document;
}

int zapata::redis::Collection::update(std::string _collection, std::string _url, zapata::JSONPtr _document) {	
	assertz(_document->ok() && _document->type() == zapata::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (string) this->__options["redis"]["db"]);

	zapata::JSONPtr _record = this->get(_collection, _url);
	zapata::JSONPtr _new_record = _record + _document;

	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _url.data(), ((string) _new_record).data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zapata::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zapata::emergency);
		exit(-10);
	}
	freeReplyObject(_reply);	

	return 1;
}

int zapata::redis::Collection::unset(std::string _collection, std::string _url, zapata::JSONPtr _document) {
	assertz(_document->ok() && _document->type() == zapata::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (string) this->__options["redis"]["db"]);

	zapata::JSONPtr _record = this->get(_collection, _url);
	zapata::JSONPtr _new_record = _record->clone();
	for (auto _attribute : _document->obj()) {
		_new_record >> _attribute.first;
	}

	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, "HSET %s %s %s", _key.data(), _url.data(), ((string) _new_record).data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zapata::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zapata::emergency);
		exit(-10);
	}
	freeReplyObject(_reply);

	return 1;
}

int zapata::redis::Collection::remove(std::string _collection, std::string _url) {	
	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (string) this->__options["redis"]["db"]);

	string _command("HDEL %s %s");
	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, _command.data(), _key.data(), _url.data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zapata::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zapata::emergency);
		exit(-10);
	}
	freeReplyObject(_reply);

	return 1;
}

zapata::JSONPtr zapata::redis::Collection::get(std::string _collection, std::string _url) {	
	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (string) this->__options["redis"]["db"]);

	string _command("HGET %s %s");
	redisReply* _reply = nullptr;
	bool _success = true;
	short _retry = 0;
	do {
		pthread_mutex_lock(this->__mtx);
		redisAppendCommand(this->__conn, _command.data(), _key.data(), _url.data());
		_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK) && (_reply != nullptr);
		pthread_mutex_unlock(this->__mtx);
		if (!_success) {
			zlog("disconnected from Redis server, going to reconnect...", zapata::warning);
			this->reconnect();
		}
		_retry++;
	}
	while(!_success && _retry != 10);
	if(!_success) {
		zlog(string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zapata::emergency);
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
			return zapata::undefined;
		}
		case REDIS_REPLY_STRING: {
			string _data(_reply->str, _reply->len);
			try {
				zapata::JSONPtr _return = zapata::fromstr(_data);
				freeReplyObject(_reply);
				return _return;
			}
			catch(zapata::SyntaxErrorException& _e) {
				assertz(_reply != nullptr, string("something went wrong with the Redis server, got non JSON value:\n") + _data, 0, 0);
			}
		}
		default : {
			zlog("\nnone of the above", zapata::notice);
			break;
		}
	}

	freeReplyObject(_reply);
	return zapata::undefined;
}

zapata::JSONPtr zapata::redis::Collection::query(std::string _collection, zapata::JSONPtr _pattern) {
	std::string _key(_collection);
	_key.insert(0, "|");
	_key.insert(0, (string) this->__options["redis"]["db"]);

	string _command("HSCAN %s %i");
	int _cursor = 0;
	redisReply* _reply = nullptr;
	zapata::JSONPtr _return = zapata::mkarr();

	do {
		bool _success = true;
		short _retry = 0;
		do {
			pthread_mutex_lock(this->__mtx);
			redisAppendCommand(this->__conn, _command.data(), _key.data(), _cursor);
			_success = (redisGetReply(this->__conn, (void**) & _reply) == REDIS_OK) && (_reply != nullptr);
			pthread_mutex_unlock(this->__mtx);
			if (!_success) {
				zlog("disconnected from Redis server, going to reconnect...", zapata::warning);
				this->reconnect();
			}
			_retry++;
		}
		while(!_success && _retry != 10);
		if(!_success) {
			zlog(string("couldn't connect to ") + this->__host + string(":") + std::to_string(this->__port) + string(", after several attempts.\nEXITING since can't vouche for internal state."), zapata::emergency);
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
					std::string _json(_reply->element[1]->element[_i + 1]->str, _reply->element[1]->element[_i + 1]->len);
					zapata::JSONPtr _record = zapata::fromstr(_json);
					bool _match = true;
					for (auto _o : _pattern->obj()) {
						if (_record[_o.first] != _o.second) {
							_match = false;
							break;
						}
					}
					if (_match) {
						_return << _record;
					}
				}
				freeReplyObject(_reply);
				break;
			}
			default : {
				_cursor = 0;
				freeReplyObject(_reply);
				zlog("\nnone of the above", zapata::notice);
				break;
			}
		}
	}
	while (_cursor != 0);

	if (_return->arr()->size() == 0) {
		return zapata::undefined;
	}
	return zapata::mkptr(JSON(
		"size" << _return->arr()->size() <<
		"elements" << _return
	));
}
