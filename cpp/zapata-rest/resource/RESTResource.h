#pragma once

#include <regex.h>
#include <zapata/http.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

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

		private:
			regex_t* __url_pattern;
	};

}
