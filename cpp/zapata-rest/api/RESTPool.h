#pragma once

#include <resource/RESTResource.h>

namespace zapata {

	class RESTPool {
		public:
			RESTPool();
			virtual ~RESTPool();

			//void populate();
			JSONObj& configuration();
			void configuration(JSONObj* _conf);

			void add(RESTResource* _res);
			void init(HTTPRep& _rep);
			void process(HTTPReq& _req, HTTPRep& _rep);

		private:
			vector<RESTResource*> __resources;
			JSONObj* __configuration;
	};

}

