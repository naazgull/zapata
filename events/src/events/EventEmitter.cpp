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

void zapata::fromstr(string& _in, zapata::ev::Performative * _out) {
	if (_in == string("GET")) {
		*_out = zapata::ev::Get;
	}
	if (_in == string("PUT")) {
		*_out = zapata::ev::Put;
	}
	if (_in == string("POST")) {
		*_out = zapata::ev::Post;
	}
	if (_in == string("DELETE")) {
		*_out = zapata::ev::Delete;
	}
	if (_in == string("HEAD")) {
		*_out = zapata::ev::Head;
	}
	if (_in == string("OPTIONS")) {
		*_out = zapata::ev::Options;
	}
	if (_in == string("PATCH")) {
		*_out = zapata::ev::Patch;
	}
}

extern "C" int zapata_events() {
	return 1;
}
