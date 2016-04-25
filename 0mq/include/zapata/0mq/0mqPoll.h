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

#include <zapata/base.h>
#include <zapata/json.h>
#include <zapata/events.h>
#include <zapata/0mq/0mqClient.h>
#include <regex.h>
#include <string>
#include <map>
#include <memory>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zapata {

	class ZMQPoll;

	typedef std::shared_ptr<zapata::ZMQPoll> ZMQPollPtr;

	class ZMQPoll {
		public:
			explicit ZMQPoll(zapata::JSONObj& _options, zapata::EventEmitterPtr _emiter);
			virtual ~ZMQPoll();

			virtual zapata::JSONObj& options();
			virtual zapata::EventEmitterPtr emitter();

			virtual void repoll();
			virtual void loop();

		private:
			zapata::JSONObj __options;
			std::map< std::string, zapata::ZMQPtr > __by_name;
			std::vector< zapata::ZMQPtr > __sockets;
			zmq::context_t __context;
			zmq::socket_t ** __internal;
			::pthread_t __id;
			zmq::pollitem_t * __poll;
			size_t __poll_size;
			pthread_mutex_t* __mtx;
			pthread_mutexattr_t* __attr;
			zapata::EventEmitterPtr __emitter;
	};

}

