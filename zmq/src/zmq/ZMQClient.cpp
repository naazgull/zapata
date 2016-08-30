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
#include <chrono>
#include <ossp/uuid++.hh>

zpt::ZMQ::ZMQ(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : __options( _options ), __context(1), __connection(_connection.data()), __self(this), __emitter(_emitter) {
	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	this->__id.assign(_uuid.string());
}

zpt::ZMQ::~ZMQ() {
}

std::string zpt::ZMQ::id() {
	return this->__id;
}

zpt::json zpt::ZMQ::options() {
	return this->__options;
}

zmq::context_t& zpt::ZMQ::context() {
	return this->__context;
}

std::string& zpt::ZMQ::connection() {
	return this->__connection;
}

zpt::ZMQPtr zpt::ZMQ::self() {
	return this->__self;
}

zpt::ev::emitter zpt::ZMQ::emitter() {
	return this->__emitter;
}

zpt::json zpt::ZMQ::recv() {
	std::vector<std::string> _parts;
	int _more = 0;
	size_t _more_size = sizeof(_more);

	{
		std::lock_guard< std::mutex > _lock(this->__mtx);
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

	string _channel(_parts[0]);
	string _method(_parts[1]);
	string _resource(_parts[2]);
	string _body(_parts[_parts.size() - 1]);

	zpt::json _headers = zpt::mkobj();
	if (_parts.size() > 4) {
		for (size_t _idx = 3; _idx != _parts.size() - 1; _idx++) {
			zpt::json _pair = zpt::split(_parts[_idx], ":");
			std::string _key(_parts[_idx].substr(0, _parts[_idx].find(":")));
			std::string _value(_parts[_idx].substr(_parts[_idx].find(":") + 1));
			zpt::trim(_key);
			zpt::trim(_value);
			_headers << _key << _value;
		}
	}

	zpt::json _payload;
	try {
		_payload = zpt::json(_body);
	}
	catch(zpt::SyntaxErrorException& _e) {
		_payload = zpt::undefined;
	}

	zpt::ev::performative _performative = zpt::ev::from_str(_method);
	zlog(string("<- | \033[33;40m") + _method + string("\033[0m ") + (_performative == zpt::ev::Reply ? string("\033[") + (((int) _headers["X-Status"]) <= 299 ? "32" : (((int) _headers["X-Status"]) <= 399 ? "36" : "32")) + string(";40m") + ((string) _headers["X-Status"]) + string("\033[0m ") : "") + _resource, zpt::info);
	if (_performative == zpt::ev::Reply) {
		return zpt::json(
			{
				"channel", _channel,
				"performative", _performative,
				"status", (((int) _headers["X-Status"]) > 99 ? ((int) _headers["X-Status"]) : 501),
				"resource", _resource,
				"headers", _headers,
				"payload", _payload
			}
		);
	}
	else {
		return zpt::json(
			{
				"channel", _channel,
				"performative", _performative,
				"resource", _resource,
				"headers", _headers,
				"payload", _payload
			}
		);
	}
}

zpt::json zpt::ZMQ::send(zpt::ev::performative _performative, std::string _resource, zpt::json _payload) {
	return this->send(
		zpt::json(
			{
				"channel", _resource,
				"performative", _performative,
				"resource", _resource,
				"payload", _payload
			}
		)
	);
}

zpt::json zpt::ZMQ::send(zpt::json _envelope) {
	assertz(_envelope["channel"]->ok(), "'channel' attribute is required", 412, 0);
	assertz(_envelope["performative"]->ok() && _envelope["resource"]->ok(), "'performative' and 'resource' attributes are required", 412, 0);
	assertz(!_envelope["headers"]->ok() || _envelope["headers"]->type() == zpt::JSObject, "'headers' must be of type JSON object", 412, 0);

	if ((zpt::ev::performative) ((int) _envelope["performative"]) != zpt::ev::Reply) {
		_envelope << "headers" << (zpt::ev::init_request() + _envelope["headers"]);
	}
	else {
		assertz(_envelope["status"]->ok(), "'status' attribute is required", 412, 0);
		_envelope << "headers" << (zpt::ev::init_reply() + _envelope["headers"]);
		_envelope["headers"] << "X-Status" << _envelope["status"];
	}		
	if (!_envelope["payload"]->ok()) {
		_envelope << "payload" << zpt::mkobj();
	}

	zlog(string("-> | \033[33;40m") + zpt::ev::to_str((zpt::ev::performative) ((int) _envelope["performative"])) + string("\033[0m ") + (((int) _envelope["performative"]) == zpt::ev::Reply ? string("\033[") + (((int) _envelope["headers"]["X-Status"]) <= 299 ? "32" : (((int) _envelope["headers"]["X-Status"]) <= 399 ? "36" : "32")) + string(";40m") + ((string) _envelope["headers"]["X-Status"]) + string("\033[0m ") : "") + ((string) _envelope["resource"]), zpt::info);
	std::vector<std::string> _parts;
	_parts.push_back((string) _envelope["channel"]);
	_parts.push_back(zpt::ev::to_str((zpt::ev::performative) ((int) _envelope["performative"])));
	_parts.push_back((string) _envelope["resource"]);
	if (_envelope["headers"]->ok()) {
		for (auto _header : _envelope["headers"]->obj()) {
			_parts.push_back(_header.first + string(": ") + ((string) _header.second));
		}
	}

	{
		std::lock_guard< std::mutex > _lock(this->__mtx);
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

	return zpt::undefined;
}

void zpt::ZMQ::unbind() {
	this->__self.reset();
}

zpt::ZMQReq::ZMQReq(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : zpt::ZMQ(_connection, _options, _emitter) {
	this->__socket = new zmq::socket_t(this->context(), ZMQ_REQ);
	if (_connection.find("://*") != string::npos) {
		this->__socket->bind(_connection.data());
		zlog(string("binding REQ socket on ") + _connection, zpt::notice);
	}
	else {
		this->__socket->connect(_connection.data());
		zlog(string("connecting REQ socket on ") + _connection, zpt::notice);
	}
	int _max_messages = 1000;
	int _timeout = 20000;
	this->__socket->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
	this->__socket->setsockopt(ZMQ_RCVTIMEO, & _timeout, sizeof(int));
}

zpt::ZMQReq::~ZMQReq() {
	this->__socket->close();
	delete this->__socket;
	zlog(string("releasing REQ from ") + this->connection(), zpt::notice);
}

zmq::socket_t& zpt::ZMQReq::socket() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQReq::in() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQReq::out() {
	return * this->__socket;
}

short int zpt::ZMQReq::type() {
	return ZMQ_REQ;
}

bool zpt::ZMQReq::once() {
	return true;
}

zpt::json zpt::ZMQReq::send(zpt::json _envelope) {
	zpt::ZMQ::send(_envelope);
	return this->recv();
}

void zpt::ZMQReq::listen(zpt::poll _poll) {
}

zpt::ZMQRep::ZMQRep(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : zpt::ZMQ( _connection, _options, _emitter ) {
	this->__socket = new zmq::socket_t(this->context(), ZMQ_REP);
	if (_connection.find("://*") != string::npos) {
		this->__socket->bind(_connection.data());
		zlog(string("binding REP socket on ") + _connection, zpt::notice);
	}
	else {
		this->__socket->connect(_connection.data());
		zlog(string("connecting REP socket on ") + _connection, zpt::notice);
	}
	int _max_messages = 1000;
	int _timeout = 20000;
	this->__socket->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
}

zpt::ZMQRep::~ZMQRep() {
	this->__socket->close();
	delete this->__socket;
	zlog(string("releasing REP from ") + this->connection(), zpt::notice);
}

zmq::socket_t& zpt::ZMQRep::socket() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQRep::in() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQRep::out() {
	return * this->__socket;
}

short int zpt::ZMQRep::type() {
	return ZMQ_REP;
}

bool zpt::ZMQRep::once() {
	return false;
}

void zpt::ZMQRep::listen(zpt::poll _poll) {
	_poll->poll(this->self());
}

zpt::ZMQXPubXSub::ZMQXPubXSub(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : zpt::ZMQ( _connection, _options, _emitter ) {
	std::string _connection1(_connection.substr(0, _connection.find(",")));
	std::string _connection2(_connection.substr(_connection.find(",") + 1));
	this->__socket_sub = new zmq::socket_t(this->context(), ZMQ_XSUB);
	this->__socket_sub->bind(_connection1.data());
	this->__socket_pub = new zmq::socket_t(this->context(), ZMQ_XPUB);
	this->__socket_pub->bind(_connection2.data());
	zlog(string("binding XPUB/XSUB socket on ") + _connection, zpt::notice);
	int _max_messages = 1000;
	int _timeout = 20000;
	this->__socket_sub->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket_sub->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
	this->__socket_pub->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket_pub->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
}

zpt::ZMQXPubXSub::~ZMQXPubXSub() {
	this->__socket_sub->close();
	delete this->__socket_sub;
	this->__socket_pub->close();
	delete this->__socket_pub;
	zlog(string("releasing XPUB/XSUB from ") + this->connection(), zpt::notice);
}

zmq::socket_t& zpt::ZMQXPubXSub::socket() {
	return * this->__socket_sub;
}

zmq::socket_t& zpt::ZMQXPubXSub::in() {
	return * this->__socket_sub;
}

zmq::socket_t& zpt::ZMQXPubXSub::out() {
	return * this->__socket_pub;
}

void zpt::ZMQXPubXSub::loop() {
	try {
		zlog("going to proxy PUB/SUB...", zpt::notice);
		zmq::proxy(static_cast<void*>(*this->__socket_pub), static_cast<void*>(*this->__socket_sub), nullptr);
	}
	catch(zmq::error_t& _e) {
		zlog(_e.what(), zpt::emergency);
	}
}

short int zpt::ZMQXPubXSub::type() {
	return ZMQ_XPUB_XSUB;
}

bool zpt::ZMQXPubXSub::once() {
	return false;
}

void zpt::ZMQXPubXSub::listen(zpt::poll _poll) {
}

zpt::ZMQPubSub::ZMQPubSub(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : zpt::ZMQ( _connection, _options, _emitter ) {
	std::string _connection1(_connection.substr(0, _connection.find(",")));
	std::string _connection2(_connection.substr(_connection.find(",") + 1));
	this->__socket_sub = new zmq::socket_t(this->context(), ZMQ_SUB);
	if (_connection2.find("://*") != string::npos) {
		this->__socket_sub->bind(_connection2.data());
	}
	else {
		this->__socket_sub->connect(_connection2.data());
	}
	this->__socket_pub = new zmq::socket_t(this->context(), ZMQ_PUB);
        if (_connection1.find("://*") != string::npos) {
		this->__socket_pub->bind(_connection1.data());
		zlog(string("binding PUB/SUB socket on ") + _connection, zpt::notice);
	}
	else {
		this->__socket_pub->connect(_connection1.data());
		zlog(string("connecting PUB/SUB socket on ") + _connection, zpt::notice);
	}
	int _max_messages = 1000;
	int _timeout = 20000;
	this->__socket_sub->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket_sub->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
	this->__socket_pub->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket_pub->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
}

zpt::ZMQPubSub::~ZMQPubSub() {
	this->__socket_sub->close();
	delete this->__socket_sub;
	this->__socket_pub->close();
	delete this->__socket_pub;
	zlog(string("releasing PUB/SUB from ") + this->connection(), zpt::notice);
}

zmq::socket_t& zpt::ZMQPubSub::socket() {
	return * this->__socket_sub;
}

zmq::socket_t& zpt::ZMQPubSub::in() {
	return * this->__socket_sub;
}

zmq::socket_t& zpt::ZMQPubSub::out() {
	return * this->__socket_pub;
}

short int zpt::ZMQPubSub::type() {
	return ZMQ_PUB_SUB;
}

bool zpt::ZMQPubSub::once() {
	return false;
}

zpt::json zpt::ZMQPubSub::send(zpt::json _envelope) {
	if (((int) _envelope["performative"]) != zpt::ev::Reply) {
		std::string _cid;
		if (_envelope["headers"]["X-Cid"]->ok()) {
			_cid.assign(_envelope["headers"]["X-Cid"]->str());
		}
		else {
			uuid _uuid;
			_uuid.make(UUID_MAKE_V1);
			_cid.assign(_uuid.string());
			if (_envelope["headers"]->ok()) {
				_envelope["headers"] << "X-Cid" << _cid;
			}
			else {
				_envelope << "headers" << zpt::json({ "X-Cid", _cid });
			}
		}
		{
			std::lock_guard< std::mutex > _lock(this->__mtx);
			this->in().setsockopt(ZMQ_SUBSCRIBE, _cid.data(), _cid.length());
		}
	}
	zpt::ZMQ::send(_envelope);
	return zpt::undefined;
}

zpt::json zpt::ZMQPubSub::recv() {
	zpt::json _envelope = zpt::ZMQ::recv();
	if (((int) _envelope["performative"]) == zpt::ev::Reply) {
		std::string _cid(_envelope["channel"]->str());
		{
			std::lock_guard< std::mutex > _lock(this->__mtx);
			this->in().setsockopt(ZMQ_UNSUBSCRIBE, _cid.data(), _cid.length());
		}
	}
	return _envelope;
}

void zpt::ZMQPubSub::listen(zpt::poll _poll) {
	_poll->poll(this->self());
}

zpt::ZMQPub::ZMQPub(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : zpt::ZMQ( _connection, _options, _emitter ) {
	this->__socket = new zmq::socket_t(this->context(), ZMQ_PUB);
        if (_connection.find("://*") != string::npos) {
		this->__socket->bind(_connection.data());
		zlog(string("binding PUB socket on ") + _connection, zpt::notice);
	}
	else {
		this->__socket->connect(_connection.data());
		zlog(string("connecting PUB socket on ") + _connection, zpt::notice);
	}
	int _max_messages = 1000;
	int _timeout = 20000;
	this->__socket->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
}

zpt::ZMQPub::~ZMQPub() {
	this->__socket->close();
	delete this->__socket;
	zlog(string("releasing PUB from ") + this->connection(), zpt::notice);
}

zmq::socket_t& zpt::ZMQPub::socket() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQPub::in() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQPub::out() {
	return * this->__socket;
}

short int zpt::ZMQPub::type() {
	return ZMQ_PUB;
}

bool zpt::ZMQPub::once() {
	return false;
}

zpt::json zpt::ZMQPub::recv() {
	return zpt::undefined;
}

void zpt::ZMQPub::listen(zpt::poll _poll) {
}

zpt::ZMQSub::ZMQSub(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : zpt::ZMQ( _connection, _options, _emitter ) {
	this->__socket = new zmq::socket_t(this->context(), ZMQ_SUB);
	if (_connection.find("://*") != string::npos) {
		this->__socket->bind(_connection.data());
		zlog(string("binding SUB socket on ") + _connection, zpt::notice);
	}
	else {
		this->__socket->connect(_connection.data());
		zlog(string("connecting SUB socket on ") + _connection, zpt::notice);
	}
	int _max_messages = 1000;
	int _timeout = 20000;
	this->__socket->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
	{
		std::lock_guard< std::mutex > _lock(this->__mtx);
		this->in().setsockopt(ZMQ_SUBSCRIBE, "/", 1);
	}
}

zpt::ZMQSub::~ZMQSub() {
	this->__socket->close();
	delete this->__socket;
	zlog(string("releasing SUB from ") + this->connection(), zpt::notice);
}

zmq::socket_t& zpt::ZMQSub::socket() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQSub::in() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQSub::out() {
	return * this->__socket;
}

short int zpt::ZMQSub::type() {
	return ZMQ_SUB;
}

zpt::json zpt::ZMQSub::send(zpt::json _envelope) {
	return zpt::undefined;
}

bool zpt::ZMQSub::once() {
	return false;
}

void zpt::ZMQSub::listen(zpt::poll _poll) {
	_poll->poll(this->self());
}

zpt::ZMQPush::ZMQPush(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : zpt::ZMQ( _connection, _options, _emitter ) {
	this->__socket = new zmq::socket_t(this->context(), ZMQ_PUSH);
        if (_connection.find("://*") != string::npos) {
		this->__socket->bind(_connection.data());
		zlog(string("binding PUSH socket on ") + _connection, zpt::notice);
	}
	else {
		this->__socket->connect(_connection.data());
		zlog(string("connecting PUSH socket on ") + _connection, zpt::notice);
	}
	int _max_messages = 1000;
	int _timeout = 20000;
	this->__socket->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
}


zpt::ZMQPush::~ZMQPush() {
	this->__socket->close();
	delete this->__socket;
	zlog(string("releasing PUSH from ") + this->connection(), zpt::notice);
}

zmq::socket_t& zpt::ZMQPush::socket() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQPush::in() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQPush::out() {
	return * this->__socket;
}

short int zpt::ZMQPush::type() {
	return ZMQ_PUSH;
}

zpt::json zpt::ZMQPush::recv() {
	return zpt::undefined;
}

bool zpt::ZMQPush::once() {
	return false;
}

void zpt::ZMQPush::listen(zpt::poll _poll) {
}

zpt::ZMQPull::ZMQPull(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : zpt::ZMQ( _connection, _options, _emitter ) {
	this->__socket = new zmq::socket_t(this->context(), ZMQ_PULL);
        if (_connection.find("://*") != string::npos) {
		this->__socket->bind(_connection.data());
		zlog(string("binding PULL socket on ") + _connection, zpt::notice);
	}
	else {	
		this->__socket->connect(_connection.data());
		zlog(string("connecting PULL socket on ") + _connection, zpt::notice);
	}
	int _max_messages = 1000;
	int _timeout = 20000;
	this->__socket->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
}

zpt::ZMQPull::~ZMQPull() {
	this->__socket->close();
	delete this->__socket;
	zlog(string("releasing PULL from ") + this->connection(), zpt::notice);
}

zmq::socket_t& zpt::ZMQPull::socket() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQPull::in() {
	return * this->__socket;
}

zmq::socket_t& zpt::ZMQPull::out() {
	return * this->__socket;
}

short int zpt::ZMQPull::type() {
	return ZMQ_PULL;
}

bool zpt::ZMQPull::once() {
	return false;
}

zpt::json zpt::ZMQPull::send(zpt::json _envelope) {
	return zpt::undefined;
}

void zpt::ZMQPull::listen(zpt::poll _poll) {
	_poll->poll(this->self());
}

zpt::ZMQRouterDealer::ZMQRouterDealer(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : zpt::ZMQ(_connection, _options, _emitter) {
	std::string _connection1(_connection.substr(0, _connection.find(",")));
	std::string _connection2(_connection.substr(_connection.find(",") + 1));
	this->__socket_router = new zmq::socket_t(this->context(), ZMQ_ROUTER);
	this->__socket_router->bind(_connection1.data());
	this->__socket_dealer = new zmq::socket_t(this->context(), ZMQ_DEALER);
	this->__socket_dealer->bind(_connection2.data());
	zlog(string("binding ROUTER/DEALER socket on ") + _connection, zpt::notice);
	int _max_messages = 1000;
	int _timeout = 20000;
	this->__socket_router->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket_router->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
	this->__socket_dealer->setsockopt(ZMQ_SNDHWM, & _max_messages, sizeof(int));
	this->__socket_dealer->setsockopt(ZMQ_SNDTIMEO, & _timeout, sizeof(int));
}

zpt::ZMQRouterDealer::~ZMQRouterDealer() {
	this->__socket_router->close();
	delete this->__socket_router;
	this->__socket_dealer->close();
	delete this->__socket_dealer;
	zlog(string("releasing ROUTER/DEALER from ") + this->connection(), zpt::notice);
}

zmq::socket_t& zpt::ZMQRouterDealer::socket() {
	return * this->__socket_router;
}

zmq::socket_t& zpt::ZMQRouterDealer::in() {
	return * this->__socket_router;
}

zmq::socket_t& zpt::ZMQRouterDealer::out() {
	return * this->__socket_dealer;
}

void zpt::ZMQRouterDealer::loop() {
	try {
		zlog("going to proxy REQ/REP...", zpt::notice);
		zmq::proxy(static_cast<void*>(*this->__socket_dealer), static_cast<void*>(*this->__socket_router), nullptr);
	}
	catch(zmq::error_t& _e) {
		zlog(_e.what(), zpt::emergency);
	}
}

short int zpt::ZMQRouterDealer::type() {
	return ZMQ_ROUTER_DEALER;
}

bool zpt::ZMQRouterDealer::once() {
	return false;
}

void zpt::ZMQRouterDealer::listen(zpt::poll _poll) {
}

short zpt::str2type(std::string _type) {
	std::transform(_type.begin(), _type.end(), _type.begin(), ::toupper);
	if (_type == "ROUTER/DEALER"){
		return ZMQ_ROUTER_DEALER;
	}
	if (_type == "REQ"){
		return ZMQ_REQ;
	}
	if (_type == "REP"){
		return ZMQ_REP;
	}
	if (_type == "PUB/SUB"){
		return ZMQ_PUB_SUB;
	}
	if (_type == "PUB"){
		return ZMQ_PUB;
	}
	if (_type == "SUB"){
		return ZMQ_SUB;
	}
	if (_type == "PUSH"){
		return ZMQ_PUSH;
	}
	if (_type == "PULL"){
		return ZMQ_PULL;
	}
	return -20;
}
