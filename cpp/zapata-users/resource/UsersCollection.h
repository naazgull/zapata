#pragma once

#include <resource/RESTCollection.h>

namespace zapata {
	
	class UsersCollection: public RESTCollection {
		public:
			UsersCollection();
			virtual ~UsersCollection();

			virtual void get(HTTPReq& _req, HTTPRep& _rep);
			virtual void post(HTTPReq& _req, HTTPRep& _rep);
			virtual void head(HTTPReq& _req, HTTPRep& _rep);

	};

}
