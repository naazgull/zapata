/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

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

#include <zapata/json.h>
#include <zapata/events.h>
#include <string>
#include <map>
#include <memory>
#include <zapata/rest/RESTProtocol.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

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

namespace rest {
typedef zpt::RESTEmitter emitter;
typedef std::map<std::string, std::vector<std::pair<std::regex, zpt::ev::handlers>>> HandlerStack;

typedef zpt::ev::handler step;
typedef zpt::ev::initializer end;
}

class RESTEmitter : public zpt::EventEmitter {
      public:
	RESTEmitter(zpt::json _options);
	virtual ~RESTEmitter();

	virtual auto version() -> std::string;
	virtual auto credentials() -> zpt::json;
	virtual auto credentials(zpt::json _credentials) -> void;

	virtual auto on(zpt::ev::performative _method,
			std::string _regex,
			zpt::ev::Handler _handler,
			zpt::json _opts = zpt::undefined) -> void;
	virtual auto on(std::string _regex,
			std::map<zpt::ev::performative, zpt::ev::Handler> _handlers,
			zpt::json _opts = zpt::undefined) -> void;
	virtual auto on(zpt::ev::listener _listener, zpt::json _opts = zpt::undefined) -> void;

	virtual auto trigger(zpt::ev::performative _method,
			     std::string _resource,
			     zpt::json _payload,
			     zpt::json _opts = zpt::undefined,
			     zpt::ev::handler _callback = nullptr) -> void;
	virtual auto route(zpt::ev::performative _method,
			   std::string _resource,
			   zpt::json _payload,
			   zpt::json _opts = zpt::undefined,
			   zpt::ev::handler _callback = nullptr) -> void;
	virtual auto
	route(zpt::ev::performative _method, std::string _resource, zpt::json _payload, zpt::ev::handler _callback)
	    -> void;
	virtual auto reply(zpt::json _request, zpt::json _reply = zpt::undefined) -> void;

	virtual auto hook(zpt::ev::initializer _callback) -> void;
	virtual auto shutdown() -> void;
	virtual auto make_available() -> void;

	virtual auto pending(zpt::json _envelope, zpt::ev::handler _callback) -> void;
	virtual auto has_pending(zpt::json _envelope) -> bool;

	virtual auto poll(zpt::poll _poll) -> void;
	virtual auto poll() -> zpt::poll;
	virtual auto server(zpt::pipeline _server) -> void;
	virtual auto server() -> zpt::pipeline;

	static auto instance() -> zpt::ev::emitter;

      private:
	zpt::ev::Handler __default_options;
	zpt::ev::ReplyHandlerStack __pending;
	zpt::json __credentials;

	auto resolve(zpt::ev::performative _method,
		     std::string _url,
		     zpt::json _envelope,
		     zpt::json _opts,
		     zpt::ev::handler _callback = nullptr) -> void;
	auto sync_resolve(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts)
	    -> zpt::json;
};

namespace rest {

auto url_pattern(zpt::json _to_join) -> std::string;
auto collect(zpt::json _args, zpt::json _to_collect_from, zpt::rest::step _step, zpt::rest::end _end) -> void;
auto _collect(zpt::json _args,
	      zpt::json _to_collect_from,
	      size_t _idx,
	      zpt::json _previous,
	      zpt::rest::step _step,
	      zpt::rest::end _end,
	      zpt::ev::emitter _emitter) -> void;
auto iterate(zpt::json _to_iterate_over, zpt::rest::step _step, zpt::rest::end _end) -> void;
auto _iterate(zpt::json _to_iterate_over,
	      size_t _idx,
	      zpt::rest::step _step,
	      zpt::rest::end _end,
	      zpt::ev::emitter _emitter) -> void;
auto _collect_variables(zpt::json _kb, zpt::json _args) -> zpt::json;
}
}
