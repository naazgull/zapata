/*
Copyright (c) 2014, Muzzley
*/

#include <zapata/smtp/SMTP.h>
#include <ossp/uuid++.hh>

zpt::SMTP::SMTP() : __connected(false) {
}

zpt::SMTP::~SMTP() {
}

auto zpt::SMTP::credentials(std::string _user, std::string _passwd) -> void {
	this->__user = _user;
	this->__passwd = _passwd;
}

auto zpt::SMTP::user() -> std::string {
	return this->__user;
}

auto zpt::SMTP::passwd() -> std::string {
	return this->__passwd;
}

auto zpt::SMTP::connected() -> bool {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	return this->__connected;
}

auto zpt::SMTP::connect(std::string _host, bool _tls, int _port, int _keep_alive) -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx);
}

auto zpt::SMTP::reconnect() -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx);
}

extern "C" auto zpt_smtp() -> int {
	return 1;
}
