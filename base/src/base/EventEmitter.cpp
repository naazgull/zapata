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

#include <zapata/base/EventEmitter.h>

zapata::EventEmitterPtr::EventEmitterPtr() : std::shared_ptr<zapata::EventEmitter>(new zapata::EventEmitter()) {
}

zapata::EventEmitterPtr::EventEmitterPtr(zapata::JSONObj& _options) : std::shared_ptr<zapata::EventEmitter>(new zapata::EventEmitter(_options)) {
}

zapata::EventEmitterPtr::EventEmitterPtr(zapata::EventEmitter * _target) : std::shared_ptr<zapata::EventEmitter>(_target) {
}

zapata::EventEmitterPtr::~EventEmitterPtr() {
}

zapata::EventEmitter::EventEmitter() : __self( this ) {
}

zapata::EventEmitter::EventEmitter(zapata::JSONObj& _options) :  __self( this ), __options( _options) {
}

zapata::EventEmitter::~EventEmitter() {
}

zapata::JSONObj& zapata::EventEmitter::options() {
	return this->__options;
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
