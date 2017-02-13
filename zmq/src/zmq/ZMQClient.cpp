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
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

zactor_t* zpt::ZMQ::__auth = nullptr;

zpt::ZMQ::ZMQ(std::string _connection, zpt::json _options) : __options( _options ), __connection(_connection.data()), __self_cert(nullptr), __peer_cert(nullptr), __poll(nullptr) {
	this->__id.assign(zpt::generate::r_uuid());
}

zpt::ZMQ::~ZMQ() {
	if (this->__self_cert != nullptr) {
		zcert_destroy(&this->__self_cert);
	}
	if (this->__peer_cert != nullptr) {
		zcert_destroy(&this->__peer_cert);
	}
}

std::string zpt::ZMQ::id() {
	return this->__id;
}

zpt::json zpt::ZMQ::options() {
	return this->__options;
}

auto zpt::ZMQ::connection() -> std::string{
	return this->__connection;
}

auto zpt::ZMQ::connection(std::string _connection) -> void{
	this->__connection.assign(_connection);
	zpt::replace(this->__connection, "tcp://*:", std::string("tcp://") + zpt::net::getip() + std::string(":"));
}

zactor_t* zpt::ZMQ::auth(std::string _client_cert_dir){
	if (zpt::ZMQ::__auth == nullptr) {
		zpt::ZMQ::__auth = zactor_new(zauth, nullptr);
		if (_client_cert_dir.length() != 0) {
			zstr_sendx(zpt::ZMQ::__auth, "CURVE", _client_cert_dir.data(), nullptr);
			zsock_wait(zpt::ZMQ::__auth);
		}
	}
	return zpt::ZMQ::__auth;
}

zcert_t* zpt::ZMQ::certificate(int _which) {
	switch (_which) {
		case ZPT_SELF_CERTIFICATE : {
			return this->__self_cert;
		}
		case ZPT_PEER_CERTIFICATE : {
			return this->__peer_cert;
		}
	}
	return nullptr;
}

void zpt::ZMQ::certificate(std::string _cert_file, int _which){
	switch (_which) {
		case ZPT_SELF_CERTIFICATE : {
			this->__self_cert = zcert_load(_cert_file.data());
			break;
		}
		case ZPT_PEER_CERTIFICATE : {
			this->__peer_cert = zcert_load(_cert_file.data());
			break;
		}
	}
}

zpt::json zpt::ZMQ::recv() {
	std::vector< std::string > _parts;
	
	std::lock_guard< std::mutex > _lock(this->in_mtx());
	zframe_t* _frame1;
	zframe_t* _frame2;
	if (zsock_recv(this->in(), "ff", &_frame1, &_frame2) == 0) {
		char* _bytes = nullptr;

		_bytes = zframe_strdup(_frame1);
		std::string _directive(std::string(_bytes, zframe_size(_frame1)));
		std::free(_bytes);
		zframe_destroy(&_frame1);

		_bytes = zframe_strdup(_frame2);
		zpt::json _envelope(std::string(_bytes, zframe_size(_frame2)));
		std::free(_bytes);
		zframe_destroy(&_frame2);

		zlog(this->connection() + std::string(" | receiving message <- ") + _directive, zpt::trace);
		return _envelope;
	}
	else {
		return { "status", 503 };
	}
}

zpt::json zpt::ZMQ::send(zpt::ev::performative _performative, std::string _resource, zpt::json _payload) {
	return this->send(
		{
			"channel", _resource,
			"performative", _performative,
			"resource", _resource, 
			"payload", _payload
		}
	);
}

zpt::json zpt::ZMQ::send(zpt::json _envelope) {
	assertz(_envelope["channel"]->ok(), "'channel' attribute is required", 412, 0);
	assertz(_envelope["performative"]->ok() && _envelope["resource"]->ok(), "'performative' and 'resource' attributes are required", 412, 0);
	assertz(!_envelope["headers"]->ok() || _envelope["headers"]->type() == zpt::JSObject, "'headers' must be of type JSON object", 412, 0);

	zpt::json _uri = zpt::uri::parse(_envelope["resource"]);
	_envelope <<
	"channel" << _uri["path"] <<
	"resource" << _uri["path"] <<
	"params" << _uri["query"];
	
	zpt::ev::performative _performative = (zpt::ev::performative) ((int) _envelope["performative"]);
	if (_performative != zpt::ev::Reply) {
		_envelope << "headers" << (zpt::ev::init_request() + _envelope["headers"]);
	}
	else {
		assertz(_envelope["status"]->ok(), "'status' attribute is required", 412, 0);
		_envelope << "headers" << (zpt::ev::init_reply() + _envelope["headers"]);
		_envelope["headers"] << "X-Status" << _envelope["status"];
	}		
	if (!_envelope["payload"]->ok()) {
		_envelope << "payload" << zpt::json::object();
	}
	int _status = (int) _envelope["headers"]["X-Status"];

	std::string _directive(zpt::ev::to_str(_performative) + std::string(" ") + _envelope["resource"]->str() + (_performative == zpt::ev::Reply ? std::string(" ") + std::to_string(_status) : std::string("")));
	zframe_t* _frame1 = zframe_new(_directive.data(), _directive.size());
	std::string _buffer(_envelope);
	zframe_t* _frame2 = zframe_new(_buffer.data(), _buffer.size());

	std::lock_guard< std::mutex > _lock(this->out_mtx());
	bool _message_sent = (zsock_send(this->out(), "ff", _frame1, _frame2) == 0);
	zframe_destroy(&_frame1);
	zframe_destroy(&_frame2);
	assertz(_message_sent, std::string("unable to send message to ") + this->connection(), 500, 0);

	zlog(this->connection() + std::string(" | sending message -> ") + _directive, zpt::trace);

	return zpt::undefined;
}

void zpt::ZMQ::relay_for(zpt::socketstream_ptr _socket, zpt::assync::reply_fn _transform) {
}

void zpt::ZMQ::relay_for(zpt::socket _socket) {
}

auto zpt::ZMQ::unlisten() -> void {
	this->__poll.reset();
}

zpt::ZMQReq::ZMQReq(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options), __self(this) {
	this->__socket = zsock_new(ZMQ_REQ);
	if (_connection.find("@tcp:") != std::string::npos) {
		size_t _port_sep = _connection.find(":", 5);
		std::string _port_part = _connection.substr(_port_sep + 1);
		int _available = 32769;
		if (_port_part != "*") {
			_available = (int) zpt::json::string(_port_part);
		}
		do {
			this->connection(_connection.substr(0, _port_sep + 1) + std::to_string(_available));
			if(zsock_attach(this->__socket, this->connection().data(), false) == 0) {
				break;
			}
			_available++;
		}
		while(_available < 60999);
		assertz(_available < 60999, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, 500, 0);
	}
	else {
		assertz(zsock_attach(this->__socket, this->connection().data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, 500, 0);
	}	
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);
	zsock_set_rcvtimeo(this->__socket, 20000);

	std::string _peer(_connection.substr(_connection.find(":") + 3));
	if (_options["curve"]["certificates"]->ok() && _options["curve"]["certificates"]["self"]->ok() && _options["curve"]["certificates"]["peers"][_peer]->ok()) {
		this->certificate(_options["curve"]["certificates"]["self"]->str(), ZPT_SELF_CERTIFICATE);
		this->certificate(_options["curve"]["certificates"]["peers"][_peer]->str(), ZPT_PEER_CERTIFICATE);
		zcert_apply(this->certificate(ZPT_SELF_CERTIFICATE), this->__socket);
		zsock_set_curve_serverkey(this->__socket, zcert_public_txt(this->certificate(ZPT_PEER_CERTIFICATE)));
		this->auth();
	}
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), zpt::notice);
}

zpt::ZMQReq::~ZMQReq() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(& this->__socket);
}

auto zpt::ZMQReq::self() const -> zpt::socket {
	return this->__self;
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

auto zpt::ZMQReq::in_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQReq::out_mtx() -> std::mutex& {
	return this->__mtx;
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

auto zpt::ZMQReq::unbind() -> void {
	if (this->__poll.get() != nullptr) {
		this->__poll->unpoll(this->self());
		this->__poll.reset();
	}
	this->__self.reset();
}

zpt::ZMQRep::ZMQRep(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options), __self(this) {
	this->__socket = zsock_new(ZMQ_REP);
	if (_connection.find("@tcp:") != std::string::npos) {
		size_t _port_sep = _connection.find(":", 5);
		std::string _port_part = _connection.substr(_port_sep + 1);
		int _available = 32769;
		if (_port_part != "*") {
			_available = (int) zpt::json::string(_port_part);
		}
		do {
			this->connection(_connection.substr(0, _port_sep + 1) + std::to_string(_available));
			if(zsock_attach(this->__socket, this->connection().data(), false) == 0) {
				break;
			}
			_available++;
		}
		while(_available < 60999);
		assertz(_available < 60999, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + _connection, 500, 0);
	}
	else {
		assertz(zsock_attach(this->__socket, this->connection().data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), 500, 0);
	}	
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);

	if (_options["curve"]["certificates"]->ok() && _options["curve"]["certificates"]["self"]->ok() && _options["curve"]["certificates"]["client_dir"]->ok()) {
		zlog(std::string("curve: private ") + _options["curve"]["certificates"]["self"]->str(), zpt::warning);
		this->certificate(_options["curve"]["certificates"]["self"]->str(), ZPT_SELF_CERTIFICATE);		
		zcert_apply(this->certificate(ZPT_SELF_CERTIFICATE), this->__socket);
		zsock_set_curve_server(this->__socket, true);
		zlog(std::string("curve: public ") + _options["curve"]["certificates"]["client_dir"]->str(), zpt::warning);
		this->auth(_options["curve"]["certificates"]["client_dir"]->str());
	}
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), zpt::notice);
}

zpt::ZMQRep::~ZMQRep() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(&this->__socket);
}

auto zpt::ZMQRep::self() const -> zpt::socket {
	return this->__self;
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

auto zpt::ZMQRep::in_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQRep::out_mtx() -> std::mutex& {
	return this->__mtx;
}

short int zpt::ZMQRep::type() {
	return ZMQ_REP;
}

bool zpt::ZMQRep::once() {
	return false;
}

void zpt::ZMQRep::listen(zpt::poll _poll) {
	this->__poll = _poll;
	this->__poll->poll(this->__self);
}

auto zpt::ZMQRep::unbind() -> void {
	if (this->__poll.get() != nullptr) {
		this->__poll->unpoll(this->self());
		this->__poll.reset();
	}
	this->__self.reset();
}

zpt::ZMQXPubXSub::ZMQXPubXSub(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options), __self(this) {
	this->__socket = zactor_new(zproxy, nullptr);
	std::string _connection1(this->connection().substr(0, this->connection().find(",")));
	std::string _connection2(this->connection().substr(this->connection().find(",") + 1));
	if (_connection2.find("@tcp:") != std::string::npos) {
		size_t _port_sep = _connection2.find(":", 5);
		std::string _port_part = _connection2.substr(_port_sep + 1);
		int _available = 32769;
		if (_port_part != "*") {
			_available = (int) zpt::json::string(_port_part);
		}
		do {
			_connection2.assign(_connection2.substr(0, _port_sep + 1) + std::to_string(_available));
			if (zstr_sendx(this->__socket, "FRONTEND", "XPUB", _connection2.data(), nullptr) != -1) {
				if (zsock_wait(this->__socket) != -1) {
					break;
				}
			}
			_available++;
		}
		while(_available < 60999);
		assertz(_available < 60999, std::string("could not attach XPUB/XSUB socket to ") + _connection, 500, 0);
	}
	else {
		zstr_sendx(this->__socket, "FRONTEND", "XPUB", _connection2.data(), nullptr);
		zsock_wait(this->__socket);
	}	
	if (_connection1.find("@tcp:") != std::string::npos) {
		size_t _port_sep = _connection1.find(":", 5);
		std::string _port_part = _connection1.substr(_port_sep + 1);
		int _available = 32769;
		if (_port_part != "*") {
			_available = (int) zpt::json::string(_port_part);
		}
		do {
			_connection1.assign(_connection1.substr(0, _port_sep + 1) + std::to_string(_available));
			if (zstr_sendx(this->__socket, "BACKEND", "XSUB", _connection1.data(), nullptr) != -1) {
				if (zsock_wait(this->__socket) != -1) {
					break;
				}
			}
			_available++;
		}
		while(_available < 60999);
		assertz(_available < 60999, std::string("could not attach XPUB/XSUB socket to ") + _connection, 500, 0);
	}
	else {
		zstr_sendx(this->__socket, "BACKEND", "XSUB", _connection1.data(), nullptr);
		zsock_wait(this->__socket);
	}
	this->connection(_connection1 + std::string(",") + _connection2);
	zlog(std::string("attaching XSUB/XPUB socket to ") + this->connection(), zpt::notice);
}

zpt::ZMQXPubXSub::~ZMQXPubXSub() {
	zlog(std::string("dettaching XPUB/XSUB from ") + this->connection(), zpt::notice);
	zactor_destroy(&this->__socket);
}

auto zpt::ZMQXPubXSub::self() const -> zpt::socket {
	return this->__self;
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

auto zpt::ZMQXPubXSub::in_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQXPubXSub::out_mtx() -> std::mutex& {
	return this->__mtx;
}

short int zpt::ZMQXPubXSub::type() {
	return ZMQ_XPUB_XSUB;
}

bool zpt::ZMQXPubXSub::once() {
	return false;
}

void zpt::ZMQXPubXSub::listen(zpt::poll _poll) {
}

auto zpt::ZMQXPubXSub::unbind() -> void {
	if (this->__poll.get() != nullptr) {
		this->__poll->unpoll(this->self());
		this->__poll.reset();
	}
	this->__self.reset();
}

zpt::ZMQPubSub::ZMQPubSub(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options), __self(this) {
	std::string _connection1(this->connection().substr(0, this->connection().find(",")));
	std::string _connection2(this->connection().substr(this->connection().find(",") + 1));
	this->__socket_sub = zsock_new(ZMQ_SUB);
	assertz(zsock_attach(this->__socket_sub, _connection2.data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket_sub)) + std::string(" socket to ") + this->connection(), 500, 0);
	this->__socket_pub = zsock_new(ZMQ_PUB);
	assertz(zsock_attach(this->__socket_pub, _connection1.data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket_pub)) + std::string(" socket to ") + this->connection(), 500, 0);
	zsock_set_rcvhwm(this->__socket_sub, 1000);
	zsock_set_sndhwm(this->__socket_pub, 1000);

	std::string _peer(_connection.substr(_connection.find(":") + 3));
	if (_options["curve"]["certificates"]->ok() && _options["curve"]["certificates"]["self"]->ok() && _options["curve"]["certificates"]["peers"][_peer]->ok()) {
		this->certificate(_options["curve"]["certificates"]["self"]->str(), ZPT_SELF_CERTIFICATE);
		this->certificate(_options["curve"]["certificates"]["peers"][_peer]->str(), ZPT_PEER_CERTIFICATE);
		std::string _peer_key(zcert_public_txt(this->certificate(ZPT_PEER_CERTIFICATE)));
		zcert_apply(this->certificate(ZPT_SELF_CERTIFICATE), this->__socket_sub);		
		zsock_set_curve_serverkey(this->__socket_sub, _peer_key.data());
		zcert_apply(this->certificate(ZPT_SELF_CERTIFICATE), this->__socket_pub);		
		zsock_set_curve_serverkey(this->__socket_pub, _peer_key.data());
	}	
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket_pub)) + std::string("/") + std::string(zsock_type_str(this->__socket_sub)) + std::string(" socket to ") + this->connection(), zpt::notice);
}

zpt::ZMQPubSub::~ZMQPubSub() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket_pub)) + std::string("/") + std::string(zsock_type_str(this->__socket_sub)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(&this->__socket_pub);
	zsock_destroy(&this->__socket_sub);
}

auto zpt::ZMQPubSub::self() const -> zpt::socket {
	return this->__self;
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

auto zpt::ZMQPubSub::in_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQPubSub::out_mtx() -> std::mutex& {
	return this->__out_mtx;
}

short int zpt::ZMQPubSub::type() {
	return ZMQ_PUB_SUB;
}

bool zpt::ZMQPubSub::once() {
	return false;
}

void zpt::ZMQPubSub::listen(zpt::poll _poll) {
	this->__poll = _poll;
	this->__poll->poll(this->__self);
}

auto zpt::ZMQPubSub::unbind() -> void {
	if (this->__poll.get() != nullptr) {
		this->__poll->unpoll(this->self());
		this->__poll.reset();
	}
	this->__self.reset();
}

void zpt::ZMQPubSub::subscribe(std::string _prefix) {
	std::lock_guard< std::mutex > _lock(this->in_mtx());
	for (size_t _p = 0; _p != 8; _p++) {
		zsock_set_subscribe(this->in(), (zpt::ev::to_str((zpt::ev::performative) _p) + std::string(" ") + _prefix).data());
	}
}

zpt::ZMQPub::ZMQPub(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options), __self(this) {
	this->__socket = zsock_new(ZMQ_PUB);
	if (_connection.find("@tcp:") != std::string::npos) {
		size_t _port_sep = _connection.find(":", 5);
		std::string _port_part = _connection.substr(_port_sep + 1);
		int _available = 32769;
		if (_port_part != "*") {
			_available = (int) zpt::json::string(_port_part);
		}
		do {
			this->connection(_connection.substr(0, _port_sep + 1) + std::to_string(_available));
			if(zsock_attach(this->__socket, this->connection().data(), false) == 0) {
				break;
			}
			_available++;
		}
		while(_available < 60999);
		assertz(_available < 60999, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), 500, 0);
	}
	else {
		assertz(zsock_attach(this->__socket, this->connection().data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), 500, 0);
	}	
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);

	std::string _peer(_connection.substr(_connection.find(":") + 3));
	if (_options["curve"]["certificates"]->ok() && _options["curve"]["certificates"]["self"]->ok() && _options["curve"]["certificates"]["peers"][_peer]->ok()) {
		this->certificate(_options["curve"]["certificates"]["self"]->str(), ZPT_SELF_CERTIFICATE);
		this->certificate(_options["curve"]["certificates"]["peers"][_peer]->str(), ZPT_PEER_CERTIFICATE);
		zcert_apply(this->certificate(ZPT_SELF_CERTIFICATE), this->__socket);
		zsock_set_curve_serverkey(this->__socket, zcert_public_txt(this->certificate(ZPT_PEER_CERTIFICATE)));
	}
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), zpt::notice);
}

zpt::ZMQPub::~ZMQPub() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(&this->__socket);
}

auto zpt::ZMQPub::self() const -> zpt::socket {
	return this->__self;
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

auto zpt::ZMQPub::in_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQPub::out_mtx() -> std::mutex& {
	return this->__mtx;
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

auto zpt::ZMQPub::unbind() -> void {
	if (this->__poll.get() != nullptr) {
		this->__poll->unpoll(this->self());
		this->__poll.reset();
	}
	this->__self.reset();
}

zpt::ZMQSub::ZMQSub(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options) {
	this->__socket = zsock_new(ZMQ_SUB);
	if (_connection.find("@tcp:") != std::string::npos) {
		size_t _port_sep = _connection.find(":", 5);
		std::string _port_part = _connection.substr(_port_sep + 1);
		int _available = 32769;
		if (_port_part != "*") {
			_available = (int) zpt::json::string(_port_part);
		}
		do {
			this->connection(_connection.substr(0, _port_sep + 1) + std::to_string(_available));
			if(zsock_attach(this->__socket, this->connection().data(), false) == 0) {
				break;
			}
			_available++;
		}
		while(_available < 60999);
		assertz(_available < 60999, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), 500, 0);
	}
	else {
		assertz(zsock_attach(this->__socket, this->connection().data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), 500, 0);
	}	
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);

	std::string _peer(_connection.substr(_connection.find(":") + 3));
	if (_options["curve"]["certificates"]->ok() && _options["curve"]["certificates"]["self"]->ok() && _options["curve"]["certificates"]["peers"][_peer]->ok()) {
		this->certificate(_options["curve"]["certificates"]["self"]->str(), ZPT_SELF_CERTIFICATE);
		this->certificate(_options["curve"]["certificates"]["peers"][_peer]->str(), ZPT_PEER_CERTIFICATE);
		zcert_apply(this->certificate(ZPT_SELF_CERTIFICATE), this->__socket);
		zsock_set_curve_serverkey(this->__socket, zcert_public_txt(this->certificate(ZPT_PEER_CERTIFICATE)));
	}

	/*{
		std::lock_guard< std::mutex > _lock(this->__mtx);
		for (size_t _p = 0; _p != 8; _p++) {
			zsock_set_subscribe(this->in(), (zpt::ev::to_str((zpt::ev::performative) _p) + std::string(" /")).data());
		}
	}*/
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), zpt::notice);
}

zpt::ZMQSub::~ZMQSub() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(&this->__socket);
}

auto zpt::ZMQSub::self() const -> zpt::socket {
	return this->__self;
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

auto zpt::ZMQSub::in_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQSub::out_mtx() -> std::mutex& {
	return this->__mtx;
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
	this->__poll = _poll;
	this->__poll->poll(this->__self);
}

auto zpt::ZMQSub::unbind() -> void {
	if (this->__poll.get() != nullptr) {
		this->__poll->unpoll(this->self());
		this->__poll.reset();
	}
	this->__self.reset();
}

void zpt::ZMQSub::subscribe(std::string _prefix) {
	std::lock_guard< std::mutex > _lock(this->in_mtx());
	for (size_t _p = 0; _p != 8; _p++) {
		zsock_set_subscribe(this->in(), (zpt::ev::to_str((zpt::ev::performative) _p) + std::string(" ") + _prefix).data());
	}
}

zpt::ZMQPush::ZMQPush(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options), __self(this) {
	this->__socket = zsock_new(ZMQ_PUSH);
	if (_connection.find("@tcp:") != std::string::npos) {
		size_t _port_sep = _connection.find(":", 5);
		std::string _port_part = _connection.substr(_port_sep + 1);
		int _available = 32769;
		if (_port_part != "*") {
			_available = (int) zpt::json::string(_port_part);
		}
		do {
			this->connection(_connection.substr(0, _port_sep + 1) + std::to_string(_available));
			if(zsock_attach(this->__socket, this->connection().data(), false) == 0) {
				break;
			}
			_available++;
		}
		while(_available < 60999);
		assertz(_available < 60999, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), 500, 0);
	}
	else {
		assertz(zsock_attach(this->__socket, this->connection().data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), 500, 0);
	}	
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);

	if (_options["curve"]["certificates"]->ok() && _options["curve"]["certificates"]["self"]->ok() && _options["curve"]["certificates"]["client_dir"]->ok()) {
		this->auth(_options["curve"]["certificates"]["client_dir"]->str());
		this->certificate(_options["curve"]["certificates"]["self"]->str(), ZPT_SELF_CERTIFICATE);
		
		zcert_apply(this->certificate(ZPT_SELF_CERTIFICATE), this->__socket);
		zsock_set_curve_server(this->__socket, true);
	}
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), zpt::notice);
}


zpt::ZMQPush::~ZMQPush() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(&this->__socket);
}

auto zpt::ZMQPush::self() const -> zpt::socket {
	return this->__self;
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

auto zpt::ZMQPush::in_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQPush::out_mtx() -> std::mutex& {
	return this->__mtx;
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

auto zpt::ZMQPush::unbind() -> void {
	if (this->__poll.get() != nullptr) {
		this->__poll->unpoll(this->self());
		this->__poll.reset();
	}
	this->__self.reset();
}

zpt::ZMQPull::ZMQPull(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options), __self(this) {
	this->__socket = zsock_new(ZMQ_PULL);
	if (_connection.find("@tcp:") != std::string::npos) {
		size_t _port_sep = _connection.find(":", 5);
		std::string _port_part = _connection.substr(_port_sep + 1);
		int _available = 32769;
		if (_port_part != "*") {
			_available = (int) zpt::json::string(_port_part);
		}
		do {
			this->connection(_connection.substr(0, _port_sep + 1) + std::to_string(_available));
			if(zsock_attach(this->__socket, this->connection().data(), false) == 0) {
				break;
			}
			_available++;
		}
		while(_available < 60999);
		assertz(_available < 60999, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), 500, 0);
	}
	else {
		assertz(zsock_attach(this->__socket, this->connection().data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), 500, 0);
	}	
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);

	std::string _peer(_connection.substr(_connection.find(":") + 3));
	if (_options["curve"]["certificates"]->ok() && _options["curve"]["certificates"]["self"]->ok() && _options["curve"]["certificates"]["peers"][_peer]->ok()) {
		this->certificate(_options["curve"]["certificates"]["self"]->str(), ZPT_SELF_CERTIFICATE);
		this->certificate(_options["curve"]["certificates"]["peers"][_peer]->str(), ZPT_PEER_CERTIFICATE);
		zcert_apply(this->certificate(ZPT_SELF_CERTIFICATE), this->__socket);
		zsock_set_curve_serverkey(this->__socket, zcert_public_txt(this->certificate(ZPT_PEER_CERTIFICATE)));
	}
	zlog(std::string("attaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), zpt::notice);
}

zpt::ZMQPull::~ZMQPull() {
	zlog(std::string("dettaching ") + std::string(zsock_type_str(this->__socket)) + std::string(" from ") + this->connection(), zpt::notice);
	zsock_destroy(&this->__socket);
}

auto zpt::ZMQPull::self() const -> zpt::socket {
	return this->__self;
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

auto zpt::ZMQPull::in_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQPull::out_mtx() -> std::mutex& {
	return this->__mtx;
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
	this->__poll = _poll;
	this->__poll->poll(this->__self);
}

auto zpt::ZMQPull::unbind() -> void {
	if (this->__poll.get() != nullptr) {
		this->__poll->unpoll(this->self());
		this->__poll.reset();
	}
	this->__self.reset();
}

zpt::ZMQRouterDealer::ZMQRouterDealer(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options), __self(this) {
	this->__socket = zactor_new(zproxy, nullptr);
	std::string _connection1(this->connection().substr(0, this->connection().find(",")));
	std::string _connection2(this->connection().substr(this->connection().find(",") + 1));
	if (_connection1.find("@tcp:") != std::string::npos) {
		size_t _port_sep = _connection1.find(":", 5);
		std::string _port_part = _connection1.substr(_port_sep + 1);
		int _available = 32769;
		if (_port_part != "*") {
			_available = (int) zpt::json::string(_port_part);
		}
		do {
			_connection1.assign(_connection1.substr(0, _port_sep + 1) + std::to_string(_available));
			if (zstr_sendx(this->__socket, "FRONTEND", "ROUTER", _connection1.data(), nullptr) != -1) {
				if (zsock_wait(this->__socket) != -1) {
					break;
				}
			}
			_available++;
		}
		while(_available < 60999);
		assertz(_available < 60999, std::string("could not attach XPUB/XSUB socket to ") + _connection, 500, 0);
	}
	else {
		zstr_sendx(this->__socket, "FRONTEND", "ROUTER", _connection1.data(), nullptr);
		zsock_wait(this->__socket);
	}
	if (_connection2.find("@tcp:") != std::string::npos) {
		size_t _port_sep = _connection2.find(":", 5);
		std::string _port_part = _connection2.substr(_port_sep + 1);
		int _available = 32769;
		if (_port_part != "*") {
			_available = (int) zpt::json::string(_port_part);
		}
		do {
			_connection2.assign(_connection2.substr(0, _port_sep + 1) + std::to_string(_available));
			if (zstr_sendx(this->__socket, "BACKEND", "DEALER", _connection2.data(), nullptr) != -1) {
				if (zsock_wait(this->__socket) != -1) {
					break;
				}
			}
			_available++;
		}
		while(_available < 60999);
		assertz(_available < 60999, std::string("could not attach XPUB/XSUB socket to ") + _connection, 500, 0);
	}
	else {
		zstr_sendx(this->__socket, "BACKEND", "DEALER", _connection2.data(), nullptr);
		zsock_wait(this->__socket);
	}	
	this->connection(_connection1);
	if (_options["curve"]["certificates"]->ok() && _options["curve"]["certificates"]["self"]->ok() && _options["curve"]["certificates"]["client_dir"]->ok()) {
		this->auth(_options["curve"]["certificates"]["client_dir"]->str());
		this->certificate(_options["curve"]["certificates"]["self"]->str(), ZPT_SELF_CERTIFICATE);
		
		zcert_apply(this->certificate(ZPT_SELF_CERTIFICATE), this->__socket);
		zsock_set_curve_server(this->__socket, true);
	}

	zlog(std::string("attaching ROUTER/DEALER socket to ") + this->connection(), zpt::notice);
}

zpt::ZMQRouterDealer::~ZMQRouterDealer() {
	zlog(std::string("dettaching ROUTER/DEALER from ") + this->connection(), zpt::notice);
	zactor_destroy(&this->__socket);
}

auto zpt::ZMQRouterDealer::self() const -> zpt::socket {
	return this->__self;
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

auto zpt::ZMQRouterDealer::in_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQRouterDealer::out_mtx() -> std::mutex& {
	return this->__mtx;
}

short int zpt::ZMQRouterDealer::type() {
	return ZMQ_ROUTER_DEALER;
}

bool zpt::ZMQRouterDealer::once() {
	return false;
}

void zpt::ZMQRouterDealer::listen(zpt::poll _poll) {
}

auto zpt::ZMQRouterDealer::unbind() -> void {
	if (this->__poll.get() != nullptr) {
		this->__poll->unpoll(this->self());
		this->__poll.reset();
	}
	this->__self.reset();
}

zpt::ZMQAssyncReq::ZMQAssyncReq(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options), __type(-1), __self(this) {
	this->__socket = zsock_new(ZMQ_REQ);
	/*if (_connection.find("@tcp:") != std::string::npos) {
		size_t _port_sep = _connection.find(":", 5);
		std::string _port_part = _connection.substr(_port_sep + 1);
		int _available = 32769;
		if (_port_part != "*") {
			_available = (int) zpt::json::string(_port_part);
		}
		do {
			this->connection(_connection.substr(0, _port_sep + 1) + std::to_string(_available));
			if(zsock_attach(this->__socket, this->connection().data(), false) == 0) {
				break;
			}
			_available++;
		}
		while(_available < 60999);
		assertz(_available < 60999, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), 500, 0);
	}
	else {*/
		assertz(zsock_attach(this->__socket, this->connection().data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__socket)) + std::string(" socket to ") + this->connection(), 500, 0);
		//}	
	zsock_set_sndhwm(this->__socket, 1000);	
	zsock_set_sndtimeo(this->__socket, 20000);
	zsock_set_rcvtimeo(this->__socket, 20000);

	std::string _peer(_connection.substr(_connection.find(":") + 3));
	if (_options["curve"]["certificates"]->ok() && _options["curve"]["certificates"]["self"]->ok() && _options["curve"]["certificates"]["peers"][_peer]->ok()) {
		this->certificate(_options["curve"]["certificates"]["self"]->str(), ZPT_SELF_CERTIFICATE);
		this->certificate(_options["curve"]["certificates"]["peers"][_peer]->str(), ZPT_PEER_CERTIFICATE);
		zcert_apply(this->certificate(ZPT_SELF_CERTIFICATE), this->__socket);
		zsock_set_curve_serverkey(this->__socket, zcert_public_txt(this->certificate(ZPT_PEER_CERTIFICATE)));
		this->auth();
	}
	zlog(std::string("attaching ASSYNC_REQ socket to ") + this->connection(), zpt::notice);
}

zpt::ZMQAssyncReq::~ZMQAssyncReq() {
	zlog(std::string("dettaching ASSYNC_REQ from ") + this->connection(), zpt::notice);
	zsock_destroy(&this->__socket);
	switch (this->__type) {
		case 1 : {
			this->__raw_socket->close();
			break;
		}
		case 2 : {
			this->__zmq_socket->unbind();
			break;
		}
	}
}

auto zpt::ZMQAssyncReq::self() const -> zpt::socket {
	return this->__self;
}

zsock_t* zpt::ZMQAssyncReq::socket() {
	return this->__socket;
}

zsock_t* zpt::ZMQAssyncReq::in() {
	return this->__socket;
}

zsock_t* zpt::ZMQAssyncReq::out() {
	return this->__socket;
}

auto zpt::ZMQAssyncReq::in_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQAssyncReq::out_mtx() -> std::mutex& {
	return this->__mtx;
}

short int zpt::ZMQAssyncReq::type() {
	return ZMQ_ASSYNC_REQ;
}

bool zpt::ZMQAssyncReq::once() {
	return true;
}

zpt::json zpt::ZMQAssyncReq::send(zpt::json _envelope) {
	zpt::ZMQ::send(_envelope);
	return zpt::undefined;
}

zpt::json zpt::ZMQAssyncReq::recv() {
	zpt::json _envelope = zpt::ZMQ::recv();
	if (!_envelope["status"]->ok() || ((int) _envelope["status"]) < 100) {
		_envelope << "status" << 501;
	}
	switch (this->__type) {
		case 1 : {
			std::string _reply = this->__raw_transformer(_envelope);
			(* this->__raw_socket) << _reply << flush;
			this->__raw_socket->close();
			break;
		}
		case 2 : {
			this->__zmq_socket->send(_envelope);
			break;
		}
	}
	return zpt::undefined;
}

void zpt::ZMQAssyncReq::relay_for(zpt::socketstream_ptr _socket, zpt::assync::reply_fn _transform) {
	this->__raw_socket = _socket;
	this->__raw_transformer = _transform;
	this->__type = 1;
}

void zpt::ZMQAssyncReq::relay_for(zpt::socket _socket) {
	this->__zmq_socket = _socket;
	this->__type = 2;
}

void zpt::ZMQAssyncReq::listen(zpt::poll _poll) {
	this->__poll = _poll;
	this->__poll->poll(this->__self);
}

auto zpt::ZMQAssyncReq::unbind() -> void {
	if (this->__poll.get() != nullptr) {
		this->__poll->unpoll(this->self());
		this->__poll.reset();
	}
	this->__self.reset();
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
	if (_type == "XPUB/XSUB"){
		return ZMQ_XPUB_XSUB;
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

auto zpt::net::getip() -> std::string {
	string _out;
	struct ifaddrs * _if_addr = nullptr, * _ifa = nullptr;
	void * _tmp_add_ptr = nullptr;

	getifaddrs(&_if_addr);
	for (_ifa = _if_addr; _ifa != nullptr; _ifa = _ifa->ifa_next) {
		if (_ifa ->ifa_addr->sa_family == AF_INET) {
			char _mask[INET_ADDRSTRLEN];
			void* _mask_ptr = &((struct sockaddr_in*) _ifa->ifa_netmask)->sin_addr;
			inet_ntop(AF_INET, _mask_ptr, _mask, INET_ADDRSTRLEN);
			if (strcmp(_mask, "255.0.0.0") != 0) {
				_tmp_add_ptr = &((struct sockaddr_in *) _ifa->ifa_addr)->sin_addr;
				char _address_buf[INET_ADDRSTRLEN];
				bzero(_address_buf, INET_ADDRSTRLEN);
				inet_ntop(AF_INET, _tmp_add_ptr, _address_buf, INET_ADDRSTRLEN);
				_out.assign(_address_buf);
				if (_if_addr != nullptr) freeifaddrs(_if_addr);
				return _out;
			}
		}
		else if (_ifa->ifa_addr->sa_family == AF_INET6) {
			char _mask[INET6_ADDRSTRLEN];
			void* _mask_ptr = &((struct sockaddr_in*) _ifa->ifa_netmask)->sin_addr;
			inet_ntop(AF_INET6, _mask_ptr, _mask, INET6_ADDRSTRLEN);
			if (strcmp(_mask, "255.0.0.0") != 0) {
				_tmp_add_ptr = &((struct sockaddr_in *) _ifa->ifa_addr)->sin_addr;
				char _address_buf[INET6_ADDRSTRLEN];
				bzero(_address_buf, INET6_ADDRSTRLEN);
				inet_ntop(AF_INET6, _tmp_add_ptr, _address_buf, INET6_ADDRSTRLEN);
				_out.assign(_address_buf);
				if (_if_addr != nullptr) freeifaddrs(_if_addr);
				return _out;
			}
		}
	}
	if (_if_addr != nullptr) freeifaddrs(_if_addr);
	return _out;
}
