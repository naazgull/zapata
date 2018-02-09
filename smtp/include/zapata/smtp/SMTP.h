/*
Copyright (c) 2017, Muzzley
*/

/**
 * This example uses the mosquitto C library (http://mosquitto.org).
 * In Ubuntu based systems is installable by executing:
 * $ sudo apt-get install libmosquitto0 libmosquitto0-dev
 *
 * Compile with '-lmosquitto'.
 */
#include <unistd.h>
#include <iostream>
#include <functional>
#include <memory>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <libetpan/libetpan.h>
#include <mutex>
#include <zapata/json.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

class SMTP;
class SMTPPtr;

class SMTPPtr : public std::shared_ptr<zpt::SMTP> {
      public:
	SMTPPtr();
	virtual ~SMTPPtr();
};

namespace smtp {
typedef zpt::SMTPPtr broker;
}

class SMTP {
      public:
	SMTP();
	virtual ~SMTP();

	virtual auto credentials(std::string _user, std::string _passwd) -> void;

	virtual auto user() -> std::string;
	virtual auto passwd() -> std::string;

	virtual auto connect(std::string _connection) -> void;
	virtual auto send(zpt::json _e_mail) -> void;

      private:
	std::string __connection;
	zpt::json __uri;
	std::string __user;
	std::string __passwd;
	std::string __host;
	uint __port;
	zpt::json __type;
	std::mutex __mtx;

	auto open() -> mailsmtp*;
	auto close(mailsmtp* _smtp) -> void;
	auto compose(zpt::json _e_mail) -> std::string;
};
}
