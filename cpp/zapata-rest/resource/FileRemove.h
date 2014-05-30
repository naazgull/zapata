#pragma once

#include <resource/RESTController.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class FileUpload: public zapata::RESTController {
		public:
			FileUpload();
			virtual ~FileUpload();

			virtual void post(zapata::HTTPReq& _req, zapata::HTTPRep& _rep);
	};

}
