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
#include <mutex>
#include <zapata/json.h>
#include <zapata/zmq.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	class UPnP;
	class UPnPPtr;

	class UPnPPtr : public std::shared_ptr<zpt::UPnP> {
	public :
		UPnPPtr();
		virtual ~UPnPPtr();
	};

	namespace upnp {
		typedef zpt::UPnPPtr broker;
	}
	
	class UPnP {
	public:
		UPnP();
		virtual ~UPnP();
		
		virtual auto broadcast(zpt::json _services) ->  void;
		virtual auto search(zpt::json _services) ->  zpt::json;
		
	private:
		zpt::json __options;
		std::mutex __mtx;
		zpt::socketstream_ptr __socket;
		
	};
}
