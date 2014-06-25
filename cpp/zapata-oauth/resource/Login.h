#pragma once

#include <string>
#include <resource/RESTController.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class Login: public RESTController {
		public:
			Login();
			virtual ~Login();

			virtual void post(HTTPReq& _req, HTTPRep& _rep);
			virtual bool authenticate(string _id, string _secret, string& _out_code) = 0;

	};

}
