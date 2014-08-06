/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

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

