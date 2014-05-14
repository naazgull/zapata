#pragma once

#include <resource/RESTResource.h>

namespace zapata {

	class RESTPool {
		public:
			RESTPool();
			virtual ~RESTPool();

			void populate();
			void add(RESTResource* _res);
			void process(HTTPReq& _req, HTTPRep& _rep);

		private:
			vector<RESTResource*> __resources;
	};

}

