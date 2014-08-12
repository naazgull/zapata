/*
    Author: Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>
    Copyright (c) 2014 Pedro (n@zgul)Figueiredo
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
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

