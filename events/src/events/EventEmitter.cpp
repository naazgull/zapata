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

#include <zapata/events/EventEmitter.h>

zpt::EventEmitter::EventEmitter() : __self( this ) {
}

zpt::EventEmitter::EventEmitter(zpt::json _options) :  __options( _options), __self( this ) {
}

zpt::EventEmitter::~EventEmitter() {
}

zpt::json zpt::EventEmitter::options() {
	return this->__options;
}

zpt::EventEmitterPtr zpt::EventEmitter::self() {
	return this->__self;
}

void zpt::EventEmitter::add_kb(std::string _name, zpt::kb _kb) {
	auto _found = this->__kb.find(_name);
	if (_found == this->__kb.end()) {
		this->__kb.insert(make_pair(_name, _kb));
	}
}

zpt::kb zpt::EventEmitter::get_kb(std::string _name) {
	auto _found = this->__kb.find(_name);
	if (_found == this->__kb.end()) {
		return zpt::kb(nullptr);
	}
	return _found->second;
}

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


zpt::json zpt::ev::split(std::string _url, zpt::json _orphans) {
	zpt::JSONObj _ret;
	zpt::json _splited = zpt::split(_url, "/");

	size_t _idx = 0;
	for (auto _label : _orphans->arr()) {
		_ret << (string) _label << (string) _splited[_idx];
		_idx++;
	}
	for (; _idx < _splited->arr()->size(); _idx++) {
		_ret << (string) _splited[_idx] << (string) _splited[_idx + 1];
		_idx++;
	}
	return mkptr(_ret);
}

std::string zpt::ev::join(zpt::json _info, size_t _orphans) {
	std::string _ret;
	size_t _idx = 0;

	for (auto _field : _info->obj()) {
		if (_idx < _orphans) {
			_ret += string("/") + _field.second->str();
		}
		else {
			_ret += string("/") + _field.first + string("/") + _field.second->str();
		}
	}
	return _ret;
}

std::string zpt::ev::to_str(zpt::ev::performative _performative) {
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

zpt::ev::performative zpt::ev::from_str(std::string _performative) {
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

zpt::json zpt::ev::init_request(std::string _cid) {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);
	
	zpt::json _return(
		{
			"Accept", "application/json",
			"Accept-Charset", "utf-8",
			"Cache-Control", "no-cache",
			"Date", string(_buffer_date),
			"Expires", string(_buffer_expires),
			"User-Agent", "zapata RESTful server"
		}
	);
	if (_cid != "") {
		_return << "X-Cid" << _cid;
	}
	else {
		uuid _uuid;
		_uuid.make(UUID_MAKE_V1);
		_return << "X-Cid" << _uuid.string();
	}
	return _return;
}

zpt::json zpt::ev::init_reply(std::string _uuid) {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	_ptm.tm_hour += 1;
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	zpt::json _return(
		{
			"Server", "zapata RESTful server",
			"Cache-Control", "max-age=3600",
			"Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag",
			"Date", std::string(_buffer_date),
			"Expires", std::string(_buffer_expires)
		}
	);
	if (_uuid != "") {
		_return << "X-Cid" << _uuid;
	}
	return _return;
}
