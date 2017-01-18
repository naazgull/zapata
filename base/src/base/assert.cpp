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
		}
		else {
			zpt::tz->assign("UTC");
		}
	}
	return *zpt::tz;
}

auto zpt::ev::to_str(zpt::ev::performative _performative) -> std::string {
	switch(_performative) {
		case zpt::ev::Get : {
			return "GET";
		}
		case zpt::ev::Put : {
			return "PUT";
		}
		case zpt::ev::Post : {
			return "POST";
		}
		case zpt::ev::Delete : {
			return "DELETE";
		}
		case zpt::ev::Head : {
			return "HEAD";
		}
		case zpt::ev::Options : {
			return "OPTIONS";
		}
		case zpt::ev::Patch: {
			return "PATCH";
		}
		case zpt::ev::Reply: {
			return "REPLY";
		}
	}
	return "HEAD";
}

auto zpt::ev::from_str(std::string _performative) -> zpt::ev::performative {
	if (_performative == "GET") {
		return zpt::ev::Get;
	}
	if (_performative == "PUT") {
		return zpt::ev::Put;
	}
	if (_performative == "POST") {
		return zpt::ev::Post;
	}
	if (_performative == "DELETE") {
		return zpt::ev::Delete;
	}
	if (_performative == "HEAD") {
		return zpt::ev::Head;
	}
	if (_performative == "OPTIONS") {
		return zpt::ev::Options;
	}
	if (_performative == "PATCH") {
		return zpt::ev::Patch;
	}
	if (_performative == "REPLY") {
		return zpt::ev::Reply;
	}
	return zpt::ev::Head;
}

auto zpt::mutation::to_str(zpt::mutation::operation _performative) -> std::string{
	switch(_performative) {
		case zpt::mutation::Insert : {
			return "INSERT";
		}
		case zpt::mutation::Remove : {
			return "REMOVE";
		}
		case zpt::mutation::Update : {
			return "UPDATE";
		}
		case zpt::mutation::Replace : {
			return "REPLACE";
		}
		case zpt::mutation::Connect : {
			return "CONNECT";
		}
		case zpt::mutation::Reconnect : {
			return "RECONNECT";
		}
	}
	return "";
}

auto zpt::mutation::from_str(std::string _performative) -> zpt::mutation::operation {
	if (_performative == "INSERT") {
		return zpt::mutation::Insert;
	}
	if (_performative == "REMOVE") {
		return zpt::mutation::Remove;
	}
	if (_performative == "UPDATE") {
		return zpt::mutation::Update;
	}
	if (_performative == "REPLACE") {
		return zpt::mutation::Replace;
	}
	if (_performative == "CONNECT") {
		return zpt::mutation::Connect;
	}
	if (_performative == "RECONNECT") {
		return zpt::mutation::Reconnect;
	}
	return zpt::mutation::Insert;
}

zpt::tm_ptr zpt::get_time(time_t _t) {
	std::tm* _tm = new std::tm();
	std::memcpy(_tm, localtime(&_t), sizeof(std::tm));
	return zpt::tm_ptr(_tm);
}
