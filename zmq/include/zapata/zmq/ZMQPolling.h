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
#include <regex.h>
#include <string>
#include <map>
#include <memory>
#include <zmq.hpp>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zapata {

	class ZMQPoll;
	class ZMQ;

	typedef std::shared_ptr<zapata::ZMQ> ZMQPtr;
	typedef std::shared_ptr<zapata::ZMQPoll> ZMQPollPtr;

	class ZMQPoll {
		public:
			explicit ZMQPoll(zapata::JSONObj& _options, zapata::EventEmitterPtr _emiter);
			virtual ~ZMQPoll();

			virtual zapata::JSONObj& options();
			virtual zapata::EventEmitterPtr emitter();
			virtual zapata::ZMQPollPtr self() final;

			virtual void poll(zapata::ZMQPtr _socket);
			virtual void loop();
			virtual zapata::ZMQPtr borrow(short _type, std::string _connection);

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
			zapata::ZMQPollPtr __self;

			virtual void repoll() final;
	};

	class ZMQ {
		public:
			explicit ZMQ(zapata::JSONObj& _options);
			explicit ZMQ(std::string _obj_path, zapata::JSONObj& _options);
			explicit ZMQ(std::string _connection);
			virtual ~ZMQ();

			virtual zapata::JSONObj& options() final;
			virtual zmq::context_t& context() final;
			virtual std::string& connection() final;
			virtual zapata::ZMQPtr self() final;

			virtual zapata::JSONPtr recv() final;
			virtual void send(zapata::JSONPtr _envelope) final;
			
			virtual zmq::socket_t& socket() = 0;
			virtual zmq::socket_t& in() = 0;
			virtual zmq::socket_t& out() = 0;
			virtual short int type() = 0;
			virtual void listen(zapata::ZMQPollPtr _poll) = 0;

		private:
			zapata::JSONObj __options;
			zmq::context_t __context;
			std::string __connection;
			zapata::ZMQPtr __self;
	};

	class ZMQReq : public zapata::ZMQ {
		public:
			explicit ZMQReq(zapata::JSONObj& _options);
			explicit ZMQReq(std::string _obj_path, zapata::JSONObj& _options);
			explicit ZMQReq(std::string _connection);
			virtual ~ZMQReq();
			
			virtual zmq::socket_t& socket();
			virtual zmq::socket_t& in();
			virtual zmq::socket_t& out();
			virtual short int type();
			virtual void listen(zapata::ZMQPollPtr _poll);

		private:
			zmq::socket_t * __socket;
	};

	class ZMQRep : public zapata::ZMQ {
		public:
			explicit ZMQRep(zapata::JSONObj& _options);
			explicit ZMQRep(std::string _obj_path, zapata::JSONObj& _options);
			explicit ZMQRep(std::string _connection);
			virtual ~ZMQRep();
			
			virtual zmq::socket_t& socket();
			virtual zmq::socket_t& in();
			virtual zmq::socket_t& out();
			virtual short int type();
			virtual void listen(zapata::ZMQPollPtr _poll);

		private:
			zmq::socket_t * __socket;
	};

	class ZMQXPubXSub : public zapata::ZMQ {
		public:
			explicit ZMQXPubXSub(zapata::JSONObj& _options);
			explicit ZMQXPubXSub(std::string _obj_path, zapata::JSONObj& _options);
			explicit ZMQXPubXSub(std::string _connection);
			virtual ~ZMQXPubXSub();
			
			virtual zmq::socket_t& socket();
			virtual zmq::socket_t& in();
			virtual zmq::socket_t& out();
			virtual short int type();
			virtual void listen(zapata::ZMQPollPtr _poll);
			virtual void loop();

		private:
			zmq::socket_t * __socket_sub;
			zmq::socket_t * __socket_pub;
	};

	class ZMQPubSub : public zapata::ZMQ {
		public:
			explicit ZMQPubSub(zapata::JSONObj& _options);
			explicit ZMQPubSub(std::string _obj_path, zapata::JSONObj& _options);
			explicit ZMQPubSub(std::string _connection);
			virtual ~ZMQPubSub();
			
			virtual zmq::socket_t& socket();
			virtual zmq::socket_t& in();
			virtual zmq::socket_t& out();
			virtual short int type();
			virtual void listen(zapata::ZMQPollPtr _poll);

		private:
			zmq::socket_t * __socket_sub;
			zmq::socket_t * __socket_pub;
	};

}

