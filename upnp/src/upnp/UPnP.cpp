/*
Copyright (c) 2014, Muzzley
*/

#include <zapata/upnp/UPnP.h>
#include <ossp/uuid++.hh>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#ifndef CRLF
#define CRLF "\r\n"
#endif

zpt::UPnPPtr::UPnPPtr(zpt::json _options) : std::shared_ptr<zpt::UPnP>(new zpt::UPnP(_options)) {
}

zpt::UPnPPtr::~UPnPPtr() {
}

zpt::UPnP::UPnP(zpt::json _options) : zpt::ZMQ("udp://239.255.255.250:1900", _options), __underlying(new zpt::socketstream()), __send(new zpt::socketstream()) {
	this->__underlying->open("239.255.255.250", 1900, false, IPPROTO_UDP);
	this->__send->open("239.255.255.250", 1900, false, IPPROTO_UDP);
	
	char _no_lo = 0;
	setsockopt(this->__underlying->buffer().get_socket(), IPPROTO_IP, IP_MULTICAST_LOOP, (char *) &_no_lo, sizeof(_no_lo));

	struct in_addr _local_interface;
	_local_interface.s_addr = inet_addr("0.0.0.0");
	setsockopt(this->__underlying->buffer().get_socket(), IPPROTO_IP, IP_MULTICAST_IF, (char *) &_local_interface, sizeof(_local_interface));

	struct sockaddr_in _local_addr;
	memset((char *) &_local_addr, 0, sizeof(_local_addr));
	_local_addr.sin_family = AF_INET;
	_local_addr.sin_port = htons(1900);
	_local_addr.sin_addr.s_addr = INADDR_ANY;

	::bind(this->__underlying->buffer().get_socket(), (struct sockaddr*) &_local_addr, sizeof(_local_addr));

	struct ip_mreq _group_addr;
	_group_addr.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
	_group_addr.imr_interface.s_addr = inet_addr("0.0.0.0");

	setsockopt(this->__underlying->buffer().get_socket(), IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &_group_addr, sizeof(_group_addr));
}

zpt::UPnP::~UPnP() {
}

auto zpt::UPnP::listen() ->  zpt::http::req {
	zpt::http::req _req;
	{ std::lock_guard< std::mutex > _lock(this->in_mtx());
		(*this->__underlying) >> _req; }
	return _req;
}

auto zpt::UPnP::notify(std::string _search, std::string _location) -> void {
	zpt::http::req _req;
	_req->method(zpt::ev::Notify);
	_req->url("*");
	_req->header("Host", "239.255.255.250:1900");
	_req->header("Location", _location);
	_req->header("ST", _search);
	_req->header("MAN", "\"ssdp:discover\"");
	_req->header("MX", "3");	
	{ std::lock_guard< std::mutex > _lock(this->out_mtx());
		(*this->__send) << _req << flush; }
}

auto zpt::UPnP::search(std::string _search) -> void {
	zpt::http::req _req;
	_req->method(zpt::ev::Search);
	_req->url("*");
	_req->header("Host", "239.255.255.250:1900");
	_req->header("ST", _search);
	_req->header("MAN", "\"ssdp:discover\"");
	_req->header("MX", "3");	
	{ std::lock_guard< std::mutex > _lock(this->out_mtx());
		(*this->__send) << _req << flush; }
}

auto zpt::UPnP::id() -> std::string {
	return "__upnp_connection__";
}

auto zpt::UPnP::underlying() -> zpt::socketstream_ptr {
	return this->__underlying;;
}

auto zpt::UPnP::socket() -> zmq::socket_ptr {
	return zmq::socket_ptr(nullptr);
}

auto zpt::UPnP::in() -> zmq::socket_ptr {
	return zmq::socket_ptr(nullptr);
}

auto zpt::UPnP::out() -> zmq::socket_ptr {
	return zmq::socket_ptr(nullptr);
}

auto zpt::UPnP::fd() -> int {
	return this->__underlying->buffer().get_socket();;
}

auto zpt::UPnP::close() -> void {
	this->__underlying->close();
	this->__send->close();
}

auto zpt::UPnP::available() -> bool {
	char _c;
	int _read = ::recv(this->fd(), &_c, 1, MSG_PEEK);
	return _read > 0;
}

auto zpt::UPnP::in_mtx() -> std::mutex& {
	return this->__mtx_underlying;
}

auto zpt::UPnP::out_mtx() -> std::mutex& {
	return this->__mtx_send;
}

auto zpt::UPnP::type() -> short int {
	return ZMQ_UPNP_RAW;
}

auto zpt::UPnP::protocol() -> std::string {
	return "UPnP/1.1";
}

auto zpt::UPnP::send(zpt::json _envelope) -> zpt::json {
	zpt::ev::performative _performative = (zpt::ev::performative) ((int) _envelope["performative"]);
	
	if (_performative != zpt::ev::Search && _performative != zpt::ev::Notify) {
		return zpt::undefined;
	}
	
	zpt::json _uri = zpt::uri::parse(_envelope["resource"]);
	_envelope <<
	"resource" << "*" <<
	"protocol" << this->protocol() << 
	"params" << ((_envelope["params"]->is_object() ? _envelope["params"] : zpt::undefined) + _uri["query"]);
	
	if (_envelope["payload"]["assertion_failed"]->ok() && _envelope["payload"]["code"]->ok()) {
		_envelope["headers"] << "X-Error" << _envelope["payload"]["code"];
	}
	try {
		{ std::lock_guard< std::mutex > _lock(this->out_mtx());
			std::string _message;
			_message.assign(std::string(zpt::rest::zmq2http_req(_envelope, this->__underlying->host() + std::string(":") + std::to_string(this->__underlying->port()))));
			(*this->__send) << _message << flush; }
		
		ztrace(std::string("> ") + zpt::ev::to_str(_performative) + std::string(" ") + _envelope["resource"]->str() + (_performative == zpt::ev::Reply ? std::string(" ") + std::string(_envelope["status"]) : std::string("")));
		zverbose(zpt::ev::pretty(_envelope));
	}
	catch (std::ios_base::failure& _e) {}
	catch (std::exception& _e) {}
	return zpt::undefined;
}

auto zpt::UPnP::recv() -> zpt::json {
	zpt::json _in;
	{ std::lock_guard< std::mutex > _lock(this->in_mtx());
		zpt::http::req _request;
		(*this->__underlying) >> _request;
		_in = zpt::rest::http2zmq(_request);
		_in << "protocol" << this->protocol(); }
	ztrace(std::string("< ") + zpt::ev::to_str(zpt::ev::performative(int(_in["performative"]))) + std::string(" ") + _in["resource"]->str() + (zpt::ev::performative(int(_in["performative"])) == zpt::ev::Reply ? std::string(" ") + std::string(_in["status"]) : std::string("")));
	zverbose(zpt::ev::pretty(_in));
	return _in;
}

extern "C" auto zpt_upnp() -> int {
	return 1;
}
