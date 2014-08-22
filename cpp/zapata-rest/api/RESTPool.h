/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

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

#include <http/HTTPObj.h>
#include <json/JSONObj.h>
#include <regex.h>
#include <string>
#include <map>

using namespace std;
using namespace __gnu_cxx;

#define REST_ACCESS_CONTROL_HEADERS "X-Access-Token,X-Access-Token-Expires,X-Error-Reason,X-Error,X-Embed,X-Filter,Authorization,Accept,Accept-Language,Cache-Control,Connection,Content-Length,Content-Type,Cookie,Date,Expires,Location,Origin,Server,X-Requested-With,X-Replied-With,X-Replied-With-Status,Pragma,Cache-Control,E-Tag"
#define __INTERNAL_TRIGGER__ double _dummy_arg
namespace zapata {

	enum RESTfulType {
		RESTfulResource = 0,
		RESTfulDocument = 1,
		RESTfulCollection = 2,
		RESTfulStore = 3,
		RESTfulController = 4
	};

	class RESTPool;

	typedef std::function<void (zapata::HTTPReq&, zapata::HTTPRep&, zapata::JSONObj&, zapata::RESTPool&)> RESTHandler;
	typedef RESTHandler RESTCallback;
	typedef map<regex_t*, vector<zapata::RESTHandler> > RESTHandlerStack;

	class RESTPool {
		public:
			RESTPool();
			virtual ~RESTPool();

			JSONObj& configuration();
			void configuration(JSONObj* _conf);

			void on(vector<zapata::HTTPMethod> _events, string _regex, zapata::RESTHandler _handler, zapata::RESTfulType _resource_type);
			void on(zapata::HTTPMethod _event, string _regex, zapata::RESTHandler _handler, zapata::RESTfulType _resource_type);
			void on(string _regex, zapata::RESTHandler _handlers[9], zapata::RESTfulType _resource_type);
			void on(string _regex, zapata::RESTHandler _get, zapata::RESTHandler _put, zapata::RESTHandler _post, zapata::RESTHandler _delete, zapata::RESTHandler _head, zapata::RESTHandler _trace, zapata::RESTHandler _options, zapata::RESTHandler _patch, zapata::RESTHandler _connect, zapata::RESTfulType _resource_type);

			void trigger(HTTPReq& _req, HTTPRep& _rep, bool _is_ssl = false);
			void trigger(string _url, HTTPReq& _req, HTTPRep& _rep, bool _is_ssl = false);
			void trigger(string _url, HTTPMethod _method, HTTPRep& _rep, bool _is_ssl = false);
			void trigger(HTTPReq& _req, HTTPRep& _rep, __INTERNAL_TRIGGER__);

		private:
			JSONObj* __configuration;

			zapata::RESTHandler __default_get;
			zapata::RESTHandler __default_put;
			zapata::RESTHandler __default_post;
			zapata::RESTHandler __default_delete;
			zapata::RESTHandler __default_head;
			zapata::RESTHandler __default_trace;
			zapata::RESTHandler __default_options;
			zapata::RESTHandler __default_patch;
			zapata::RESTHandler __default_connect;

			RESTHandlerStack __resources;

			void init(HTTPRep& _rep);
			void process(HTTPReq& _req, HTTPRep& _rep);
	};

}

