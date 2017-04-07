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

#include <zapata/http.h>
#include <zapata/json.h>
#include <zapata/events.h>
#include <zapata/zmq.h>
#include <string>
#include <map>
#include <memory>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	class RESTMutationEmitter;
	class RESTMutationServer;
	class RESTMutationServerPtr;

	namespace mutation {
		typedef zpt::RESTMutationServerPtr server;

		namespace zmq {
			typedef zpt::RESTMutationEmitter emitter;

			extern zpt::mutation::zmq::emitter* __emitter;
		}
	}

	class RESTMutationServerPtr : public std::shared_ptr<zpt::RESTMutationServer> {
	public:
		RESTMutationServerPtr(zpt::json _options);
		RESTMutationServerPtr(zpt::RESTMutationServer* _ptr);
		virtual ~RESTMutationServerPtr();

		static zpt::mutation::server setup(zpt::json _options);
		static int launch(int argc, char* argv[], int _semaphore);
	};

	class RESTMutationServer {
	public:
		RESTMutationServer(zpt::json _options);
		virtual ~RESTMutationServer();

		virtual auto start() -> void;
		virtual auto options() -> zpt::json;
		
	private:
		zpt::json __options;
		zpt::mutation::server __self;
		zpt::ZMQ* __server;
		zpt::ZMQ* __client;
	};

	class RESTMutationEmitter : public zpt::MutationEmitter {
	public:
		RESTMutationEmitter(zpt::json _options);
		virtual ~RESTMutationEmitter();

		virtual auto version() -> std::string;

		virtual auto loop() -> void;

		virtual auto on(zpt::mutation::operation _operation, std::string _data_class_ns,  zpt::mutation::Handler _handler, zpt::json _opts = zpt::undefined) -> std::string;
		virtual auto on(std::string _data_class_ns,  std::map< zpt::mutation::operation, zpt::mutation::Handler > _handlers, zpt::json _opts = zpt::undefined) -> std::string;
		virtual auto on(zpt::mutation::listener _listener, zpt::json _opts = zpt::undefined) -> std::string;
		virtual auto off(zpt::mutation::operation _operation, std::string _callback_id) -> void;
		virtual auto off(std::string _callback_id) -> void;
		
		virtual auto trigger(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record, zpt::json _opts = zpt::undefined) -> zpt::json;
		virtual auto route(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record, zpt::json _opts = zpt::undefined) -> zpt::json;

		static auto instance() -> zpt::mutation::emitter;
		
	private:
		zpt::mutation::HandlerStack __resources;
		zpt::ZMQ* __socket;
		std::mutex __mtx;
		
	};

}

