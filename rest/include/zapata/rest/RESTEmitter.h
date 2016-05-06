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
#include <regex.h>
#include <string>
#include <map>
#include <memory>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

#define REST_ACCESS_CONTROL_HEADERS "X-Access-Token,X-Access-Token-Expires,X-Error-Reason,X-Error,X-Embed,X-Filter,Authorization,Accept,Accept-Language,Cache-Control,Connection,Content-Length,Content-Type,Cookie,Date,Expires,Location,Origin,Server,X-Requested-With,X-Replied-With,X-Replied-With-Status,Pragma,Cache-Control,E-Tag"

#define no_get nullptr
#define no_put nullptr
#define no_post nullptr
#define no_delete nullptr
#define no_head nullptr
#define no_trace nullptr
#define no_options nullptr
#define no_patch nullptr
#define no_connect nullptr

namespace zapata {

	enum RESTfulType {
		RESTfulResource = 0,
		RESTfulDocument = 1,
		RESTfulCollection = 2,
		RESTfulStore = 3,
		RESTfulController = 4
	};

	class RESTEmitter : public zapata::EventEmitter {
		public:
			explicit RESTEmitter(zapata::JSONObj& _options);
			virtual ~RESTEmitter();

			virtual void on(zapata::ev::Performative _method, string _regex,  zapata::ev::Handler _handler);
			virtual void on(string _regex,  zapata::ev::Handler _handlers[9]);

			virtual zapata::JSONPtr trigger(zapata::ev::Performative _method, std::string _resource, zapata::JSONPtr _payload);

		private:
			zapata::JSONObj __options;
			zapata::ev::Handler __default_get;
			zapata::ev::Handler __default_put;
			zapata::ev::Handler __default_post;
			zapata::ev::Handler __default_delete;
			zapata::ev::Handler __default_head;
			zapata::ev::Handler __default_options;
			zapata::ev::Handler __default_patch;
			zapata::ev::HandlerStack __resources;

			void init(zapata::HTTPRep& _rep);
			void init(zapata::HTTPReq& _req);
			void restify(zapata::JSONPtr _body, zapata::HTTPReq& _request);
		};

}

