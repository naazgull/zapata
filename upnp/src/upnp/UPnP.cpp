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

zpt::UPnPPtr::UPnPPtr() : std::shared_ptr<zpt::UPnP>(new zpt::UPnP()) {
}

zpt::UPnPPtr::~UPnPPtr() {
}

zpt::UPnP::UPnP() : __socket(new zpt::socketstream()) {
	this->__socket->open("239.255.255.250", 1900, false, IPPROTO_UDP);

	char _no_lo = 0;
	setsockopt(this->__socket->buffer().get_socket(), IPPROTO_IP, IP_MULTICAST_LOOP, (char *) &_no_lo, sizeof(_no_lo));

	struct in_addr _local_interface;
	_local_interface.s_addr = inet_addr("0.0.0.0");
	setsockopt(this->__socket->buffer().get_socket(), IPPROTO_IP, IP_MULTICAST_IF, (char *) &_local_interface, sizeof(_local_interface));

	struct sockaddr_in _local_addr;
	memset((char *) &_local_addr, 0, sizeof(_local_addr));
	_local_addr.sin_family = AF_INET;
	_local_addr.sin_port = htons(1900);
	_local_addr.sin_addr.s_addr = INADDR_ANY;

	::bind(this->__socket->buffer().get_socket(), (struct sockaddr*) &_local_addr, sizeof(_local_addr));

	struct ip_mreq _group_addr;
	_group_addr.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
	_group_addr.imr_interface.s_addr = inet_addr("0.0.0.0");

	setsockopt(this->__socket->buffer().get_socket(), IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &_group_addr, sizeof(_group_addr));
}

zpt::UPnP::~UPnP() {
}

auto zpt::UPnP::broadcast(zpt::json _services) ->  void {
}

auto zpt::UPnP::search(zpt::json _services) ->  zpt::json {
	return zpt::undefined;
}

extern "C" auto zpt_upnp() -> int {
	return 1;
}
