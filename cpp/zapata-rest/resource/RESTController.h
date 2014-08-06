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
