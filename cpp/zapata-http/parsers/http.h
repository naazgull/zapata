/*
    Author: Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>
    Copyright (c) 2014 Pedro (n@zgul)Figueiredo
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
