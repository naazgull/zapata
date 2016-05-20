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

#include <zapata/zmq/ZMQPolling.h>

zapata::ZMQ::ZMQ(std::string _connection, zapata::JSONPtr _options, zapata::EventEmitterPtr _emitter) : __options( _options ), __context(1), __connection(_connection.data()), __self(this), __emitter(_emitter) {
	this->__pt_mtx = new pthread_mutex_t();
	this->__pt_attr = new pthread_mutexattr_t();
	pthread_mutexattr_init(this->__pt_attr);
	pthread_mutex_init(this->__pt_mtx, this->__pt_attr);
}

zapata::ZMQ::~ZMQ() {
	pthread_mutexattr_destroy(this->__pt_attr);
	pthread_mutex_destroy(this->__pt_mtx);
	delete this->__pt_mtx;
	delete this->__pt_attr;
}

zapata::JSONPtr zapata::ZMQ::options() {
	return this->__options;
}

zmq::context_t& zapata::ZMQ::context() {
	return this->__context;
}

std::string& zapata::ZMQ::connection() {
	return this->__connection;
}

zapata::ZMQPtr zapata::ZMQ::self() {
	return this->__self;
}

zapata::EventEmitterPtr zapata::ZMQ::emitter() {
	return this->__emitter;
}

void zapata::ZMQ::lock() {
	pthread_mutex_lock(this->__pt_mtx);
}

void zapata::ZMQ::unlock() {
	pthread_mutex_unlock(this->__pt_mtx);	
}

zapata::JSONPtr zapata::ZMQ::recv() {
	std::vector<std::string> _parts;
	int _more = 0;
	size_t _more_size = sizeof(_more);

	this->lock();
	{
		do {
			zmq::message_t _request;
			try {
				this->in().recv(& _request);
			}
			catch(zmq::error_t& e) {
				throw;
			}
			_parts.push_back(string(static_cast<char*>(_request.data()), _request.size()));

			_more = 0;
			this->in().getsockopt(ZMQ_RCVMORE, & _more, & _more_size);
		}
		while(_more);
	}
	this->unlock();

	string _channel(_parts[0]);
	string _method(_parts[1]);
	string _resource(_parts[2]);
	string _body(_parts[_parts.size() - 1]);

	zapata::JSONPtr _headers = zapata::mkobj();
	if (_parts.size() > 4) {
		for (size_t _idx = 3; _idx != _parts.size() - 1; _idx++) {
			zapata::JSONPtr _pair = zapata::split(_parts[_idx], ":");
			std::string _key(_parts[_idx].substr(0, _parts[_idx].find(":")));
			std::string _value(_parts[_idx].substr(_parts[_idx].find(":") + 1));
			zapata::trim(_key);
			zapata::trim(_value);
			_headers << _key << _value;
		}
	}

	zapata::JSONPtr _payload;
	try {
		_payload = zapata::json(_body);
	}
	catch(zapata::SyntaxErrorException& _e) {
		_payload = zapata::undefined;
	}

	zapata::ev::Performative _performative = zapata::ev::from_str(_method);
	zlog(string("<- | \033[33;40m") + _method + string("\033[0m ") + (_performative == zapata::ev::Reply ? string("\033[") + (((int) _headers["X-Status"]) > 299 ? "31" : "32") + string(";40m") + ((string) _headers["X-Status"]) + string("\033[0m ") : "") + _resource, zapata::info);
	return zapata::mkptr(JSON(
		"channel" << _channel <<
		"performative" << _performative <<
		"resource" << _resource <<
		"headers" << _headers <<
		"payload" << _payload
	));
}

zapata::JSONPtr zapata::ZMQ::send(zapata::ev::Performative _performative, std::string _resource, zapata::JSONPtr _payload) {
	return this->send(zapata::mkptr(JSON(
		"channel" << _resource <<
		"performative" << _performative <<
		"resource" << _resource <<
		"payload" << _payload
	)));
}

zapata::JSONPtr zapata::ZMQ::send(zapata::JSONPtr _envelope) {
	assertz(_envelope["channel"]->ok(), "'channel' attribute is required", 412, 0);
	assertz(_envelope["performative"]->ok() && _envelope["resource"]->ok(), "'performative' and 'resource' attributes are required", 412, 0);
	assertz(!_envelope["headers"]->ok() || _envelope["headers"]->type() == zapata::JSObject, "'headers' must be of type JSON object", 412, 0);

	if ((zapata::ev::Performative) ((int) _envelope["performative"]) != zapata::ev::Reply) {
		_envelope << "headers" << (zapata::ev::init_request() + _envelope["headers"]);
	}
	else {
		assertz(_envelope["status"]->ok(), "'status' attribute is required", 412, 0);
		_envelope << "headers" << (zapata::ev::init_reply() + _envelope["headers"]);
		_envelope["headers"] << "X-Status" << _envelope["status"];
	}		
	if (!_envelope["payload"]->ok()) {
		_envelope << "payload" << zapata::mkobj();
	}

	zlog(string("-> | \033[33;40m") + zapata::ev::to_str((zapata::ev::Performative) ((int) _envelope["performative"])) + string("\033[0m ") + (((int) _envelope["performative"]) == zapata::ev::Reply ? string("\033[") + (((int) _envelope["headers"]["X-Status"]) > 299 ? "31" : "32") + string(";40m") + ((string) _envelope["headers"]["X-Status"]) + string("\033[0m ") : "") + ((string) _envelope["resource"]), zapata::info);
	std::vector<std::string> _parts;
	_parts.push_back((string) _envelope["channel"]);
	_parts.push_back(zapata::ev::to_str((zapata::ev::Performative) ((int) _envelope["performative"])));
	_parts.push_back((string) _envelope["resource"]);
	if (_envelope["headers"]->ok()) {
		for (auto _header : _envelope["headers"]->obj()) {
			_parts.push_back(_header.first + string(": ") + ((string) _header.second));
		}
	}

	this->lock(); 
	{
		for (auto _buffer : _parts) {
			zmq::message_t _request(_buffer.size());
			memcpy ((void *) _request.data(), _buffer.data(), _buffer.size());
			this->out().send(_request, ZMQ_SNDMORE);
		}

		std::string _payload = (string) _envelope["payload"];
		zmq::message_t _request(_payload.size());
		memcpy ((void *) _request.data(), _payload.data(), _payload.size());
		this->out().send(_request);
	} 
	this->unlock();

	return zapata::undefined;
}

zapata::ZMQReq::ZMQReq(std::string _connection, zapata::JSONPtr _options, zapata::EventEmitterPtr _emitter) : zapata::ZMQ( _connection, _options, _emitter ) {
	this->__socket = new zmq::socket_t(this->context(), ZMQ_REQ);
	this->__socket->connect(_connection.data());
	zlog(string("connecting REQ socket on ") + _connection, zapata::notice);
}

zapata::ZMQReq::~ZMQReq() {
	this->__socket->close();
	delete this->__socket;
}

zmq::socket_t& zapata::ZMQReq::socket() {
	return * this->__socket;
}

zmq::socket_t& zapata::ZMQReq::in() {
	return * this->__socket;
}

zmq::socket_t& zapata::ZMQReq::out() {
	return * this->__socket;
}

short int zapata::ZMQReq::type() {
	return ZMQ_REQ;
}

zapata::JSONPtr zapata::ZMQReq::send(zapata::JSONPtr _envelope) {
	zapata::ZMQ::send(_envelope);
	return this->recv();
}

void zapata::ZMQReq::listen(zapata::ZMQPollPtr _poll) {
}

zapata::ZMQRep::ZMQRep(std::string _connection, zapata::JSONPtr _options, zapata::EventEmitterPtr _emitter) : zapata::ZMQ( _connection, _options, _emitter ) {
	this->__socket = new zmq::socket_t(this->context(), ZMQ_REP);
	this->__socket->bind(_connection.data());
	zlog(string("binding REP socket on ") + _connection, zapata::notice);
}

zapata::ZMQRep::~ZMQRep() {
	this->__socket->close();
	delete this->__socket;
}

zmq::socket_t& zapata::ZMQRep::socket() {
	return * this->__socket;
}

zmq::socket_t& zapata::ZMQRep::in() {
	return * this->__socket;
}

zmq::socket_t& zapata::ZMQRep::out() {
	return * this->__socket;
}

short int zapata::ZMQRep::type() {
	return ZMQ_REP;
}

void zapata::ZMQRep::listen(zapata::ZMQPollPtr _poll) {
	_poll->poll(this->self());
}

zapata::ZMQXPubXSub::ZMQXPubXSub(std::string _connection, zapata::JSONPtr _options, zapata::EventEmitterPtr _emitter) : zapata::ZMQ( _connection, _options, _emitter ) {
	std::string _connection1(_connection.substr(0, _connection.find(",")));
	std::string _connection2(_connection.substr(_connection.find(",") + 1));
	this->__socket_sub = new zmq::socket_t(this->context(), ZMQ_XSUB);
	this->__socket_sub->bind(_connection1.data());
	this->__socket_pub = new zmq::socket_t(this->context(), ZMQ_XPUB);
	this->__socket_pub->bind(_connection2.data());
	zlog(string("binding XPUB/XSUB socket on ") + _connection, zapata::notice);
}

zapata::ZMQXPubXSub::~ZMQXPubXSub() {
	this->__socket_sub->close();
	delete this->__socket_sub;
	this->__socket_pub->close();
	delete this->__socket_pub;
}

zmq::socket_t& zapata::ZMQXPubXSub::socket() {
	return * this->__socket_sub;
}

zmq::socket_t& zapata::ZMQXPubXSub::in() {
	return * this->__socket_sub;
}

zmq::socket_t& zapata::ZMQXPubXSub::out() {
	return * this->__socket_pub;
}

void zapata::ZMQXPubXSub::loop() {
	try {
		zlog("going to proxy PUB/SUB...", zapata::notice);
		zmq::proxy(static_cast<void *>(* this->__socket_pub), static_cast<void *>(* this->__socket_sub), nullptr);
	}
	catch(zmq::error_t& _e) {
		zlog(_e.what(), zapata::emergency);
	}
}

short int zapata::ZMQXPubXSub::type() {
	return ZMQ_XPUB;
}

void zapata::ZMQXPubXSub::listen(zapata::ZMQPollPtr _poll) {
}

zapata::ZMQPubSub::ZMQPubSub(std::string _connection, zapata::JSONPtr _options, zapata::EventEmitterPtr _emitter) : zapata::ZMQ( _connection, _options, _emitter ) {
	std::string _connection1(_connection.substr(0, _connection.find(",")));
	std::string _connection2(_connection.substr(_connection.find(",") + 1));
	this->__socket_sub = new zmq::socket_t(this->context(), ZMQ_SUB);
	this->__socket_sub->connect(_connection2.data());
	this->__socket_pub = new zmq::socket_t(this->context(), ZMQ_PUB);
	this->__socket_pub->connect(_connection1.data());
	zlog(string("connecting PUB/SUB socket on ") + _connection, zapata::notice);
}

zapata::ZMQPubSub::~ZMQPubSub() {
	this->__socket_sub->close();
	delete this->__socket_sub;
	this->__socket_pub->close();
	delete this->__socket_pub;
}

zmq::socket_t& zapata::ZMQPubSub::socket() {
	return * this->__socket_sub;
}

zmq::socket_t& zapata::ZMQPubSub::in() {
	return * this->__socket_sub;
}

zmq::socket_t& zapata::ZMQPubSub::out() {
	return * this->__socket_pub;
}

short int zapata::ZMQPubSub::type() {
	return ZMQ_PUB;
}

zapata::JSONPtr zapata::ZMQPubSub::send(zapata::JSONPtr _envelope) {
	if (((int) _envelope["performative"]) != zapata::ev::Reply) {
		uuid _uuid;
		_uuid.make(UUID_MAKE_V1);
		std::string _cid(_uuid.string());
		if (_envelope["headers"]->ok()) {
			_envelope["headers"] << "X-Cid" << _cid;
		}
		else {
			_envelope << "headers" << JSON( "X-Cid" << _cid );
		}
		this->lock();
		{
			this->in().setsockopt(ZMQ_SUBSCRIBE, _cid.data(), _cid.length());
		}
		this->unlock();
	}
	zapata::ZMQ::send(_envelope);
	return zapata::undefined;
}

zapata::JSONPtr zapata::ZMQPubSub::recv() {
	zapata::JSONPtr _envelope = zapata::ZMQ::recv();
	if (((int) _envelope["performative"]) == zapata::ev::Reply) {
		std::string _cid(_envelope["channel"]->str());
		this->lock();
		{
			this->in().setsockopt(ZMQ_UNSUBSCRIBE, _cid.data(), _cid.length());
		}
		this->unlock();
	}
	return _envelope;
}

void zapata::ZMQPubSub::listen(zapata::ZMQPollPtr _poll) {
	_poll->poll(this->self());
}

short zapata::str2type(std::string _type) {
	std::transform(_type.begin(), _type.end(), _type.begin(), ::toupper);
	if (_type == "REQ/REP"){
		return ZMQ_REQ;
	}
	if (_type == "PUB/SUB"){
		return ZMQ_PUB;
	}
	if (_type == "PUSH/PULL"){
		return ZMQ_PUSH;
	}
	return ZMQ_REQ;
}
