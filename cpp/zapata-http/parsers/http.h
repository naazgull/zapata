#pragma once

#include <string>
#include <http/HTTPObj.h>
#include <parsers/HTTPParser.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	void fromfile(ifstream& _in, HTTPReq& _out) ;
	void fromfile(ifstream& _in, HTTPRep& _out) ;
	void fromfile(ifstream& _in, HTTPElement** _out, zapata::HTTPType* type ) ;

	void fromstream(istream& _in, HTTPReq& _out) ;
	void fromstream(istream& _in, HTTPRep& _out) ;
	void fromstream(istream& _in, HTTPElement** _out, zapata::HTTPType* type ) ;

	void fromstr(string& _in, HTTPReq& _out) ;
	void fromstr(string& _in, HTTPRep& _out) ;
	void fromstr(string& _in, HTTPElement** _out, zapata::HTTPType* type ) ;

	void tostr(string& _out, HTTPElement& _in) ;
	void tostr(string& _out, HTTPReq& _in) ;
	void tostr(string& _out, HTTPRep& _in) ;

}
