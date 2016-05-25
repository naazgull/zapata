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
#include <zapata/rest/SocketStreams.h>
#include <zapata/rest/RESTEmitter.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	class RESTServer;
	class RESTClient;

	class RESTServerPtr : public std::shared_ptr<zpt::RESTServer> {
		public:
			RESTServerPtr(zpt::JSONPtr _options);
			RESTServerPtr(zpt::RESTServer * _ptr);
			virtual ~RESTServerPtr();
	};

	class RESTClientPtr : public std::shared_ptr<zpt::RESTClient> {
		public:
			RESTClientPtr(zpt::JSONPtr _options);
			RESTClientPtr(zpt::RESTClient * _ptr);
			virtual ~RESTClientPtr();
	};

	class RESTServer {
	public:
		RESTServer(zpt::JSONPtr _options);
		virtual ~RESTServer();

		virtual void start();

		virtual zpt::JSONPtr options();
		virtual zpt::ZMQPollPtr poll();
		virtual zpt::EventEmitterPtr emitter();

		virtual bool route_http(zpt::socketstream_ptr _cs, std::map<std::string, zpt::socketstream_ptr>& _pending);
		virtual bool route_mqtt(std::iostream& _cs);

		
	private:
		zpt::EventEmitterPtr __emitter;
		zpt::ZMQPollPtr __poll;
		zpt::JSONPtr __options;
		std::string __type;
		zpt::ZMQPtr __assync;
		std::string __assync_http_cid;

		zpt::JSONPtr http2zmq(zpt::HTTPReq _request);
		zpt::HTTPRep zmq2http(zpt::JSONPtr _out);
	};

	class RESTClient {
	public:
		RESTClient(zpt::JSONPtr _options);
		virtual ~RESTClient();

		virtual void start();

		virtual zpt::JSONPtr options();
		virtual zpt::ZMQPollPtr poll();
		virtual zpt::EventEmitterPtr emitter();

		virtual zpt::ZMQPtr bind(short _type, std::string _connection);
		virtual zpt::ZMQPtr bind(std::string _object_path);

	private:
		zpt::EventEmitterPtr __emitter;
		zpt::ZMQPollPtr __poll;
		zpt::JSONPtr __options;
	};


	namespace conf {
		void dirs(std::string _dir, zpt::JSONPtr _options);
		void dirs(zpt::JSONPtr _options);
		void env(zpt::JSONPtr _options);
	}

	namespace rest {
		zpt::JSONPtr not_found(std::string _resource);
	}
}
