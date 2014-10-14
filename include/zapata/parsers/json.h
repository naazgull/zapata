/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <string>
#include <zapata/json/JSONObj.h>
#include <zapata/parsers/JSONParser.h>

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
