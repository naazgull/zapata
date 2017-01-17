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
#include <regex.h>
#include <string>
#include <map>
#include <memory>
#include <ossp/uuid++.hh>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	class EventEmitter;
	class EventListener;

	typedef std::weak_ptr<zpt::EventEmitter> EventEmitterWPtr;
	typedef std::shared_ptr<zpt::EventEmitter> EventEmitterPtr;
	typedef std::shared_ptr<zpt::EventListener> EventListenerPtr;

	namespace ev {
		extern std::string* __default_authorization;

		typedef std::function<zpt::json (zpt::ev::performative, std::string, zpt::json, zpt::EventEmitterPtr)> Handler;
		typedef std::function<zpt::json (zpt::ev::performative, std::string, zpt::json, zpt::EventEmitterPtr)> handler;
		typedef Handler Callback;
		typedef handler callback;
		typedef std::map< std::string, pair<regex_t*, vector< zpt::ev::Handler> > > HandlerStack;
		typedef std::map< std::string, zpt::ev::Handler > ReplyHandlerStack;

		zpt::json split(std::string _url, zpt::json _orphans);
		std::string join(zpt::json _info, size_t _orphans);

		void set_default_authorization(std::string _default_authorization);
		std::string get_default_authorization();

		zpt::json init_request(std::string _cid = "");
		zpt::json init_reply(std::string _cid = "");

		typedef zpt::EventEmitterPtr emitter;
		typedef zpt::EventListenerPtr listener;
	}

	class EventEmitter {
	public:
		EventEmitter();
		EventEmitter(zpt::json _options);
		virtual ~EventEmitter();
		
		virtual auto options() -> zpt::json;
		virtual auto self() -> zpt::ev::emitter;
		virtual auto version() -> std::string;
		
		virtual auto on(zpt::ev::performative _method, std::string _regex,  zpt::ev::Handler _handler, zpt::json _opts = undefined) -> std::string = 0;
		virtual auto on(std::string _regex,  zpt::ev::Handler _handlers[7], zpt::json _opts = undefined) -> std::string = 0;
		virtual auto on(string _regex,  std::map< zpt::ev::performative, zpt::ev::Handler > _handlers, zpt::json _opts = undefined) -> std::string = 0;
		virtual auto on(zpt::ev::listener _listener, zpt::json _opts = undefined) -> std::string = 0;
		virtual auto off(zpt::ev::performative _method, std::string _callback_id) -> void = 0;
		virtual auto off(std::string _callback_id) -> void = 0;
		
		virtual auto trigger(zpt::ev::performative _method, std::string _resource, zpt::json _payload) -> zpt::json = 0;
		virtual auto route(zpt::ev::performative _method, std::string _resource, zpt::json _payload) -> zpt::json = 0;
		
		virtual auto add_connector(std::string _name, zpt::connector _connector) -> void final;
		virtual auto get_connector(std::string _name) -> zpt::connector final;

	private:
		zpt::json __options;
		zpt::ev::emitter __self;
		std::map<std::string, zpt::connector> __connector;

	};

	class EventListener {
	public:
		EventListener(std::string _regex);
		virtual ~EventListener();

		virtual std::string regex() final;
		
		virtual zpt::json get(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter);
		virtual zpt::json put(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter);
		virtual zpt::json post(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter);
		virtual zpt::json del(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter);
		virtual zpt::json head(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter);
		virtual zpt::json options(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter);
		virtual zpt::json patch(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter);
		virtual zpt::json reply(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter);

	private:
		std::string __regex;
	};

}

