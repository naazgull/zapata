#pragma once

#include <resource/RESTResource.h>

namespace zapata {

	class RESTController: public RESTResource {
		public:
			RESTController(string _url_pattern);
			virtual ~RESTController();

			virtual void get(HTTPReq& _req, HTTPRep& _rep) final;
			virtual void put(HTTPReq& _req, HTTPRep& _rep) final;
			virtual void post(HTTPReq& _req, HTTPRep& _rep);
			virtual void remove(HTTPReq& _req, HTTPRep& _rep) final;
			virtual void head(HTTPReq& _req, HTTPRep& _rep) final;
			virtual void patch(HTTPReq& _req, HTTPRep& _rep) final;

	};

}
