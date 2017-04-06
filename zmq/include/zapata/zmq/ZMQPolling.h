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

#define ZPT_SELF_CERTIFICATE 0
#define ZPT_PEER_CERTIFICATE 1

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

	namespace assync {
		typedef std::function< std::string (zpt::json _envelope) > reply_fn;

		class AssyncReplyException : public std::exception {
		public:
			AssyncReplyException(zpt::socket _relay);
			virtual ~AssyncReplyException();

			virtual std::string what();
			virtual zpt::socket relay();

		private:
			zpt::socket __relay;
		};
	}
	
	class ZMQPoll {
	public:
		ZMQPoll(zpt::json _options, zpt::ev::emitter _emiter = nullptr);
		virtual ~ZMQPoll();
		
		virtual auto options() -> zpt::json;
		virtual auto emitter() -> zpt::ev::emitter;
		virtual auto self() const -> zpt::ZMQPollPtr;

		auto get_by_name(std::string _name) -> zpt::socket;
		auto get_by_uuid(std::string _uuid) -> zpt::socket;
		auto get_by_zsock(zsock_t* _sock) -> zpt::socket;
		auto add_by_name(zpt::socket _socket) -> void;
		auto add_by_uuid(zpt::socket _socket) -> void;
		auto add_by_zsock(zpt::socket _socket) -> void;
		auto remove_by_name(zpt::socket _socket) -> void;
		auto remove_by_uuid(zpt::socket _socket) -> void;
		auto remove_by_zsock(zpt::socket _socket) -> void;

		
		virtual auto poll(zpt::socket _socket) -> void;
		virtual auto unpoll(zpt::socket _socket, bool _signal = true) -> void;
		virtual auto loop() -> void;
		virtual auto bind(short _type, std::string _connection) -> zpt::socket;
		
		virtual auto unbind() -> void;
			
	private:
		zpt::json __options;
		std::map< std::string, zpt::socket > __by_name;
		std::map< std::string, zpt::socket > __by_uuid;
		std::map< int, zpt::socket > __by_socket;
		::pthread_t __id;
		zpoller_t* __poll;
		zsock_t* __sync[4];
		std::mutex __mtx[2];
		zpt::ZMQPollPtr __self;
		zpt::ev::emitter __emitter;
		
		auto signal(std::string _message, int _idx_send) -> void;
		auto notify(std::string _message, int _idx_send) -> void;
		auto wait(int _idx_send) -> void;

	};

	class ZMQ {
	public:
		ZMQ(std::string _connection, zpt::json _options);
		virtual ~ZMQ();
		
		virtual std::string id();
		virtual zpt::json options();
		virtual auto connection() -> std::string;
		virtual auto connection(std::string _connection) -> void;
		virtual auto uri(size_t _idx = 0) -> zpt::json;
		virtual auto uri(std::string _connection) -> void;
		virtual auto detach() -> void;
		virtual zactor_t* auth(std::string _client_cert_dir = "");
		virtual zcert_t* certificate(int _which = ZPT_SELF_CERTIFICATE);
		virtual void certificate(std::string cert_file, int _which = ZPT_SELF_CERTIFICATE);
		
		virtual zpt::json recv();
		static zpt::json recv(zsock_t* _socket);
		virtual zpt::json send(zpt::ev::performative _performative, std::string _resource, zpt::json _payload);
		virtual zpt::json send(zpt::json _envelope);
		static zpt::json send(zpt::ev::performative _performative, std::string _resource, zpt::json _payload, zsock_t* _socket);
		static zpt::json send(zpt::json _envelope, zsock_t* _socket);
		
		virtual void relay_for(zpt::socketstream_ptr _socket, zpt::assync::reply_fn _transform);
		virtual void relay_for(zpt::socket _socket);

		virtual auto self() const -> zpt::socket = 0;
		virtual auto socket() -> zsock_t* = 0;
		virtual auto in() -> zsock_t* = 0;
		virtual auto out() -> zsock_t* = 0;
		virtual auto in_mtx() -> std::mutex& = 0;
		virtual auto out_mtx() -> std::mutex& = 0;
		virtual auto type() -> short int = 0;
		virtual auto once() -> bool = 0;
		virtual auto listen(zpt::poll _poll) -> void = 0;
		virtual auto unbind() -> void = 0;
		
		virtual auto unlisten() -> void;
		
	private:
		zpt::json __options;
		std::string __connection;
		std::string __id;
		zcert_t* __self_cert;
		zcert_t* __peer_cert;
		zpt::json __uri;

		static zactor_t* __auth;

	protected:
		std::mutex __mtx;
		zpt::poll __poll;
		
	};

	class ZMQReq : public zpt::ZMQ {
	public:
		ZMQReq(std::string _connection, zpt::json _options);
		virtual ~ZMQReq();
		
		virtual zpt::json send(zpt::json _envelope);
		
		virtual auto self() const -> zpt::socket;
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual auto in_mtx() -> std::mutex&;
		virtual auto out_mtx() -> std::mutex&;
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		virtual auto unbind() -> void;

		
	private:
		zsock_t* __socket;
		zpt::socket __self;
	};
	
	class ZMQRep : public zpt::ZMQ {
	public:
		ZMQRep(std::string _connection, zpt::json _options);
		virtual ~ZMQRep();
		
		virtual auto self() const -> zpt::socket;
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual auto in_mtx() -> std::mutex&;
		virtual auto out_mtx() -> std::mutex&;
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		virtual auto unbind() -> void;

		
	private:
		zsock_t* __socket;
		zpt::socket __self;
	};
	
	class ZMQXPubXSub : public zpt::ZMQ {
	public:
		ZMQXPubXSub(std::string _connection, zpt::json _options);
		virtual ~ZMQXPubXSub();
		
		virtual auto self() const -> zpt::socket;
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual auto in_mtx() -> std::mutex&;
		virtual auto out_mtx() -> std::mutex&;
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		virtual auto unbind() -> void;

		
	private:
		zactor_t* __socket;
		zpt::socket __self;
	};
	
	class ZMQPubSub : public zpt::ZMQ {
	public:
		ZMQPubSub(std::string _connection, zpt::json _options);
		virtual ~ZMQPubSub();
		
		virtual auto self() const -> zpt::socket;
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual auto in_mtx() -> std::mutex&;
		virtual auto out_mtx() -> std::mutex&;
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		virtual auto unbind() -> void;

		virtual void subscribe(std::string _prefix);
		
	private:
		zsock_t* __socket_sub;
		zsock_t* __socket_pub;
		zpt::socket __self;
		std::mutex __out_mtx;
	};
	
	class ZMQPub : public zpt::ZMQ {
	public:
		ZMQPub(std::string _connection, zpt::json _options);
		virtual ~ZMQPub();
		
		virtual zpt::json recv();
		
		virtual auto self() const -> zpt::socket;
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual auto in_mtx() -> std::mutex&;
		virtual auto out_mtx() -> std::mutex&;
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		virtual auto unbind() -> void;

		
	private:
		zsock_t* __socket;
		zpt::socket __self;
	};
	
	class ZMQSub : public zpt::ZMQ {
	public:
		ZMQSub(std::string _connection, zpt::json _options);
		virtual ~ZMQSub();
		
		virtual zpt::json send(zpt::json _envelope);
		
		virtual auto self() const -> zpt::socket;
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual auto in_mtx() -> std::mutex&;
		virtual auto out_mtx() -> std::mutex&;
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		virtual auto unbind() -> void;

		virtual void subscribe(std::string _prefix);
		
	private:
		zsock_t* __socket;
		zpt::socket __self;
	};
	
	class ZMQPush : public zpt::ZMQ {
	public:
		ZMQPush(std::string _connection, zpt::json _options);
		virtual ~ZMQPush();
		
		virtual zpt::json recv();
		
		virtual auto self() const -> zpt::socket;
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual auto in_mtx() -> std::mutex&;
		virtual auto out_mtx() -> std::mutex&;
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		virtual auto unbind() -> void;

		
	private:
		zsock_t* __socket;
		zpt::socket __self;
	};

	class ZMQPull : public zpt::ZMQ {
	public:
		ZMQPull(std::string _connection, zpt::json _options);
		virtual ~ZMQPull();
		
		virtual zpt::json send(zpt::json _envelope);
		
		virtual auto self() const -> zpt::socket;
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual auto in_mtx() -> std::mutex&;
		virtual auto out_mtx() -> std::mutex&;
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		virtual auto unbind() -> void;

		
	private:
		zsock_t* __socket;
		zpt::socket __self;
	};

	class ZMQRouterDealer : public zpt::ZMQ {
	public:
		ZMQRouterDealer(std::string _connection, zpt::json _options);
		virtual ~ZMQRouterDealer();
		
		virtual auto self() const -> zpt::socket;
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual auto in_mtx() -> std::mutex&;
		virtual auto out_mtx() -> std::mutex&;
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		virtual auto unbind() -> void;

		
	private:
		zactor_t* __socket;
		zpt::socket __self;
	};

	class ZMQAssyncReq : public zpt::ZMQ {
	public:
		ZMQAssyncReq(std::string _connection, zpt::json _options);
		virtual ~ZMQAssyncReq();
		
		virtual zpt::json recv();
		virtual zpt::json send(zpt::json _envelope);

		virtual void relay_for(zpt::socketstream_ptr _socket, zpt::assync::reply_fn _transform);
		virtual void relay_for(zpt::socket _socket);

		virtual auto self() const -> zpt::socket;
		virtual zsock_t* socket();
		virtual zsock_t* in();
		virtual zsock_t* out();
		virtual auto in_mtx() -> std::mutex&;
		virtual auto out_mtx() -> std::mutex&;
		virtual short int type();
		virtual bool once();
		virtual void listen(zpt::poll _poll);
		virtual auto unbind() -> void;

		
	private:
		zsock_t* __socket;
		zpt::socketstream_ptr __raw_socket;
		zpt::socket __zmq_socket;
		short __type;
		zpt::assync::reply_fn __raw_transformer;
		zpt::socket __self;
	};

	namespace net {
		auto getip() -> std::string;
	}
}

