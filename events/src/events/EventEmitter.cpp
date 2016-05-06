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

zapata::EventEmitter::EventEmitter() : __self( this ) {
}

zapata::EventEmitter::EventEmitter(zapata::JSONObj& _options) :  __options( _options), __self( this ) {
	
}

zapata::EventEmitter::~EventEmitter() {
}

zapata::JSONObj& zapata::EventEmitter::options() {
	return this->__options;
}

zapata::EventEmitterPtr zapata::EventEmitter::self() {
	return this->__self;
}

void zapata::EventEmitter::add_kb(std::string _name, zapata::KBPtr _kb) {
	this->__kb.insert(make_pair(_name, _kb));
}

zapata::KBPtr zapata::EventEmitter::get_kb(std::string _name) {
	auto _found = this->__kb.find(_name);
	if (_found == this->__kb.end()) {
		return zapata::KBPtr(nullptr);
	}
	return _found->second;
}

zapata::JSONPtr zapata::split(std::string _to_split, std::string _separator) {
	std::istringstream _iss(_to_split);
	std::string _part;
	zapata::JSONArr _ret;
	while(_iss.good()) {
		getline(_iss, _part, _separator[0]);
		if (_part.length() != 0) {
			_ret << _part;
		}
	}
	return make_ptr(_ret);
}

std::string zapata::join(zapata::JSONPtr _to_join, std::string _separator) {
	assertz(_to_join->type() == zapata::JSArray, "JSON to join must be an array", 412, 0);
	std::string _return;
	for (auto _a : _to_join->arr()) {
		if (_return.length() != 0) {
			_return += _separator;
		}
		_return += ((string) _a);
	}
	return _return;
}


zapata::JSONPtr zapata::ev::split(std::string _url, zapata::JSONPtr _orphans) {
	zapata::JSONObj _ret;
	zapata::JSONPtr _splited = zapata::split(_url, "/");

	size_t _idx = 0;
	for (auto _label : _orphans->arr()) {
		_ret << (string) _label << (string) _splited[_idx];
		_idx++;
	}
	for (; _idx < _splited->arr()->size(); _idx++) {
		_ret << (string) _splited[_idx] << (string) _splited[_idx + 1];
		_idx++;
	}
	return make_ptr(_ret);
}

std::string zapata::ev::join(zapata::JSONPtr _info, size_t _orphans) {
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

std::string zapata::ev::to_str(zapata::ev::Performative _performative) {
	switch(_performative) {
		case zapata::ev::Get : {
			return "GET";
		}
		case zapata::ev::Put : {
			return "PUT";
		}
		case zapata::ev::Post : {
			return "POST";
		}
		case zapata::ev::Delete : {
			return "DELETE";
		}
		case zapata::ev::Head : {
			return "HEAD";
		}
		case zapata::ev::Options : {
			return "OPTIONS";
		}
		case zapata::ev::Patch: {
			return "PATCH";
		}
	}
	return "HEAD";
}

zapata::ev::Performative zapata::ev::from_str(std::string _performative) {
	if (_performative == "GET") {
		return zapata::ev::Get;
	}
	if (_performative == "PUT") {
		return zapata::ev::Put;
	}
	if (_performative == "POST") {
		return zapata::ev::Post;
	}
	if (_performative == "DELETE") {
		return zapata::ev::Delete;
	}
	if (_performative == "HEAD") {
		return zapata::ev::Head;
	}
	if (_performative == "OPTIONS") {
		return zapata::ev::Options;
	}
	if (_performative == "PATCH") {
		return zapata::ev::Patch;
	}
	return zapata::ev::Head;
}

extern "C" int zapata_events() {
	return 1;
}
