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

#include <zapata/http/params.h>

#define __SPECIAL_PARAMS string("orderBy pageSize pageStartIndex fields embed ")

void zapata::fromparams(zapata::HTTPReq& _in, zapata::JSONObj& _out, zapata::RESTfulType _resource_type, bool _regexp) {
	if (_resource_type != zapata::RESTfulResource) {
		switch(_resource_type) {
			case zapata::RESTfulDocument : {
				_out << "_id" << _in->url();
				break;
			}
			default : { // collection/store
				string _uri(_in->url());
				_uri.insert(0, "^");
				_uri.insert(_uri.length(), "/([^/]+)$");
				zapata::replace(_uri, "/", "\\\\/");
				_uri.insert(0, "m/");
				_uri.insert(_uri.length(), "/i");
				_out << "_id" << _uri;
				break;
			}
		}
	}
	for (auto _i : _in->params()) {
		string _value = _i.second;
		string _key(_i.first.data());
		zapata::url::decode(_value);
		if (_regexp && __SPECIAL_PARAMS.find(_key + string(" ")) == string::npos) {
			zapata::replace(_value, "/", "\\\\/");
			zapata::replace(_value, " ", "(.*)");
			_value.insert(0, "m/");
			_value.insert(_value.length(), "/i");
		}
		_out << _i.first << _value;
	}
}
