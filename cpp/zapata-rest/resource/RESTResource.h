#pragma once

#include <regex.h>
#include <json/JSONObj.h>
#include <zapata/http.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class RESTPool;

	class RESTResource {
		public:
			RESTResource(string _url_pattern);
			virtual ~RESTResource();

			virtual void get(HTTPReq& _req, HTTPRep& _rep);
			virtual void put(HTTPReq& _req, HTTPRep& _rep);
			virtual void post(HTTPReq& _req, HTTPRep& _rep);
			virtual void remove(HTTPReq& _req, HTTPRep& _rep);
			virtual void head(HTTPReq& _req, HTTPRep& _rep);
			virtual void trace(HTTPReq& _req, HTTPRep& _rep);
			virtual void options(HTTPReq& _req, HTTPRep& _rep);
			virtual void patch(HTTPReq& _req, HTTPRep& _rep);
			virtual void connect(HTTPReq& _req, HTTPRep& _rep);

			virtual bool relations(HTTPReq& _req, JSONObj& _out);
			virtual void fields(HTTPReq& _req, JSONObj& _in_out);
			virtual void embed(HTTPReq& _req, JSONObj& _in_out);

			virtual bool allowed(HTTPReq& _req);
			bool matches(string _url);

			regex_t* pattern();
			JSONObj& configuration();
			void configuration(JSONObj* _conf);
			RESTPool& pool();
			void pool(RESTPool* _pool);

		private:
			regex_t* __url_pattern;
			JSONObj* __configuration;
			RESTPool* __pool;
	};

}
