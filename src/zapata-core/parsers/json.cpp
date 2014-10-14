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

#include <zapata/parsers/json.h>

void zapata::fromstr(string& _in, JSONObj& _out) {
	istringstream _ss;
	_ss.str(_in);
	zapata::JSONParser _p;
	_p.switchRoots(&_out);
	_p.switchStreams(_ss);
	_p.parse();

}

void zapata::fromstr(string& _in, JSONArr& _out) {
	istringstream _ss;
	_ss.str(_in);
	zapata::JSONParser _p;
	_p.switchRoots(NULL, &_out);
	_p.switchStreams(_ss);
	_p.parse();
}

void zapata::fromstr(string& _in, JSONElement** _out, zapata::JSONType* _type )  {
	istringstream _ss;
	_ss.str(_in);

	zapata::JSONParser _p;
	JSONObj _o;
	JSONArr _a;
	_p.switchRoots(&_o, &_a);
	_p.switchStreams(_ss);
	_p.parse();

	if (_p.d_scanner.__root_type == zapata::JSObject) {
		*_type = zapata::JSObject;
		*_out = _o.get();
	}
	else {
		*_type = zapata::JSArray;
		*_out = _a.get();
	}
}

void zapata::fromfile(ifstream& _in, JSONObj& _out) {
	if (_in.is_open()) {
		zapata::JSONParser _p;
		_p.switchRoots(&_out);
		_p.switchStreams(_in);
		_p.parse();
	}
}

void zapata::fromfile(ifstream& _in, JSONArr& _out) {
	if (_in.is_open()) {
		zapata::JSONParser _p;
		_p.switchRoots(NULL, &_out);
		_p.switchStreams(_in);
		_p.parse();
	}
}

void zapata::fromfile(ifstream& _in, JSONElement** _out, zapata::JSONType* _type )  {
	if (_in.is_open()) {
		zapata::JSONParser _p;
		JSONObj _o;
		JSONArr _a;
		_p.switchRoots(&_o, &_a);
		_p.switchStreams(_in);
		_p.parse();

		if (_p.d_scanner.__root_type == zapata::JSObject) {
			*_type = zapata::JSObject;
			*_out = _o.get();
		}
		else {
			*_type = zapata::JSArray;
			*_out = _a.get();
		}
	}
}

void zapata::tostr(string& _out, JSONElement& _in)  {
	_in.stringify(_out, _in.flags());
}

void zapata::tostr(string& _out, JSONObj& _in)  {
	_in->stringify(_out, _in->flags());
}

void zapata::tostr(string& _out, JSONArr& _in)  {
	_in->stringify(_out, _in->flags());
}
