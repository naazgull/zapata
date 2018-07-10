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

#include <zapata/base/assert.h>
#include <fstream>

namespace zpt {

std::string* tz = nullptr;
}

std::string zpt::get_tz() {
	if (zpt::tz == nullptr) {
		zpt::tz = new string();
		std::ifstream _tzf;
		_tzf.open("/etc/timezone");
		if (_tzf.is_open()) {
			_tzf >> (*zpt::tz);
			_tzf.close();
		} else {
			zpt::tz->assign("UTC");
		}
	}
	return *zpt::tz;
}

auto zpt::to_str(zpt::performative _performative) -> std::string {
	// switch (_performative) {
	// case zpt::Get: {
	// 	return "GET";
	// }
	// case zpt::Put: {
	// 	return "PUT";
	// }
	// case zpt::Post: {
	// 	return "POST";
	// }
	// case zpt::Delete: {
	// 	return "DELETE";
	// }
	// case zpt::Head: {
	// 	return "HEAD";
	// }
	// case zpt::Options: {
	// 	return "OPTIONS";
	// }
	// case zpt::Patch: {
	// 	return "PATCH";
	// }
	// case zpt::Reply: {
	// 	return "REPLY";
	// }
	// case zpt::Search: {
	// 	return "M-SEARCH";
	// }
	// case zpt::Notify: {
	// 	return "NOTIFY";
	// }
	// case zpt::Trace: {
	// 	return "TRACE";
	// }
	// case zpt::Connect: {
	// 	return "CONNECT";
	// }
	// }
	// return "HEAD";
	return "";
}

auto zpt::from_str(std::string _performative) -> zpt::performative {
	// if (_performative == "GET" || _performative == "get") {
	// 	return zpt::Get;
	// }
	// if (_performative == "PUT" || _performative == "put") {
	// 	return zpt::Put;
	// }
	// if (_performative == "POST" || _performative == "post") {
	// 	return zpt::Post;
	// }
	// if (_performative == "DELETE" || _performative == "delete") {
	// 	return zpt::Delete;
	// }
	// if (_performative == "HEAD" || _performative == "head") {
	// 	return zpt::Head;
	// }
	// if (_performative == "OPTIONS" || _performative == "options") {
	// 	return zpt::Options;
	// }
	// if (_performative == "PATCH" || _performative == "patch") {
	// 	return zpt::Patch;
	// }
	// if (_performative == "REPLY" || _performative == "reply") {
	// 	return zpt::Reply;
	// }
	// if (_performative == "M-SEARCH" || _performative == "m-search") {
	// 	return zpt::Search;
	// }
	// if (_performative == "NOTIFY" || _performative == "notify") {
	// 	return zpt::Notify;
	// }
	// if (_performative == "TRACE" || _performative == "trace") {
	// 	return zpt::Search;
	// }
	// if (_performative == "CONNECT" || _performative == "connect") {
	// 	return zpt::Search;
	// }
	return 0;
}

zpt::tm_ptr zpt::get_time(time_t _t) {
	std::tm* _tm = new std::tm();
	std::memcpy(_tm, localtime(&_t), sizeof(std::tm));
	return zpt::tm_ptr(_tm);
}
