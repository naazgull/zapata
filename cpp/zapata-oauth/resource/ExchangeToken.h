#pragma once

#include <string>
#include <resource/RESTResource.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class ExchangeToken: public RESTResource {
		public:
			ExchangeToken();
			virtual ~ExchangeToken();

			virtual void get(HTTPReq& _req, HTTPRep& _rep);
			virtual void post(HTTPReq& _req, HTTPRep& _rep);

			virtual bool usrtoken(string _id, string _secret, string _code, zapata::JSONObj& _out_token) = 0;
			virtual bool apptoken(string _id, string _secret, string _code, zapata::JSONObj& _out_token) = 0;

	};

}
