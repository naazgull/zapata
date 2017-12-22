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
		UPnPPtr(zpt::json _options);
		virtual ~UPnPPtr();
	};

	namespace upnp {
		typedef zpt::UPnPPtr broker;
	}
	
	class UPnP : public zpt::Channel {
	public:
		UPnP(zpt::json _options);
		virtual ~UPnP();
		
		virtual auto notify(std::string _search, std::string _location) -> void;
		virtual auto search(std::string _search) ->  void;
		virtual auto listen() ->  zpt::http::req;

		virtual auto recv() -> zpt::json;
		virtual auto send(zpt::json _envelope) -> zpt::json;

		virtual auto id() -> std::string;
		virtual auto underlying() -> zpt::socketstream_ptr;
		virtual auto socket() -> zmq::socket_ptr;
		virtual auto in() -> zmq::socket_ptr;
		virtual auto out() -> zmq::socket_ptr;
		virtual auto fd() -> int;
		virtual auto in_mtx() -> std::mutex&;
		virtual auto out_mtx() -> std::mutex&;
		virtual auto type() -> short int;
		virtual auto protocol() -> std::string;
		virtual auto close() -> void;
		virtual auto available() -> bool;		
		
	private:
		std::mutex __mtx_underlying;
		std::mutex __mtx_send;
		zpt::socketstream_ptr __underlying;
		zpt::socketstream_ptr __send;
		
	};
}
