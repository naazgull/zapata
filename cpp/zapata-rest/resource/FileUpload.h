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

#include <resource/RESTController.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class FileUpload: public zapata::RESTController {
		public:
			FileUpload();
			virtual ~FileUpload();

			virtual void post(zapata::HTTPReq& _req, zapata::HTTPRep& _rep);
	};

}
