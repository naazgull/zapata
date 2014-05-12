#pragma once

#include <resource/RESTResource.h>

namespace zapata {

	class RESTCollection: public RESTResource {
		public:
			RESTCollection(string _url_pattern);
			virtual ~RESTCollection();

			virtual void get(HTTPReq& _req, HTTPRep& _rep);
			virtual void put(HTTPReq& _req, HTTPRep& _rep) final;
			virtual void post(HTTPReq& _req, HTTPRep& _rep);
			virtual void remove(HTTPReq& _req, HTTPRep& _rep) final;
			virtual void head(HTTPReq& _req, HTTPRep& _rep);
			virtual void trace(HTTPReq& _req, HTTPRep& _rep);
			virtual void options(HTTPReq& _req, HTTPRep& _rep);
			virtual void patch(HTTPReq& _req, HTTPRep& _rep) final;
			virtual void connect(HTTPReq& _req, HTTPRep& _rep);

	};

}
