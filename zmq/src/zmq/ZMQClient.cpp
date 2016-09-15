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

zpt::ZMQ::ZMQ(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : __options( _options ), __connection(_connection.data()), __self(this), __emitter(_emitter) {
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
	std::vector< std::string > _parts;
	
	std::unique_lock< std::mutex > _synchronize(this->__mtx);
	_synchronize.unlock();

	_synchronize.lock();
	zframe_t* _frame;
	if (zsock_recv(this->in(), "f", &_frame) == 0) {
		char* _bytes = zframe_strdup(_frame);
		zpt::json _envelope(std::string(_bytes, zframe_size(_frame)));		

		delete _bytes;
		zframe_destroy(&_frame);
		zpt::ev::performative _performative = (zpt::ev::performative) ((int) _envelope["performative"]);//zpt::ev::from_str(_method);
		zlog(std::string("<- | \033[33;40m") + zpt::ev::to_str(_performative) + string("\033[0m ") + (_performative == zpt::ev::Reply ? string("\033[") + (((int) _envelope["headers"]["X-Status"]) <= 299 ? "32" : (((int) _envelope["headers"]["X-Status"]) <= 399 ? "36" : "32")) + string(";40m") + ((std::string) _envelope["headers"]["X-Status"]) + string("\033[0m ") : "") + _envelope["resource"]->str(), zpt::info);

		_synchronize.unlock();
		return _envelope;
	}
	else {
		_synchronize.unlock();
		return zpt::json(
			{
				"status", 503
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

	zlog(std::string("-> | \033[33;40m") + zpt::ev::to_str((zpt::ev::performative) ((int) _envelope["performative"])) + string("\033[0m ") + (((int) _envelope["performative"]) == zpt::ev::Reply ? string("\033[") + (((int) _envelope["headers"]["X-Status"]) <= 299 ? "32" : (((int) _envelope["headers"]["X-Status"]) <= 399 ? "36" : "32")) + string(";40m") + ((std::string) _envelope["headers"]["X-Status"]) + string("\033[0m ") : "") + ((std::string) _envelope["resource"]), zpt::info);

	std::string _buffer(_envelope);
	zframe_t* _frame = zframe_new(_buffer.data(), _buffer.size());
	bool _message_sent = (zsock_send(this->out(), "f", _frame) == 0);
	zframe_destroy(&_frame);
	assertz(_message_sent, std::string("unable to send message to ") + this->connection(), 500, 0);

	return zpt::undefined;
}

void zpt::ZMQ::unbind() {
	this->__self.reset();
}

zpt::ZMQReq::ZMQReq(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : zpt::ZMQ(_connection, _options, _emitter) {
	this->__socket = zsock_new(ZMQ_REQ);
	assertz(zsock_attach(this->__socket, _connection.data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, 500, 0);
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, zpt::notice);
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);
	zsock_set_rcvtimeo(this->__socket, 20000);
}

zpt::ZMQReq::~ZMQReq() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(& this->__socket);
}

zsock_t* zpt::ZMQReq::socket() {
	return this->__socket;
}

zsock_t* zpt::ZMQReq::in() {
	return this->__socket;
}

zsock_t* zpt::ZMQReq::out() {
	return this->__socket;
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
	this->__socket = zsock_new(ZMQ_REP);
	assertz(zsock_attach(this->__socket, this->connection().data(), true) == 0, std::string("unable to attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, 500, 0);
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, zpt::notice);
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);
}

zpt::ZMQRep::~ZMQRep() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(& this->__socket);
}

zsock_t* zpt::ZMQRep::socket() {
	return this->__socket;
}

zsock_t* zpt::ZMQRep::in() {
	return this->__socket;
}

zsock_t* zpt::ZMQRep::out() {
	return this->__socket;
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
	this->__socket = zactor_new(zproxy, nullptr);
	zstr_sendx(this->__socket, "FRONTEND", "XSUB", _connection1.data(), nullptr);
	zsock_wait(this->__socket);
	zstr_sendx(this->__socket, "BACKEND", "XPUB", _connection2.data(), nullptr);
	zsock_wait(this->__socket);
	zlog(std::string("attaching XPUB/XSUB socket to ") + _connection, zpt::notice);
}

zpt::ZMQXPubXSub::~ZMQXPubXSub() {
	zlog(std::string("dettaching XPUB/XSUB from ") + this->connection(), zpt::notice);
	zactor_destroy(&this->__socket);
}

zsock_t* zpt::ZMQXPubXSub::socket() {
	return zactor_sock(this->__socket);
}

zsock_t* zpt::ZMQXPubXSub::in() {
	return zactor_sock(this->__socket);
}

zsock_t* zpt::ZMQXPubXSub::out() {
	return zactor_sock(this->__socket);
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
	this->__socket_sub = zsock_new(ZMQ_SUB);
	assertz(zsock_attach(this->__socket_sub, _connection2.data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket_sub)) + std::string(" socket to ") + _connection2, 500, 0);
	this->__socket_pub = zsock_new(ZMQ_PUB);
	assertz(zsock_attach(this->__socket_pub, _connection1.data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket_pub)) + std::string(" socket to ") + _connection1, 500, 0);
	zlog(std::string("attaching PUB/SUB socket to ") + _connection, zpt::notice);
	zsock_set_sndhwm(this->__socket_sub, 1000);	
	zsock_set_sndtimeo(this->__socket_sub, 20000);
	zsock_set_sndhwm(this->__socket_pub, 1000);	
	zsock_set_sndtimeo(this->__socket_pub, 20000);
}

zpt::ZMQPubSub::~ZMQPubSub() {
	zlog(std::string("dettaching PUB/SUB from ") + this->connection(), zpt::notice);
	zsock_destroy(&this->__socket_pub);
	zsock_destroy(&this->__socket_sub);
}

zsock_t* zpt::ZMQPubSub::socket() {
	return this->__socket_sub;
}

zsock_t* zpt::ZMQPubSub::in() {
	return this->__socket_sub;
}

zsock_t* zpt::ZMQPubSub::out() {
	return this->__socket_pub;
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
			zsock_set_subscribe(this->in(), _cid.data());
			//this->in().setsockopt(ZMQ_SUBSCRIBE, _cid.data(), _cid.length());
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
			zsock_set_unsubscribe(this->in(), _cid.data());
			//this->in().setsockopt(ZMQ_UNSUBSCRIBE, _cid.data(), _cid.length());
		}
	}
	return _envelope;
}

void zpt::ZMQPubSub::listen(zpt::poll _poll) {
	_poll->poll(this->self());
}

zpt::ZMQPub::ZMQPub(std::string _connection, zpt::json _options, zpt::ev::emitter _emitter) : zpt::ZMQ( _connection, _options, _emitter ) {
	this->__socket = zsock_new(ZMQ_PUB);
	assertz(zsock_attach(this->__socket, _connection.data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, 500, 0);
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, zpt::notice);
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);
}

zpt::ZMQPub::~ZMQPub() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(& this->__socket);
}

zsock_t* zpt::ZMQPub::socket() {
	return this->__socket;
}

zsock_t* zpt::ZMQPub::in() {
	return this->__socket;
}

zsock_t* zpt::ZMQPub::out() {
	return this->__socket;
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
	this->__socket = zsock_new(ZMQ_SUB);
	assertz(zsock_attach(this->__socket, _connection.data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, 500, 0);
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, zpt::notice);
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);
	{
		std::lock_guard< std::mutex > _lock(this->__mtx);
		zsock_set_subscribe(this->in(), "/");
		//this->in().setsockopt(ZMQ_SUBSCRIBE, "/", 1);
	}
}

zpt::ZMQSub::~ZMQSub() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(& this->__socket);
}

zsock_t* zpt::ZMQSub::socket() {
	return this->__socket;
}

zsock_t* zpt::ZMQSub::in() {
	return this->__socket;
}

zsock_t* zpt::ZMQSub::out() {
	return this->__socket;
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
	this->__socket = zsock_new(ZMQ_PUSH);
	assertz(zsock_attach(this->__socket, _connection.data(), true) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, 500, 0);
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, zpt::notice);
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);
}


zpt::ZMQPush::~ZMQPush() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(& this->__socket);
}

zsock_t* zpt::ZMQPush::socket() {
	return this->__socket;
}

zsock_t* zpt::ZMQPush::in() {
	return this->__socket;
}

zsock_t* zpt::ZMQPush::out() {
	return this->__socket;
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
	this->__socket = zsock_new(ZMQ_PULL);
	assertz(zsock_attach(this->__socket, _connection.data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, 500, 0);
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, zpt::notice);
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);
}

zpt::ZMQPull::~ZMQPull() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(& this->__socket);
}

zsock_t* zpt::ZMQPull::socket() {
	return this->__socket;
}

zsock_t* zpt::ZMQPull::in() {
	return this->__socket;
}

zsock_t* zpt::ZMQPull::out() {
	return this->__socket;
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
	this->__socket = zactor_new(zproxy, nullptr);
	// zstr_sendx(this->__socket, "VERBOSE", nullptr);
	// zsock_wait(this->__socket);
	zstr_sendx(this->__socket, "FRONTEND", "ROUTER", _connection1.data(), nullptr);
	zsock_wait(this->__socket);
	zstr_sendx(this->__socket, "BACKEND", "DEALER", _connection2.data(), nullptr);
	zsock_wait(this->__socket);
	zlog(std::string("attaching ROUTER/DEALER socket to ") + _connection, zpt::notice);
}

zpt::ZMQRouterDealer::~ZMQRouterDealer() {
	zlog(std::string("dettaching ROUTER/DEALER from ") + this->connection(), zpt::notice);
	zactor_destroy(&this->__socket);
}

zsock_t* zpt::ZMQRouterDealer::socket() {
	return zactor_sock(this->__socket);
}

zsock_t* zpt::ZMQRouterDealer::in() {
	return zactor_sock(this->__socket);
}

zsock_t* zpt::ZMQRouterDealer::out() {
	return zactor_sock(this->__socket);
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
