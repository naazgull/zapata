/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

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
#include <zapata/redis/convert_redis.h>
#include <zapata/json/json.h>
#include <zapata/log/log.h>

auto zpt::redis::to_regex(zpt::json _regexp) -> std::string {
	std::string _return;
	if( _regexp->type() == zpt::JSObject) {
		for (auto _r : _regexp->obj()) {
			_return += std::string("*\"") + _r.first + std::string("\":") + zpt::redis::to_regex(_r.second);
		}	
	}
	else if (_regexp->type() == zpt::JSArray) {
		for (auto _r : _regexp->arr()) {
			_return +=  std::string("*") + zpt::redis::to_regex(_r);
		}	
	}
	else {
		std::string _value;
		_regexp->stringify(_value);
		return _value;
	}
	return _return;
}

