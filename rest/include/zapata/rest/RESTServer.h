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

#pragma once

#include <zapata/events.h>
#include <zapata/zmq.h>
#include <zapata/zmq/SocketStreams.h>
#include <zapata/rest/RESTEmitter.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	class RESTServer;
	class RESTClient;
	class RESTServerPtr;
	class RESTClientPtr;

	namespace rest {
		typedef zpt::RESTServerPtr server;
		typedef zpt::RESTClientPtr client;
	}

	class RESTServerPtr : public std::shared_ptr<zpt::RESTServer> {
	public:
		RESTServerPtr(std::string _name, zpt::json _options);
		RESTServerPtr(zpt::RESTServer * _ptr);
		virtual ~RESTServerPtr();

		static zpt::rest::server setup(zpt::json _options, std::string _name);
		static int launch(int argc, char* argv[]);
	};

	class RESTClientPtr : public std::shared_ptr<zpt::RESTClient> {
	public:
		RESTClientPtr(zpt::json _options);
		RESTClientPtr(zpt::RESTClient * _ptr);
		virtual ~RESTClientPtr();

		static zpt::rest::client launch(int argc, char* argv[]);
	};

	class RESTServer {
	public:
		RESTServer(std::string _name, zpt::json _options);
		virtual ~RESTServer();

		virtual void start();

		virtual std::string name();
		virtual zpt::json options();
		virtual zpt::poll poll();
		virtual zpt::ev::emitter emitter();

		virtual bool route_http(zpt::socketstream_ptr _cs);
		virtual bool route_mqtt(std::iostream& _cs);

	private:
		std::string __name;
		zpt::ev::emitter __emitter;
		zpt::poll __poll;
		zpt::json __options;
		std::vector< zpt::socket > __pub_sub;
		std::vector< zpt::socket > __router_dealer;
		std::vector< std::shared_ptr< std::thread > > __threads;

	};

	class RESTClient {
	public:
		RESTClient(zpt::json _options);
		virtual ~RESTClient();

		virtual void start();

		virtual zpt::json options();
		virtual zpt::poll poll();
		virtual zpt::ev::emitter emitter();

		virtual zpt::socket bind(short _type, std::string _connection);
		virtual zpt::socket bind(std::string _object_path);

	private:
		zpt::ev::emitter __emitter;
		zpt::poll __poll;
		zpt::json __options;
	};


	namespace conf {
		zpt::json init(int argc, char* argv[]);
		void setup(zpt::json _options);
		void dirs(std::string _dir, zpt::json _options);
		void dirs(zpt::json _options);
		void env(zpt::json _options);
	}

	namespace rest {
		zpt::json http2zmq(zpt::http::req _request);
		zpt::HTTPRep zmq2http(zpt::json _out);

		namespace http {
			zpt::json deserialize(std::string _body);
		}

		namespace authorization {
			std::string serialize(zpt::json _info);
			zpt::json deserialize(std::string _token);
			std::string extract(zpt::json _envelope);
		}

		namespace scopes {
			std::string serialize(zpt::json _info);
			zpt::json deserialize(std::string _scope);
			bool has_permission(std::string _scope, std::string _ns, std::string _permissions);
			bool has_permission(zpt::json _scope, std::string _ns, std::string _permissions);
		}
	}
}
