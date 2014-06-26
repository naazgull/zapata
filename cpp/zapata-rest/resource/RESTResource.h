#pragma once

#include <regex.h>
#include <base/assert.h>
#include <json/JSONObj.h>
#include <zapata/http.h>

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

	class RESTResource {
		public:
			RESTResource(string _url_pattern);
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
			void invoke(HTTPReq& _req, HTTPRep& _rep, bool _is_ssl = false);

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
