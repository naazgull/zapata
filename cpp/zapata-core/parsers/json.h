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
