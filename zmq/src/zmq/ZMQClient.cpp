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
#include <zapata/zmq.h>
#include <chrono>
#include <ossp/uuid++.hh>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace zpt {
	zmq::context_t __context(1);
}

zpt::socket_ref::socket_ref() : std::string(), __poll(nullptr) {
}

zpt::socket_ref::socket_ref(const zpt::socket_ref& _rhs) : std::string(_rhs.data()), __poll(_rhs.__poll) {
}

zpt::socket_ref::socket_ref(std::string _rhs, zpt::poll _poll) : std::string(_rhs), __poll(_poll) {
}

auto zpt::socket_ref::poll(zpt::poll _poll) {
	this->__poll = _poll;
}

auto zpt::socket_ref::poll() -> zpt::poll {
	return this->__poll;
}

auto zpt::socket_ref::operator->() -> zpt::ZMQ* {
	return this->__poll->relay(this->data());
}

auto zpt::socket_ref::operator*() -> zpt::ZMQ* {
	return this->__poll->relay(this->data());
}

zpt::ZMQ::ZMQ(std::string _connection, zpt::json _options) : __options( _options ), __connection(_connection.data()),  __poll(nullptr) {
	this->__id.assign(zpt::generate::r_uuid());
}

zpt::ZMQ::~ZMQ() {
	//zdbg(std::string("disposing of ") + this->id());
}

auto zpt::ZMQ::id() -> std::string {
	return this->__id;
}

auto zpt::ZMQ::options() -> zpt::json {
	return this->__options;
}

auto zpt::ZMQ::connection() -> std::string {
	return this->__connection;
}

auto zpt::ZMQ::connection(std::string _connection) -> void{
	this->__connection.assign(_connection);
}

auto zpt::ZMQ::uri(size_t _idx) -> zpt::json {
	return this->__uri[_idx];
}

auto zpt::ZMQ::uri(std::string _uris) -> void{
	this->__uri = zpt::json::array();

	zpt::json _addresses = zpt::split(_uris, ",", true);
	for (auto _address : _addresses->arr()) {
		zpt::json _uri = zpt::uri::parse(std::string(_address));
		if (!_uri["type"]->is_string()) {
			_uri << "type" << ">";
		}
		if (!_uri["port"]->is_string()) {
			_uri << "port" << "*";
		}
		this->__uri << _uri;
	}
}

auto zpt::ZMQ::detach() -> void {
	if (this->uri()["type"] == zpt::json::string("@")) {
		this->in()->unbind(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["domain"]) + std::string(":") + std::string(this->uri()["port"]));
	}
	else {
		this->in()->disconnect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["domain"]) + std::string(":") + std::string(this->uri()["port"]));
	}
}

auto zpt::ZMQ::close() -> void {
	this->in()->close();
	this->out()->close();
}

auto zpt::ZMQ::recv() -> zpt::json {
	std::lock_guard< std::mutex > _lock(this->in_mtx());
	return zpt::ZMQ::recv(this->in());
}

auto zpt::ZMQ::recv(zmq::socket_ptr _socket) -> zpt::json {
	zmq::message_t _frame1;
	zmq::message_t _frame2;

	try {
		int64_t _more = 0;
		size_t _more_size = sizeof(_more);

		_socket->recv(&_frame1);
		_socket->getsockopt(ZMQ_RCVMORE, &_more, &_more_size);
		if (_more != 0) {
			_socket->recv(&_frame2);
		}
	
		std::string _directive(static_cast<char*>(_frame1.data()), _frame1.size());
		std::string _raw(static_cast<char*>(_frame2.data()), _frame2.size());
		try {
			zpt::json _envelope(_raw);
			zverbose(std::string("< ") + _directive);
			zverbose(zpt::json::pretty(_envelope));
			return _envelope;
		}
		catch(zpt::SyntaxErrorException& _e) {
			return { "error", true, "status", 400, "payload", { "text", _e.what() } };
		}
	}
	catch(zmq::error_t& _e) {
		zlog(_e.what(), zpt::error);
		throw;
	}
	return { "error", true, "status", 503 };	
}

auto zpt::ZMQ::send(zpt::ev::performative _performative, std::string _resource, zpt::json _payload) -> zpt::json {
	return this->send(
		{
			"channel", _resource,
			"performative", _performative,
			"resource", _resource, 
			"payload", _payload
		}
	);
}

auto zpt::ZMQ::send(zpt::json _envelope) -> zpt::json {
	std::lock_guard< std::mutex > _lock(this->out_mtx());
	return zpt::ZMQ::send(_envelope, this->out());
}

auto zpt::ZMQ::send(zpt::ev::performative _performative, std::string _resource, zpt::json _payload, zmq::socket_ptr _socket) -> zpt::json {
	return zpt::ZMQ::send(
		{
			"channel", _resource,
			"performative", _performative,
			"resource", _resource, 
			"payload", _payload
		},
		_socket
	);
}

auto zpt::ZMQ::send(zpt::json _envelope, zmq::socket_ptr _socket) -> zpt::json {
	assertz(_envelope["channel"]->ok(), "'channel' attribute is required", 412, 0);
	assertz(_envelope["performative"]->ok() && _envelope["resource"]->ok(), "'performative' and 'resource' attributes are required", 412, 0);
	assertz(!_envelope["headers"]->ok() || _envelope["headers"]->type() == zpt::JSObject, "'headers' must be of type JSON object", 412, 0);

	zpt::json _uri = zpt::uri::parse(_envelope["resource"]);
	_envelope <<
	"channel" << _uri["path"] <<
	"resource" << _uri["path"] <<
	"params" << ((_envelope["params"]->is_object() ? _envelope["params"] : zpt::undefined) + _uri["query"]);
	
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

	std::string _directive(zpt::ev::to_str(_performative) + std::string(" ") + _envelope["resource"]->str() + (_performative == zpt::ev::Reply ? std::string(" ") + std::to_string(_status) : std::string("")));// + std::string(" ") + zpt::generate::r_uuid());
	std::string _buffer(_envelope);
	
	zmq::message_t _frame1(_directive.length());
	zmq::message_t _frame2(_buffer.length());
	memcpy(_frame1.data(), _directive.data(), _directive.length());
	memcpy(_frame2.data(), _buffer.data(), _buffer.length());

	assertz(_socket->send(_frame1, ZMQ_SNDMORE), std::string("unable to send message"), 500, 0);
	assertz(_socket->send(_frame2), std::string("unable to send message"), 500, 0);

	return zpt::undefined;
}

auto zpt::ZMQ::relay_for(zpt::socketstream_ptr _socket, zpt::assync::reply_fn _transform) -> void {
}

auto zpt::ZMQ::relay_for(zpt::socket_ref _socket) -> void {
}

zpt::ZMQReq::ZMQReq(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options)/*, __context(0)*/, __socket(nullptr)/*, __self(this)*/ {
	this->__socket = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_REQ));
	// this->__socket->setsockopt(ZMQ_SNDTIMEO, 10000);
	this->uri(_connection);
	if (this->uri()["scheme"] == zpt::json::string("tcp")) {
		if (this->uri()["type"] == zpt::json::string("@")) {
			int _available = 1025;
			if (this->uri()["port"] != zpt::json::string("*")) {
				_available = int(this->uri()["port"]);
			}
			do {
				this->connection(std::string("@tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
				this->uri() << "port" << std::to_string(_available);
				try {
					this->__socket->bind(std::string("tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
					break;
				}
				catch(zmq::error_t& _e) {
				}
				_available++;
			}
			while(_available < 60999);
			assertz(_available < 60999, std::string("could not attach socket to ") + _connection, 500, 0);
		}
		else {
			this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			try {
				this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			}
			catch(zmq::error_t& _e) {
				assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
			}
		}
	}
	else {
		this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		try {
			this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		}
		catch(zmq::error_t& _e) {
			assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
		}
	}

	ztrace(std::string("attaching socket to ") + this->connection());
}

zpt::ZMQReq::~ZMQReq() {
	if (this->__socket.get() != nullptr) {
		ztrace(std::string("dettaching from ") + this->connection());
		this->__socket->close();
		this->__socket.reset();
	}
}

auto zpt::ZMQReq::socket() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQReq::in() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQReq::out() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQReq::fd() -> int {
	int _return = 0;
	size_t _size = sizeof (_return);
	this->__socket->getsockopt(ZMQ_FD, &_return, &_size);
	return _return;
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

zpt::json zpt::ZMQReq::send(zpt::json _envelope) {
	zpt::ZMQ::send(_envelope);
	return this->recv();
}

zpt::ZMQRep::ZMQRep(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options)/*, __context(0)*/, __socket(nullptr)/*, __self(this)*/ {
	this->__socket = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_REP));
	// this->__socket->setsockopt(ZMQ_RCVTIMEO, 10000);
	this->uri(_connection);
	if (this->uri()["scheme"] == zpt::json::string("tcp")) {
		if (this->uri()["type"] == zpt::json::string("@")) {
			int _available = 1025;
			if (this->uri()["port"] != zpt::json::string("*")) {
				_available = int(this->uri()["port"]);
			}
			do {
				this->connection(std::string("@tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
				this->uri() << "port" << std::to_string(_available);
				try {
					this->__socket->bind(std::string("tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
					break;
				}
				catch(zmq::error_t& _e) {
				}
				_available++;
			}
			while(_available < 60999);
			assertz(_available < 60999, std::string("could not attach socket to ") + _connection, 500, 0);
		}
		else {
			this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			try {
				this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			}
			catch(zmq::error_t& _e) {
				assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
			}
		}
	}
	else {
		this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		try {
			this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		}
		catch(zmq::error_t& _e) {
			assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
		}
	}

	ztrace(std::string("attaching socket to ") + this->connection());
}

zpt::ZMQRep::~ZMQRep() {
	if (this->__socket.get() != nullptr) {
		ztrace(std::string("dettaching from ") + this->connection());
		this->__socket->close();
		this->__socket.reset();
	}
}

auto zpt::ZMQRep::socket() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQRep::in() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQRep::out() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQRep::fd() -> int {
	int _return = 0;
	size_t _size = sizeof (_return);
	this->__socket->getsockopt(ZMQ_FD, &_return, &_size);
	return _return;
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

zpt::ZMQXPubXSub::ZMQXPubXSub(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options)/*, __context(0)*/, __socket_sub(nullptr), __socket_pub(nullptr)/*, __self(this)*/ {
	this->__socket_sub = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_XSUB));
	this->__socket_pub = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_XPUB));
	this->uri(_connection);

	if (this->uri(1)["scheme"] == zpt::json::string("tcp")) {
		if (this->uri(1)["type"] == zpt::json::string("@")) {
			int _available = 1025;
			if (this->uri(1)["port"] != zpt::json::string("*")) {
				_available = int(this->uri(1)["port"]);
			}
			do {
				this->uri(1) << "port" << std::to_string(_available);
				try {
					this->__socket_pub->bind(std::string("tcp://") + std::string(this->uri(1)["domain"]) + std::string(":") + std::to_string(_available));
					break;
				}
				catch(zmq::error_t& _e) {
				}
				_available++;
			}
			while(_available < 60999);
			assertz(_available < 60999, std::string("could not attach socket to ") + _connection, 500, 0);
		}
		else {
			try {
				this->__socket_pub->connect(std::string(this->uri(1)["scheme"]) + std::string("://") + std::string(this->uri(1)["authority"]));
			}
			catch(zmq::error_t& _e) {
				assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
			}
		}
	}
	else {
		try {
			this->__socket_pub->connect(std::string(this->uri(1)["scheme"]) + std::string("://") + std::string(this->uri(1)["authority"]));
		}
		catch(zmq::error_t& _e) {
			assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
		}
	}

	if (this->uri(0)["scheme"] == zpt::json::string("tcp")) {
		if (this->uri(0)["type"] == zpt::json::string("@")) {
			int _available = 1025;
			if (this->uri(0)["port"] != zpt::json::string("*")) {
				_available = int(this->uri(0)["port"]);
			}
			do {
				this->uri(0) << "port" << std::to_string(_available);
				try {
					this->__socket_sub->bind(std::string("tcp://") + std::string(this->uri(0)["domain"]) + std::string(":") + std::to_string(_available));
					break;
				}
				catch(zmq::error_t& _e) {
				}
				_available++;
			}
			while(_available < 60999);
			assertz(_available < 60999, std::string("could not attach socket to ") + _connection, 500, 0);
		}
		else {
			try {
				this->__socket_sub->connect(std::string(this->uri(0)["scheme"]) + std::string("://") + std::string(this->uri(0)["authority"]));
			}
			catch(zmq::error_t& _e) {
				assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
			}
		}
	}
	else {
		try {
			this->__socket_sub->connect(std::string(this->uri(0)["scheme"]) + std::string("://") + std::string(this->uri(0)["authority"]));
		}
		catch(zmq::error_t& _e) {
			assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
		}
	}

	std::thread _proxy(
		[] (zmq::socket_ptr _frontend, zmq::socket_ptr _backend) -> void {
			zmq::proxy((void*)(*_frontend), (void*)(*_backend), nullptr);
		},
		this->__socket_pub,
		this->__socket_sub
	);
	_proxy.detach();
	
	this->connection(_connection);
	ztrace(std::string("attaching socket to ") + this->connection());
}

zpt::ZMQXPubXSub::~ZMQXPubXSub() {
	ztrace(std::string("dettaching from ") + this->connection());
	if (this->__socket_sub.get() != nullptr) {
		this->__socket_sub->close();
		this->__socket_sub.reset();
	}
	if (this->__socket_pub.get() != nullptr) {
		this->__socket_pub->close();
		this->__socket_pub.reset();
	}
}

// auto zpt::ZMQXPubXSub::self() const -> zpt::socket_ref {
// 	return this->__self;
// }

auto zpt::ZMQXPubXSub::socket() -> zmq::socket_ptr {
	return this->__socket_sub;
}

auto zpt::ZMQXPubXSub::in() -> zmq::socket_ptr {
	return this->__socket_sub;
}

auto zpt::ZMQXPubXSub::out() -> zmq::socket_ptr {
	return this->__socket_pub;
}

auto zpt::ZMQXPubXSub::fd() -> int {
	int _return = 0;
	size_t _size = sizeof (_return);
	this->__socket_sub->getsockopt(ZMQ_FD, &_return, &_size);
	return _return;
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

zpt::ZMQPubSub::ZMQPubSub(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options)/*, __context(0)*/, __socket_sub(nullptr), __socket_pub(nullptr)/*, __self(this)*/ {
	this->__socket_sub = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_SUB));
	this->__socket_pub = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_PUB));
	this->uri(_connection);

	
	this->__socket_pub->connect(std::string(this->uri(0)["scheme"]) + std::string("://") + std::string(this->uri(0)["authority"]));
	this->__socket_sub->connect(std::string(this->uri(1)["scheme"]) + std::string("://") + std::string(this->uri(1)["authority"]));

	this->connection(_connection);
	ztrace(std::string("attaching socket to ") + this->connection());
}

zpt::ZMQPubSub::~ZMQPubSub() {
	ztrace(std::string("dettaching from ") + this->connection());
	if (this->__socket_sub.get() != nullptr) {
		this->__socket_sub->close();
		this->__socket_sub.reset();
	}
	if (this->__socket_pub.get() != nullptr) {
		this->__socket_pub->close();
		this->__socket_pub.reset();
	}
}

auto zpt::ZMQPubSub::socket() -> zmq::socket_ptr {
	return this->__socket_sub;
}

auto zpt::ZMQPubSub::in() -> zmq::socket_ptr {
	return this->__socket_sub;
}

auto zpt::ZMQPubSub::out() -> zmq::socket_ptr {
	return this->__socket_pub;
}

auto zpt::ZMQPubSub::fd() -> int {
	int _return = 0;
	size_t _size = sizeof (_return);
	this->__socket_sub->getsockopt(ZMQ_FD, &_return, &_size);
	return _return;
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

void zpt::ZMQPubSub::subscribe(std::string _prefix) {
	std::lock_guard< std::mutex > _lock(this->in_mtx());
	for (size_t _p = 0; _p != 8; _p++) {
		this->__socket_sub->setsockopt (ZMQ_SUBSCRIBE, (zpt::ev::to_str((zpt::ev::performative) _p) + std::string(" ") + _prefix).data(), 0);
	}
}

zpt::ZMQPub::ZMQPub(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options)/*, __context(0)*/, __socket(nullptr)/*, __self(this)*/ {
	this->__socket = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_PUB));
	this->uri(_connection);
	if (this->uri()["scheme"] == zpt::json::string("tcp")) {
		if (this->uri()["type"] == zpt::json::string("@")) {
			int _available = 1025;
			if (this->uri()["port"] != zpt::json::string("*")) {
				_available = int(this->uri()["port"]);
			}
			do {
				this->connection(std::string("@tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
				this->uri() << "port" << std::to_string(_available);
				try {
					this->__socket->bind(std::string("tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
					break;
				}
				catch(zmq::error_t& _e) {
				}
				_available++;
			}
			while(_available < 60999);
			assertz(_available < 60999, std::string("could not attach socket to ") + _connection, 500, 0);
		}
		else {
			this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			try {
				this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			}
			catch(zmq::error_t& _e) {
				assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
			}
		}
	}
	else {
		this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		try {
			this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		}
		catch(zmq::error_t& _e) {
			assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
		}
	}

	ztrace(std::string("attaching socket to ") + this->connection());
}

zpt::ZMQPub::~ZMQPub() {
	if (this->__socket.get() != nullptr) {
		ztrace(std::string("dettaching from ") + this->connection());
		this->__socket->close();
		this->__socket.reset();
	}
}

auto zpt::ZMQPub::socket() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQPub::in() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQPub::out() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQPub::fd() -> int {
	int _return = 0;
	size_t _size = sizeof (_return);
	this->__socket->getsockopt(ZMQ_FD, &_return, &_size);
	return _return;
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

zpt::json zpt::ZMQPub::recv() {
	return zpt::undefined;
}

zpt::ZMQSub::ZMQSub(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options)/*, __context(0)*/, __socket(nullptr)/*, __self(this)*/ {
	this->__socket = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_SUB));
	this->uri(_connection);
	if (this->uri()["scheme"] == zpt::json::string("tcp")) {
		if (this->uri()["type"] == zpt::json::string("@")) {
			int _available = 1025;
			if (this->uri()["port"] != zpt::json::string("*")) {
				_available = int(this->uri()["port"]);
			}
			do {
				this->connection(std::string("@tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
				this->uri() << "port" << std::to_string(_available);
				try {
					this->__socket->bind(std::string("tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
					break;
				}
				catch(zmq::error_t& _e) {
				}
				_available++;
			}
			while(_available < 60999);
			assertz(_available < 60999, std::string("could not attach socket to ") + _connection, 500, 0);
		}
		else {
			this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			try {
				this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			}
			catch(zmq::error_t& _e) {
				assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
			}
		}
	}
	else {
		this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		try {
			this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		}
		catch(zmq::error_t& _e) {
			assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
		}
	}

	ztrace(std::string("attaching socket to ") + this->connection());
}

zpt::ZMQSub::~ZMQSub() {
	if (this->__socket.get() != nullptr) {
		ztrace(std::string("dettaching from ") + this->connection());
		this->__socket->close();
		this->__socket.reset();
	}
}

auto zpt::ZMQSub::socket() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQSub::in() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQSub::out() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQSub::fd() -> int {
	int _return = 0;
	size_t _size = sizeof (_return);
	this->__socket->getsockopt(ZMQ_FD, &_return, &_size);
	return _return;
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

void zpt::ZMQSub::subscribe(std::string _prefix) {
	std::lock_guard< std::mutex > _lock(this->in_mtx());
	for (size_t _p = 0; _p != 8; _p++) {
		this->__socket->setsockopt (ZMQ_SUBSCRIBE, (zpt::ev::to_str((zpt::ev::performative) _p) + std::string(" ") + _prefix).data(), 0);
	}
}

zpt::ZMQPush::ZMQPush(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options)/*, __context(0)*/, __socket(nullptr)/*, __self(this)*/ {
	this->__socket = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_PUSH));
	this->uri(_connection);
	if (this->uri()["scheme"] == zpt::json::string("tcp")) {
		if (this->uri()["type"] == zpt::json::string("@")) {
			int _available = 1025;
			if (this->uri()["port"] != zpt::json::string("*")) {
				_available = int(this->uri()["port"]);
			}
			do {
				this->connection(std::string("@tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
				this->uri() << "port" << std::to_string(_available);
				try {
					this->__socket->bind(std::string("tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
					break;
				}
				catch(zmq::error_t& _e) {
				}
				_available++;
			}
			while(_available < 60999);
			assertz(_available < 60999, std::string("could not attach socket to ") + _connection, 500, 0);
		}
		else {
			this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			try {
				this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			}
			catch(zmq::error_t& _e) {
				assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
			}
		}
	}
	else {
		this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		try {
			this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		}
		catch(zmq::error_t& _e) {
			assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
		}
	}
	this->__socket->setsockopt(ZMQ_SNDHWM, 100000);	

	ztrace(std::string("attaching socket to ") + this->connection());
}


zpt::ZMQPush::~ZMQPush() {
	if (this->__socket.get() != nullptr) {
		ztrace(std::string("dettaching from ") + this->connection());
		this->__socket->close();
		this->__socket.reset();
	}
}

auto zpt::ZMQPush::socket() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQPush::in() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQPush::out() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQPush::fd() -> int {
	int _return = 0;
	size_t _size = sizeof (_return);
	this->__socket->getsockopt(ZMQ_FD, &_return, &_size);
	return _return;
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

zpt::ZMQPull::ZMQPull(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options)/*, __context(0)*/, __socket(nullptr)/*, __self(this)*/ {
	this->__socket = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_PULL));
	this->uri(_connection);
	if (this->uri()["scheme"] == zpt::json::string("tcp")) {
		if (this->uri()["type"] == zpt::json::string("@")) {
			int _available = 1025;
			if (this->uri()["port"] != zpt::json::string("*")) {
				_available = int(this->uri()["port"]);
			}
			do {
				this->connection(std::string("@tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
				this->uri() << "port" << std::to_string(_available);
				try {
					this->__socket->bind(std::string("tcp://") + std::string(this->uri()["domain"]) + std::string(":") + std::to_string(_available));
					break;
				}
				catch(zmq::error_t& _e) {
				}
				_available++;
			}
			while(_available < 60999);
			assertz(_available < 60999, std::string("could not attach socket to ") + _connection, 500, 0);
		}
		else {
			this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			try {
				this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
			}
			catch(zmq::error_t& _e) {
				assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
			}
		}
	}
	else {
		this->connection(std::string(this->uri()["type"]) + std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		try {
			this->__socket->connect(std::string(this->uri()["scheme"]) + std::string("://") + std::string(this->uri()["authority"]));
		}
		catch(zmq::error_t& _e) {
			assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
		}
	}

	ztrace(std::string("attaching socket to ") + this->connection());
}

zpt::ZMQPull::~ZMQPull() {
	if (this->__socket.get() != nullptr) {
		ztrace(std::string("dettaching from ") + this->connection());
		this->__socket->close();
		this->__socket.reset();
	}
}

// auto zpt::ZMQPull::self() const -> zpt::socket_ref {
// 	return this->__self;
// }

auto zpt::ZMQPull::socket() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQPull::in() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQPull::out() -> zmq::socket_ptr {
	return this->__socket;
}

auto zpt::ZMQPull::fd() -> int {
	int _return = 0;
	size_t _size = sizeof (_return);
	this->__socket->getsockopt(ZMQ_FD, &_return, &_size);
	return _return;
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

zpt::json zpt::ZMQPull::send(zpt::json _envelope) {
	return zpt::undefined;
}

zpt::ZMQRouterDealer::ZMQRouterDealer(std::string _connection, zpt::json _options) : zpt::ZMQ(_connection, _options)/*, __context(0)*/, __socket_router(nullptr), __socket_dealer(nullptr)/*, __self(this)*/ {
	this->__socket_router = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_ROUTER));
	this->__socket_dealer = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_DEALER));
	this->uri(_connection);

	if (this->uri(0)["scheme"] == zpt::json::string("tcp")) {
		if (this->uri(0)["type"] == zpt::json::string("@")) {
			int _available = 1025;
			if (this->uri(0)["port"] != zpt::json::string("*")) {
				_available = int(this->uri(0)["port"]);
			}
			do {
				this->uri(0) << "port" << std::to_string(_available);
				try {
					this->__socket_router->bind(std::string("tcp://") + std::string(this->uri(0)["domain"]) + std::string(":") + std::to_string(_available));
					break;
				}
				catch(zmq::error_t& _e) {
				}
				_available++;
			}
			while(_available < 60999);
			assertz(_available < 60999, std::string("could not attach socket to ") + _connection, 500, 0);
		}
		else {
			try {
				this->__socket_router->connect(std::string(this->uri(0)["scheme"]) + std::string("://") + std::string(this->uri(0)["authority"]));
			}
			catch(zmq::error_t& _e) {
				assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
			}
		}
	}
	else {
		try {
			this->__socket_router->connect(std::string(this->uri(0)["scheme"]) + std::string("://") + std::string(this->uri(0)["authority"]));
		}
		catch(zmq::error_t& _e) {
			assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
		}
	}

	if (this->uri(1)["scheme"] == zpt::json::string("tcp")) {
		if (this->uri(1)["type"] == zpt::json::string("@")) {
			int _available = 1025;
			if (this->uri(1)["port"] != zpt::json::string("*")) {
				_available = int(this->uri(1)["port"]);
			}
			do {
				this->uri(1) << "port" << std::to_string(_available);
				try {
					this->__socket_dealer->bind(std::string("tcp://") + std::string(this->uri(1)["domain"]) + std::string(":") + std::to_string(_available));
					break;
				}
				catch(zmq::error_t& _e) {
				}
				_available++;
			}
			while(_available < 60999);
			assertz(_available < 60999, std::string("could not attach socket to ") + _connection, 500, 0);
		}
		else {
			try {
				this->__socket_dealer->connect(std::string(this->uri(1)["scheme"]) + std::string("://") + std::string(this->uri(1)["authority"]));
			}
			catch(zmq::error_t& _e) {
				assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
			}
		}
	}
	else {
		try {
			this->__socket_dealer->connect(std::string(this->uri(1)["scheme"]) + std::string("://") + std::string(this->uri(1)["authority"]));
		}
		catch(zmq::error_t& _e) {
			assertz(false, std::string("could not attach socket to ") + this->connection(), 500, 0);
		}
	}

	std::thread _proxy(
		[] (zmq::socket_ptr _frontend, zmq::socket_ptr _backend) -> void {
			zmq::proxy((void*)(*_frontend), (void*)(*_backend), nullptr);			
		},
		this->__socket_router,
		this->__socket_dealer
	);
	_proxy.detach();
	
	this->connection(_connection);
	ztrace(std::string("attaching socket to ") + this->connection());
}

zpt::ZMQRouterDealer::~ZMQRouterDealer() {
	ztrace(std::string("dettaching from ") + this->connection());
	if (this->__socket_router.get() != nullptr) {
		this->__socket_router->close();
		this->__socket_router.reset();
	}
	if (this->__socket_dealer.get() != nullptr) {
		this->__socket_dealer->close();
		this->__socket_dealer.reset();
	}
}

auto zpt::ZMQRouterDealer::socket() -> zmq::socket_ptr {
	return this->__socket_router;
}

auto zpt::ZMQRouterDealer::in() -> zmq::socket_ptr {
	return this->__socket_router;
}

auto zpt::ZMQRouterDealer::out() -> zmq::socket_ptr {
	return this->__socket_dealer;
}

auto zpt::ZMQRouterDealer::fd() -> int {
	int _return = 0;
	size_t _size = sizeof (_return);
	this->__socket_router->getsockopt(ZMQ_FD, &_return, &_size);
	return _return;
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

zpt::ZMQHttp::ZMQHttp(zpt::socketstream_ptr _underlying, zpt::json _options) : zpt::ZMQ("http://*:*", _options), __underlying(_underlying) {
}

zpt::ZMQHttp::~ZMQHttp() {
}

auto zpt::ZMQHttp::socket() -> zmq::socket_ptr {
	return zmq::socket_ptr(nullptr);
}

auto zpt::ZMQHttp::in() -> zmq::socket_ptr {
	return zmq::socket_ptr(nullptr);
}

auto zpt::ZMQHttp::out() -> zmq::socket_ptr {
	return zmq::socket_ptr(nullptr);
}

auto zpt::ZMQHttp::fd() -> int {
	return this->__underlying->buffer().get_socket();;
}

auto zpt::ZMQHttp::close() -> void {
}

auto zpt::ZMQHttp::in_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQHttp::out_mtx() -> std::mutex& {
	return this->__mtx;
}

auto zpt::ZMQHttp::type() -> short int {
	return ZMQ_HTTP_RAW;
}

auto zpt::ZMQHttp::send(zpt::json _envelope) -> zpt::json {
	assertz(_envelope["resource"]->ok(), "''resource' attributes are required", 412, 0);

	zpt::json _uri = zpt::uri::parse(_envelope["resource"]);
	_envelope <<
	"channel" << _uri["path"] <<
	"resource" << _uri["path"] <<
	"params" << ((_envelope["params"]->is_object() ? _envelope["params"] : zpt::undefined) + _uri["query"]);
	
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
	std::string _reply = std::string(zpt::rest::zmq2http(_envelope));
	//zdbg(_reply);
	(*this->__underlying) << _reply << flush;
	return zpt::undefined;
}

auto zpt::ZMQHttp::recv() -> zpt::json {
	zpt::http::req _request;
	try {
		char _c;
		assertz(::recv(this->fd(), &_c, 1, MSG_PEEK) > 0, "the underlying socket was closed by peer", 503, 0);
		(*this->__underlying) >> _request;
	}
	catch(zpt::SyntaxErrorException& _e) {
		zlog(std::string("error while parsing HTTP request: syntax error exception"), zpt::error);
		return { "error", true };
	}	
	zpt::json _in = zpt::rest::http2zmq(_request);
	return _in;
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
				if (_out.length() == 0 || _out == "::") {
					return "127.0.0.1";
				}
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
				if (_out.length() == 0 || _out == "::") {
					return "127.0.0.1";
				}
				return _out;
			}
		}
	}
	if (_if_addr != nullptr) freeifaddrs(_if_addr);
	if (_out.length() == 0 || _out == "::") {
		return "127.0.0.1";
	}
	return _out;
}

auto zpt::rest::http2zmq(zpt::http::req _request) -> zpt::json {
	zpt::json _return = zpt::json::object();
	_return <<
		"channel" << _request->url() <<
		"performative" << _request->method() <<
		"resource" << _request->url();
	
	zpt::json _payload;
	if (_request->body() != "") {
		if (_request->header("Content-Type").find("application/x-www-form-urlencoded") != std::string::npos) {
			_payload = zpt::rest::http::deserialize(_request->body());
		}
		else if (_request->header("Content-Type").find("application/json") != std::string::npos) {
			try {
				_payload = zpt::json(_request->body());
			}
			catch(zpt::SyntaxErrorException& _e) {
			}
		}
		else {
			_payload = { "text", _request->body() };
		}
	}
	else {
		_payload = zpt::json::object();
	}
	_return << "payload" << _payload;

	if (_request->params().size() != 0) {
		zpt::json _params = zpt::json::object();
		for (auto _param : _request->params()) {
			_params << _param.first << zpt::url::r_decode(_param.second);
		}
		_return << "params" << _params;
	}
	
	zpt::json _headers = zpt::json::object();
	for (auto _header : _request->headers()) {
		_headers << _header.first << _header.second;
	}
	if (_headers->obj()->size() != 0) {
		_return << "headers" << _headers;
	}
	
	return _return;
}

auto zpt::rest::http2zmq(zpt::http::rep _reply) -> zpt::json {
	zpt::json _return = zpt::json::object();
	_return <<
	"status" << (int) _reply->status() <<
	"channel" << zpt::generate::r_uuid() <<
	"performative" << zpt::ev::Reply <<
	"resource" << zpt::generate::r_uuid();
	
	zpt::json _payload;
	if (_reply->body() != "") {
		if (_reply->header("Content-Type").find("application/x-www-form-urlencoded") != std::string::npos) {
			_payload = zpt::rest::http::deserialize(_reply->body());
		}
		else if (_reply->header("Content-Type").find("application/json") != std::string::npos) {
			try {
				_payload = zpt::json(_reply->body());
			}
			catch(zpt::SyntaxErrorException& _e) {
			}
		}
		else {
			_payload = { "text", _reply->body() };
		}
	}
	else {
		_payload = zpt::json::object();
	}
	_return << "payload" << _payload;

	zpt::json _headers = zpt::json::object();
	for (auto _header : _reply->headers()) {
		_headers << _header.first << _header.second;
	}
	if (_headers->obj()->size() != 0) {
		_return << "headers" << _headers;
	}
	
	return _return;
}

auto zpt::rest::zmq2http(zpt::json _out) -> zpt::http::rep {
	zpt::http::rep _return;
	_return->status((zpt::HTTPStatus) ((int) _out["status"]));
	
	_out << "headers" << (zpt::ev::init_reply() + _out["headers"]);
	for (auto _header : _out["headers"]->obj()) {
		_return->header(_header.first, ((std::string) _header.second));
	}
	
	if (_out["payload"]->ok()) {
		if (_out["payload"]->is_object() || _out["payload"]->is_array()) {
			_return->header("Content-Type", "application/json");
		}
		else {
			_return->header("Content-Type", "text/plain");
		}
		std::string _body = (std::string) _out["payload"];
		_return->body(_body);
		_return->header("Content-Length", std::to_string(_body.length()));
	}
	
	return _return;
}

zpt::json zpt::rest::http::deserialize(std::string _body) {
	zpt::json _return = zpt::json::object();
	std::string _name;
	std::string _collected;
	for (const auto& _c : _body) {
		switch(_c) {
			case '=' : {
				_name.assign(_collected.data());
				_collected.assign("");
				break;
			}
			case '&' : {
				zpt::url::decode(_collected);
				_return << _name << _collected;
				_name.assign("");
				_collected.assign("");
				break;
			}
			default : {
				_collected.push_back(_c);
			}
		}
	}
	zpt::url::decode(_collected);
	_return << _name << _collected;
	return _return;
}

extern "C" auto zpt_zmq() -> int {
	return 1;
}
