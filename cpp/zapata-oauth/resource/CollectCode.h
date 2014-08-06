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

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>
#include <resource/RESTResource.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class CollectCode: public RESTResource {
		public:
			CollectCode();
			virtual ~CollectCode();

			virtual void get(HTTPReq& _req, HTTPRep& _rep);
			virtual void post(HTTPReq& _req, HTTPRep& _rep);

			virtual bool usrtoken(string _id, string _secret, string _code, zapata::JSONObj& _out_token) = 0;
			virtual bool apptoken(string _id, string _secret, string _code, zapata::JSONObj& _out_token) = 0;

	};

}
