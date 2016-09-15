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
#include <czmq.h>
#include <mutex>
#include <zapata/zmq/SocketStreams.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

#define ZMQ_PUB_SUB -1
#define ZMQ_XPUB_XSUB -2
#define ZMQ_ROUTER_DEALER -3
#define ZMQ_ASSYNC_REQ -4

namespace zpt {

	short str2type(std::string _type);
	
	class ZMQPoll;
	class ZMQ;

	typedef std::shared_ptr<zpt::ZMQ> ZMQPtr;

	class ZMQPollPtr : public std::shared_ptr<zpt::ZMQPoll> {
	public:
		ZMQPollPtr(zpt::json _options, zpt::ev::emitter _emiter);
		ZMQPollPtr(zpt::json _options);
		ZMQPollPtr(zpt::ZMQPoll * _ptr);
		virtual ~ZMQPollPtr();
	};
	
	typedef zpt::ZMQPtr socket;
	typedef zpt::ZMQPollPtr poll;

	class ZMQPoll {
	public:
		ZMQPoll(zpt::json _options, zpt::ev::emitter _emiter);
		ZMQPoll(zpt::json _options);
		virtual ~ZMQPoll();
		
		virtual zpt::json options();
		virtual zpt::ev::emitter emitter();
		virtual zpt::ZMQPollPtr self();
		
		virtual void poll(zpt::ZMQPtr _socket);
		virtual void unpoll(zpt::ZMQ& _socket);
		virtual void loop();
		virtual zpt::socket bind(short _type, std::string _connection);
		
		virtual void unbind();
			
	private:
		zpt::json __options;
		std::map< std::string, zpt::socket > __by_name;
		std::map< zsock_t*, zpt::socket > __by_socket;
		::pthread_t __id;
		zpoller_t* __poll;
		zsock_t* __interrupt;
		zsock_t* __signal;
		std::mutex __mtx;
		zpt::ev::emitter __emitter;
		zpt::ZMQPollPtr __self;
		
		virtual void unpoll_no_mutex(zpt::socket _socket);
		virtual void signal_poller();

	};

	class ZMQ {
	public:
		ZMQ(zpt::json _options, zpt::ev::emitter _emitter);
		ZMQ(std::string _obj_path, zpt::json _options, zpt::ev::emitter _emitter);
		ZMQ(std::string _connection, zpt::ev::emitter _emitter);
		virtual ~ZMQ();
		
		virtual std::string id();
		virtual zpt::json options();
		virtual std::string& connection();
		virtual zpt::ZMQPtr self();
		virtual zpt::ev::emitter emitter();
		
		virtual zpt::json recv();
		virtual zpt::json send(zpt::ev::performative _performative, std::string _resource, zpt::json _payload);
		virtual zpt::json send(zpt::json _envelope);
		
		virtual zsock_t* socket() = 0;
		virtual zsock_t* in() = 0;
		virtual zsock_t* out() = 0;
		virtual short int type() = 0;
		virtual bool once() = 0;
		virtual void listen(zpt::poll _poll) = 0;
		
		virtual void unbind();
		
	private:
		zpt::json __options;
		std::string __connection;
		zpt::ZMQPtr __self;
		zpt::ev::emitter __emitter;
		std::string __id;

	protected:
		std::mutex __mtx;		
	};

	class ZMQReq : public zpt::ZMQ {
	public:
		ZMQReq(zpt::json _options, zpt::ev::emitter _emitter);
		ZMQReq(std::string _obj_path, zpt::json _options, zpt::ev::emitter _emitter);
		ZMQReq(std::string _connection, zpt::ev::emitter _emitter);
		virtual ~ZMQReq();
		
		virtual zpt::json send(zpt::json _envelope);
		
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		
	private:
		zsock_t* __socket;
	};
	
	class ZMQRep : public zpt::ZMQ {
	public:
		ZMQRep(zpt::json _options, zpt::ev::emitter _emitter);
		ZMQRep(std::string _obj_path, zpt::json _options, zpt::ev::emitter _emitter);
		ZMQRep(std::string _connection, zpt::ev::emitter _emitter);
		virtual ~ZMQRep();
		
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		
	private:
		zsock_t* __socket;
	};
	
	class ZMQXPubXSub : public zpt::ZMQ {
	public:
		ZMQXPubXSub(zpt::json _options, zpt::ev::emitter _emitter);
		ZMQXPubXSub(std::string _obj_path, zpt::json _options, zpt::ev::emitter _emitter);
		ZMQXPubXSub(std::string _connection, zpt::ev::emitter _emitter);
		virtual ~ZMQXPubXSub();
		
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		
	private:
		zactor_t* __socket;
	};
	
	class ZMQPubSub : public zpt::ZMQ {
	public:
		ZMQPubSub(zpt::json _options, zpt::ev::emitter _emitter);
		ZMQPubSub(std::string _obj_path, zpt::json _options, zpt::ev::emitter _emitter);
		ZMQPubSub(std::string _connection, zpt::ev::emitter _emitter);
		virtual ~ZMQPubSub();
		
		virtual zpt::json recv();
		virtual zpt::json send(zpt::json _envelope);
		
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		
	private:
		zsock_t* __socket_sub;
		zsock_t* __socket_pub;
	};
	
	class ZMQPub : public zpt::ZMQ {
	public:
		ZMQPub(zpt::json _options, zpt::ev::emitter _emitter);
		ZMQPub(std::string _obj_path, zpt::json _options, zpt::ev::emitter _emitter);
		ZMQPub(std::string _connection, zpt::ev::emitter _emitter);
		virtual ~ZMQPub();
		
		virtual zpt::json recv();
		
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		
	private:
		zsock_t* __socket;
	};
	
	class ZMQSub : public zpt::ZMQ {
	public:
		ZMQSub(zpt::json _options, zpt::ev::emitter _emitter);
		ZMQSub(std::string _obj_path, zpt::json _options, zpt::ev::emitter _emitter);
		ZMQSub(std::string _connection, zpt::ev::emitter _emitter);
		virtual ~ZMQSub();
		
		virtual zpt::json send(zpt::json _envelope);
		
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		
	private:
		zsock_t* __socket;
	};
	
	class ZMQPush : public zpt::ZMQ {
	public:
		ZMQPush(zpt::json _options, zpt::ev::emitter _emitter);
		ZMQPush(std::string _obj_path, zpt::json _options, zpt::ev::emitter _emitter);
		ZMQPush(std::string _connection, zpt::ev::emitter _emitter);
		virtual ~ZMQPush();
		
		virtual zpt::json recv();
		
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		
	private:
		zsock_t* __socket;
	};

	class ZMQPull : public zpt::ZMQ {
	public:
		ZMQPull(zpt::json _options, zpt::ev::emitter _emitter);
		ZMQPull(std::string _obj_path, zpt::json _options, zpt::ev::emitter _emitter);
		ZMQPull(std::string _connection, zpt::ev::emitter _emitter);
		virtual ~ZMQPull();
		
		virtual zpt::json send(zpt::json _envelope);
		
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		
	private:
		zsock_t* __socket;
	};

	class ZMQRouterDealer : public zpt::ZMQ {
	public:
		ZMQRouterDealer(zpt::json _options, zpt::ev::emitter _emitter);
		ZMQRouterDealer(std::string _obj_path, zpt::json _options, zpt::ev::emitter _emitter);
		ZMQRouterDealer(std::string _connection, zpt::ev::emitter _emitter);
		virtual ~ZMQRouterDealer();
		
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		
	private:
		zactor_t* __socket;
	};

}

