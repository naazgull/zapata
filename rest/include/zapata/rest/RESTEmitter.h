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

#define REST_ACCESS_CONTROL_HEADERS "X-Cid,X-Status,X-No-Redirection,X-Redirect-To,Authorization,Accept,Accept-Language,Cache-Control,Connection,Content-Length,Content-Type,Cookie,Date,Expires,Location,Origin,Server,X-Requested-With,X-Replied-With,Pragma,Cache-Control,E-Tag"

#define no_get nullptr
#define no_put nullptr
#define no_post nullptr
#define no_delete nullptr
#define no_head nullptr
#define no_trace nullptr
#define no_options nullptr
#define no_patch nullptr
#define no_connect nullptr

namespace zpt {

	enum RESTfulType {
		RESTfulResource = 0,
		RESTfulDocument = 1,
		RESTfulCollection = 2,
		RESTfulStore = 3,
		RESTfulController = 4
	};

	class RESTEmitter;
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
		virtual zpt::ev::emitter events();
		virtual zpt::mutation::emitter mutations();

		virtual bool route_http(zpt::socketstream_ptr _cs);
		virtual bool route_mqtt(std::iostream& _cs);

		virtual auto assync_on(std::string _regex, zpt::json _opts) -> void;
		
	private:
		std::string __name;
		zpt::ev::emitter __emitter;
		zpt::poll __poll;
		zpt::json __options;
		std::vector< zpt::socket > __pub_sub;
		std::vector< zpt::socket > __router_dealer;
		std::vector< std::shared_ptr< std::thread > > __threads;
		zpt::rest::server __self;
	};

	class RESTClient {
	public:
		RESTClient(zpt::json _options);
		virtual ~RESTClient();

		virtual void start();

		virtual zpt::json options();
		virtual zpt::poll poll();
		virtual zpt::ev::emitter events();
		virtual zpt::mutation::emitter mutations();

		virtual zpt::socket bind(short _type, std::string _connection);
		virtual zpt::socket bind(std::string _object_path);

	private:
		zpt::ev::emitter __emitter;
		zpt::poll __poll;
		zpt::json __options;
	};

	class RESTEmitter : public zpt::EventEmitter {
	public:
		RESTEmitter(zpt::json _options);
		virtual ~RESTEmitter();

		virtual auto version() -> std::string;
		
		virtual auto on(zpt::ev::performative _method, std::string _regex,  zpt::ev::Handler _handler, zpt::json _opts = zpt::undefined) -> std::string;
		virtual auto on(std::string _regex,  std::map< zpt::ev::performative, zpt::ev::Handler > _handlers, zpt::json _opts = zpt::undefined) -> std::string;
		virtual auto on(zpt::ev::listener _listener, zpt::json _opts = zpt::undefined) -> std::string;
		virtual auto off(zpt::ev::performative _method, std::string _callback_id) -> void;
		virtual auto off(std::string _callback_id) -> void;
		
		virtual auto trigger(zpt::ev::performative _method, std::string _resource, zpt::json _payload) -> zpt::json;
		virtual auto route(zpt::ev::performative _method, std::string _resource, zpt::json _payload) -> zpt::json;

		virtual auto poll(zpt::poll _poll) -> void;
		virtual auto poll() -> zpt::poll;
		virtual auto server(zpt::rest::server _server) -> void;
		virtual auto server() -> zpt::rest::server;
		
	private:
		zpt::ev::Handler __default_get;
		zpt::ev::Handler __default_put;
		zpt::ev::Handler __default_post;
		zpt::ev::Handler __default_delete;
		zpt::ev::Handler __default_head;
		zpt::ev::Handler __default_options;
		zpt::ev::Handler __default_patch;
		zpt::ev::Handler __default_assync_reply;
		zpt::ev::HandlerStack __resources;
		zpt::ev::ReplyHandlerStack __replies;
		zpt::poll __poll;
		zpt::rest::server __server;

	};

	namespace rest {
		auto not_found(std::string _resource) -> zpt::json;
		auto accepted(std::string _resource) -> zpt::json;
		auto no_content(std::string _resource) -> zpt::json;
		auto temporary_redirect(std::string _resource, std::string _target_resource) -> zpt::json;
		auto see_other(std::string _resource, std::string _target_resource) -> zpt::json;
		auto options(std::string _resource, std::string _origin) -> zpt::json;

		auto url_pattern(zpt::json _to_join) -> std::string;

		namespace cookies {
			auto deserialize(std::string _cookie_header) -> zpt::json;
			auto serialize(zpt::json _credentials) -> std::string;
		}
	}

}

