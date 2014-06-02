#pragma once

#include <resource/RESTController.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class FileRemove: public zapata::RESTController {
		public:
			FileRemove();
			virtual ~FileRemove();

			virtual void post(zapata::HTTPReq& _req, zapata::HTTPRep& _rep);
	};

}
