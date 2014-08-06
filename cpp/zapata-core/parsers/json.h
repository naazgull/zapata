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
#include <json/JSONObj.h>
#include <parsers/JSONParser.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	void fromfile(ifstream& _in, JSONObj& _out) ;
	void fromfile(ifstream& _in, JSONArr& _out) ;
	void fromfile(ifstream& _in, JSONElement** _out, zapata::JSONType* type ) ;

	void fromstr(string& _in, JSONObj& _out) ;
	void fromstr(string& _in, JSONArr& _out) ;
	void fromstr(string& _in, JSONElement** _out, zapata::JSONType* type ) ;

	void tostr(string& _out, JSONElement& _in) ;
	void tostr(string& _out, JSONObj& _in) ;
	void tostr(string& _out, JSONArr& _in) ;

}
