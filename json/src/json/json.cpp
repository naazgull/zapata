/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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

#include <zapata/json/json.h>

zpt::json zpt::split(std::string _to_split, std::string _separator) {
	std::istringstream _iss(_to_split);
	std::string _part;
	zpt::json _ret = zpt::mkarr();
	while(_iss.good()) {
		getline(_iss, _part, _separator[0]);
		if (_part.length() != 0) {
			_ret << _part;
		}
	}
	return _ret;
}

std::string zpt::join(zpt::json _to_join, std::string _separator) {
	assertz(_to_join->type() == zpt::JSArray, "JSON to join must be an array", 412, 0);
	std::string _return;
	for (auto _a : _to_join->arr()) {
		if (_return.length() != 0) {
			_return += _separator;
		}
		_return += ((string) _a);
	}
	return _return;
}

zpt::json zpt::path::split(std::string _to_split) {
	return zpt::split(_to_split, "/");
}

std::string zpt::path::join(zpt::json _to_join) {
	return std::string("/") + zpt::join(_to_join, "/");
}
