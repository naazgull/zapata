#pragma once

#include <resource/RESTDocument.h>

namespace zapata {
	
	class UsersDocument: public RESTDocument {
		public:
			UsersDocument();
			virtual ~UsersDocument();

			virtual void get(HTTPReq& _req, HTTPRep& _rep);
			virtual void put(HTTPReq& _req, HTTPRep& _rep);
			virtual void remove(HTTPReq& _req, HTTPRep& _rep);
			virtual void head(HTTPReq& _req, HTTPRep& _rep);
			virtual void patch(HTTPReq& _req, HTTPRep& _rep);

	};

}
