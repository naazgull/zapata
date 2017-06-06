/*
Copyright (c) 2014, Muzzley
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
	
	class SMTP {
	public:
		SMTP();
		virtual ~SMTP();
		
		virtual auto credentials(std::string _user, std::string _passwd) -> void;
		
		virtual auto user() -> std::string;
		virtual auto passwd() -> std::string;
		virtual auto connected() -> bool;

		virtual auto connect(std::string _host, bool _tls = false, int _port = 1883, int _keep_alive = 25) ->  void;
		virtual auto reconnect() -> void;

		
	private:
		std::string __user;
		std::string __passwd;
		std::mutex __mtx;
		bool __connected;
	};
}
