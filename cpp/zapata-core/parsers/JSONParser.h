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

#include <json/JSONObj.h>
#include <parsers/JSONTokenizer.h>

namespace zapata {

	class JSONParser: public JSONTokenizer {
		public:
			JSONParser(std::istream &_in = std::cin, std::ostream &_out = std::cout, JSONObj* _rootobj = NULL, JSONArr* _rootarr = NULL);
			virtual ~JSONParser();

			void switchRoots(JSONObj* _rootobj = NULL, JSONArr* _rootarr = NULL);
			void switchStreams(std::istream &_in = std::cin, std::ostream &_out = std::cout);
	};

}
