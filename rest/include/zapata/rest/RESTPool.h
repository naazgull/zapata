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

	class RESTPool;

	typedef std::shared_ptr<zapata::RESTPool> RESTPoolPtr;
	typedef std::function<zapata::JSONPtr (zapata::HTTPMethod _method, std::string _url, zapata::JSONPtr, zapata::RESTPoolPtr&)> RESTHandler;
	typedef RESTHandler RESTCallback;
	typedef vector<pair<regex_t*, vector<zapata::RESTHandler> > > RESTHandlerStack;

	class RESTPool {
		public:
			explicit RESTPool(zapata::JSONObj& _options);
			virtual ~RESTPool();

			virtual zapata::JSONObj& options();

			virtual void on(zapata::HTTPMethod _method, string _regex, zapata::RESTHandler _handler);
			virtual void on(string _regex, zapata::RESTHandler _handlers[9]);

			virtual zapata::JSONPtr trigger(zapata::HTTPMethod _method, std::string _url, zapata::JSONPtr _payload, bool _is_ssl = false);

			virtual void add_kb(std::string _name, zapata::KBPtr _kb);
			virtual zapata::KBPtr get_kb(std::string _name);

		private:
			zapata::JSONObj __options;
			zapata::RESTHandler __default_get;
			zapata::RESTHandler __default_put;
			zapata::RESTHandler __default_post;
			zapata::RESTHandler __default_delete;
			zapata::RESTHandler __default_head;
			zapata::RESTHandler __default_trace;
			zapata::RESTHandler __default_options;
			zapata::RESTHandler __default_patch;
			zapata::RESTHandler __default_connect;
			zapata::RESTHandlerStack __resources;
			zapata::RESTPoolPtr __self;
			std::map<std::string, zapata::KBPtr> __kb;

			void init(zapata::HTTPRep& _rep);
			void init(zapata::HTTPReq& _req);
			void resttify(zapata::JSONPtr _body, zapata::HTTPReq& _request);
			zapata::JSONPtr process(zapata::HTTPMethod _method, std::string _url, zapata::JSONPtr _payload);
	};

}

