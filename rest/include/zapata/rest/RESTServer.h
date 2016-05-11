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

namespace zapata {

	class RESTServer;
	class RESTClient;

	class RESTServerPtr : public std::shared_ptr<zapata::RESTServer> {
		public:
			RESTServerPtr(zapata::JSONPtr _options);
			RESTServerPtr(zapata::RESTServer * _ptr);
			virtual ~RESTServerPtr();
	};

	class RESTClientPtr : public std::shared_ptr<zapata::RESTClient> {
		public:
			RESTClientPtr(zapata::JSONPtr _options);
			RESTClientPtr(zapata::RESTClient * _ptr);
			virtual ~RESTClientPtr();
	};

	class RESTServer {
	public:
		RESTServer(zapata::JSONPtr _options);
		virtual ~RESTServer();

		virtual void start();

		virtual zapata::JSONPtr options();
		virtual zapata::ZMQPollPtr poll();
		virtual zapata::EventEmitterPtr emitter();

	private:
		zapata::EventEmitterPtr __emitter;
		zapata::ZMQPollPtr __poll;
		zapata::JSONPtr __options;
		std::string __type;
		zapata::ZMQPtr __assync;
	};

	class RESTClient {
	public:
		RESTClient(zapata::JSONPtr _options);
		virtual ~RESTClient();

		virtual void start();

		virtual zapata::JSONPtr options();
		virtual zapata::ZMQPollPtr poll();
		virtual zapata::EventEmitterPtr emitter();

		virtual zapata::ZMQPtr borrow(short _type, std::string _connection);

	private:
		zapata::EventEmitterPtr __emitter;
		zapata::ZMQPollPtr __poll;
		zapata::JSONPtr __options;
	};


	void dirs(std::string _dir, zapata::JSONPtr& _options);
	void env(zapata::JSONPtr _options);

}
