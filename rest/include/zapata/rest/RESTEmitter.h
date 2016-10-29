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
#include <regex.h>
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

	class RESTEmitter : public zpt::EventEmitter {
	public:
		RESTEmitter(zpt::json _options);
		virtual ~RESTEmitter();

		virtual std::string on(zpt::ev::performative _method, std::string _regex,  zpt::ev::Handler _handler);
		virtual std::string on(std::string _regex,  zpt::ev::Handler _handlers[7]);
		virtual std::string on(std::string _regex,  std::map< zpt::ev::performative, zpt::ev::Handler > _handlers);
		virtual std::string on(zpt::ev::listener _listener);
		virtual void off(zpt::ev::performative _method, std::string _callback_id);
		virtual void off(std::string _callback_id);

		virtual zpt::json trigger(zpt::ev::performative _method, std::string _resource, zpt::json _payload);
		virtual zpt::json route(zpt::ev::performative _method, std::string _resource, zpt::json _payload);

		virtual void poll(zpt::poll _poll);
		
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

	};

	namespace rest {
		zpt::json not_found(std::string _resource);
		zpt::json accepted(std::string _resource);
		zpt::json no_content(std::string _resource);
		zpt::json temporary_redirect(std::string _resource, std::string _target_resource);
		zpt::json see_other(std::string _resource, std::string _target_resource);
		zpt::json options(std::string _resource, std::string _origin);

		std::string url_pattern(zpt::json _to_join);

		namespace cookies {
			zpt::json deserialize(std::string _cookie_header);
			std::string serialize(zpt::json _credentials);
		}
	}

}

