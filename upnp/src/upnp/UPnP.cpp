/*
Copyright (c) 2017, Muzzley
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

zpt::UPnPPtr::UPnPPtr() : std::shared_ptr<zpt::UPnP>(nullptr) {}

zpt::UPnPPtr::UPnPPtr(zpt::json _options) : std::shared_ptr<zpt::UPnP>(new zpt::UPnP(_options)) {}

zpt::UPnPPtr::~UPnPPtr() {}

zpt::UPnP::UPnP(zpt::json _options)
    : zpt::Channel(
	  (_options["upnp"]["bind"]->is_string() ? _options["upnp"]["bind"]->str() : "udp://239.255.255.250:1991"),
	  _options),
      __underlying(), __send() {
	this->uri(this->connection());

	this->__underlying->open(this->uri()["domain"]->str().data(), int(this->uri()["port"]), false, IPPROTO_UDP);
	assertz(!this->__underlying->is_error(),
		std::string("upnp: error with socket: ") + this->__underlying->error_string(),
		503,
		this->__underlying->error_code());
	this->__send->open(this->uri()["domain"]->str().data(), int(this->uri()["port"]), false, IPPROTO_UDP);
	assertz(!this->__send->is_error(),
		std::string("upnp: error with socket: ") + this->__send->error_string(),
		503,
		this->__send->error_code());

	char _accept_lo = 1;
	setsockopt(this->__underlying->buffer().get_socket(),
		   IPPROTO_IP,
		   IP_MULTICAST_LOOP,
		   (char*)&_accept_lo,
		   sizeof _accept_lo);

	std::string _ip;
	if (_options["upnp"]["interface"]->is_string()) {
		_ip = zpt::net::getip(_options["upnp"]["interface"]->str());
		// if (_ip == "127.0.0.1") {
		// 	_ip = "0.0.0.0";
		// }
	} else {
		_ip = "0.0.0.0";
	}

	struct in_addr _local_interface;
	_local_interface.s_addr = inet_addr(_ip.data());
	setsockopt(this->__underlying->buffer().get_socket(),
		   IPPROTO_IP,
		   IP_MULTICAST_IF,
		   (char*)&_local_interface,
		   sizeof _local_interface);

	struct sockaddr_in _local_addr;
	memset((char*)&_local_addr, 0, sizeof _local_addr);
	_local_addr.sin_family = AF_INET;
	_local_addr.sin_port = htons(int(this->uri()["port"]));
	_local_addr.sin_addr.s_addr = INADDR_ANY;
	::bind(this->__underlying->buffer().get_socket(), (struct sockaddr*)&_local_addr, sizeof _local_addr);

	struct ip_mreq _group_addr;
	_group_addr.imr_multiaddr.s_addr = inet_addr(this->uri()["domain"]->str().data());
	_group_addr.imr_interface.s_addr = inet_addr(_ip.data());
	setsockopt(this->__underlying->buffer().get_socket(),
		   IPPROTO_IP,
		   IP_ADD_MEMBERSHIP,
		   (char*)&_group_addr,
		   sizeof _group_addr);

	struct timeval _tv;
	_tv.tv_sec = 0;
	_tv.tv_usec = 500000;
	setsockopt(
	    this->__underlying->buffer().get_socket(), SOL_SOCKET, SO_RCVTIMEO, (char*)&_tv, sizeof(struct timeval));
}

zpt::UPnP::~UPnP() {}

auto zpt::UPnP::listen() -> zpt::http::req {
	zpt::http::req _req;
	{
		std::lock_guard<std::mutex> _lock(this->in_mtx());
		(*this->__underlying) >> _req;
	}
	return _req;
}

auto zpt::UPnP::notify(std::string _search, std::string _location) -> void {
	this->send({"performative",
		    int(zpt::ev::Notify),
		    "headers",
		    {"Location", _location, "ST", _search, "MAN", "\"ssdp:discover\"", "MX", "3"}});
}

auto zpt::UPnP::search(std::string _search) -> void {
	this->send(
	    {"performative",
	     int(zpt::ev::Search),
	     "headers",
	     {"ST", (std::string("urn:schemas-upnp-org:service:") + _search), "MAN", "\"ssdp:discover\"", "MX", "3"}});
}

auto zpt::UPnP::id() -> std::string { return "__upnp_connection__"; }

auto zpt::UPnP::underlying() -> zpt::socketstream_ptr {
	return this->__underlying;
	;
}

auto zpt::UPnP::socket() -> zmq::socket_ptr { return zmq::socket_ptr(nullptr); }

auto zpt::UPnP::in() -> zmq::socket_ptr { return zmq::socket_ptr(nullptr); }

auto zpt::UPnP::out() -> zmq::socket_ptr { return zmq::socket_ptr(nullptr); }

auto zpt::UPnP::fd() -> int {
	return this->__underlying->buffer().get_socket();
	;
}

auto zpt::UPnP::close() -> void {
	this->__underlying->close();
	this->__send->close();
}

auto zpt::UPnP::available() -> bool { return true; }

auto zpt::UPnP::in_mtx() -> std::mutex& { return this->__mtx_underlying; }

auto zpt::UPnP::out_mtx() -> std::mutex& { return this->__mtx_send; }

auto zpt::UPnP::type() -> short int { return ZMQ_UPNP_RAW; }

auto zpt::UPnP::protocol() -> std::string { return "UPnP/1.1"; }

auto zpt::UPnP::send(zpt::json _envelope) -> zpt::json {
	zpt::ev::performative _performative = (zpt::ev::performative)((int)_envelope["performative"]);

	if (_performative != zpt::ev::Search && _performative != zpt::ev::Notify) {
		return zpt::undefined;
	}

	zpt::json _uri = zpt::uri::parse(_envelope["resource"]);
	_envelope << "resource"
		  << "*"
		  << "protocol" << this->protocol() << "params"
		  << ((_envelope["params"]->is_object() ? _envelope["params"] : zpt::undefined) + _uri["query"]);

	if (_envelope["payload"]["assertion_failed"]->ok() && _envelope["payload"]["code"]->ok()) {
		_envelope["headers"] << "X-Error" << _envelope["payload"]["code"];
	}
	try {
		{
			std::lock_guard<std::mutex> _lock(this->out_mtx());
			std::string _message;
			_message.assign(
			    std::string(zpt::rest::zmq2http_req(_envelope,
								this->__underlying->host() + std::string(":") +
								    std::to_string(this->__underlying->port()))));
			(*this->__send) << _message << flush;
		}

		ztrace(std::string("> ") + zpt::ev::to_str(_performative) + std::string(" ") +
		       _envelope["resource"]->str() +
		       (_performative == zpt::ev::Reply ? std::string(" ") + std::string(_envelope["status"])
							: std::string("")));
		zverbose(zpt::ev::pretty(_envelope));
	} catch (std::ios_base::failure& _e) {
	} catch (std::exception& _e) {
	}
	return zpt::undefined;
}

auto zpt::UPnP::recv() -> zpt::json {
	zpt::json _in;
	{
		std::lock_guard<std::mutex> _lock(this->in_mtx());
		zpt::http::req _request;
		(*this->__underlying) >> _request;
		_in = zpt::rest::http2zmq(_request);
		_in << "resource"
		    << "*"
		    << "protocol" << this->protocol();
	}
	ztrace(std::string("< ") + zpt::ev::to_str(zpt::ev::performative(int(_in["performative"]))) + std::string(" ") +
	       _in["resource"]->str() + (zpt::ev::performative(int(_in["performative"])) == zpt::ev::Reply
					     ? std::string(" ") + std::string(_in["status"])
					     : std::string("")));
	zverbose(zpt::ev::pretty(_in));
	return _in;
}

extern "C" auto zpt_upnp() -> int { return 1; }
