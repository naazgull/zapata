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

#include <regex.h>
#include <base/assert.h>
#include <json/JSONObj.h>
#include <zapata/http.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class RESTPool;

	class RESTResource {
		public:
			RESTResource();
			virtual ~RESTResource();

			virtual void get(HTTPReq& _req, HTTPRep& _rep);
			virtual void put(HTTPReq& _req, HTTPRep& _rep);
			virtual void post(HTTPReq& _req, HTTPRep& _rep);
			virtual void remove(HTTPReq& _req, HTTPRep& _rep);
			virtual void head(HTTPReq& _req, HTTPRep& _rep);
			virtual void trace(HTTPReq& _req, HTTPRep& _rep) final;
			virtual void options(HTTPReq& _req, HTTPRep& _rep);
			virtual void patch(HTTPReq& _req, HTTPRep& _rep);
			virtual void connect(HTTPReq& _req, HTTPRep& _rep) final;

			virtual bool relations(HTTPReq& _req, JSONObj& _out);
			virtual void fields(HTTPReq& _req, JSONObj& _in_out);
			virtual void embed(HTTPReq& _req, JSONObj& _in_out);

			virtual bool allowed(HTTPReq& _req);
			bool matches(string _url);

			void invoke(string _url, HTTPReq& _req, HTTPRep& _rep, bool _is_ssl = false);
			void invoke(string _url, HTTPMethod _method, HTTPRep& _rep, bool _is_ssl = false);
			void invoke(HTTPReq& _req, HTTPRep& _rep, bool _is_ssl = false);

			regex_t* pattern();
			JSONObj& configuration();
			void configuration(JSONObj* _conf);
			RESTPool& pool();
			void pool(RESTPool* _pool);

		private:
			regex_t* __url_pattern;
			//JSONObj* __configuration;
			RESTPool* __pool;
	};

}
