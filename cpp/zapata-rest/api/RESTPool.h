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

			void invoke(HTTPReq& _req, HTTPRep& _rep, bool _is_ssl = false);

		private:
			vector<RESTResource*> __resources;
			JSONObj* __configuration;
	};

}

