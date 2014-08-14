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

#include <json/JSONObj.h>

#include <iostream>
#include <text/convert.h>
#include <exceptions/CastException.h>
#include <exceptions/NoAttributeNameException.h>

zapata::JSONDblRef::JSONDblRef(double _in) : __value(_in) {
}

zapata::JSONDblRef::~JSONDblRef() {
}

zapata::JSONType zapata::JSONDblRef::type() {
	return zapata::JSDouble;
}

void zapata::JSONDblRef::put(int _in) {
	this->__value = _in;
}

void zapata::JSONDblRef::put(long _in) {
	this->__value = _in;
}

void zapata::JSONDblRef::put(long long _in) {
	this->__value = _in;
}

#ifdef __LP64__
void zapata::JSONDblRef::put(unsigned int _in) {
	this->__value = _in;
}
#endif

void zapata::JSONDblRef::put(size_t _in) {
	this->__value = _in;
}

void zapata::JSONDblRef::put(double _in) {
	this->__value = _in;
}

void zapata::JSONDblRef::put(bool _in) {
	this->__value = _in;
}

void zapata::JSONDblRef::put(string _in) {
	zapata::fromstr(_in, &this->__value);
}

void zapata::JSONDblRef::put(JSONObj& _in) {
}

void zapata::JSONDblRef::put(JSONArr& _in) {
}

void zapata::JSONDblRef::put(JSONBool& _in) {
	this->put((bool) *_in.get());
}

void zapata::JSONDblRef::put(JSONInt& _in) {
	this->put((long long) *_in.get());
}

void zapata::JSONDblRef::put(JSONDbl& _in) {
	this->put((double) *_in.get());
}

void zapata::JSONDblRef::put(JSONStr& _in) {
	this->put((string) *_in.get());
}

void zapata::JSONDblRef::put(JSONNil& _in) {
	this->__value = 0;
}

bool zapata::JSONDblRef::compare(JSONElement& _in) {
	return this->type() == _in.type() && this->__value == ((JSONDblRef) _in).__value;
}

zapata::JSONElement& zapata::JSONDblRef::get(size_t _idx) {
	return *this;
}

zapata::JSONElement& zapata::JSONDblRef::get(const char* _idx) {
	return *this;
}

int zapata::JSONDblRef::getInt() {
	return (int) this->__value;
}

long zapata::JSONDblRef::getLong() {
	return (long) this->__value;
}

long zapata::JSONDblRef::getLongLong() {
	return (long long) this->__value;
}

unsigned int zapata::JSONDblRef::getUnsignedInt() {
	return (unsigned int) this->__value;
}

double zapata::JSONDblRef::getDouble() {
	return this->__value;
}

bool zapata::JSONDblRef::getBool() {
	return (bool) this->__value;
}

string zapata::JSONDblRef::getString() {
	string _ret;
	zapata::tostr(_ret, this->__value);
	return _ret;
}

zapata::JSONObjRef& zapata::JSONDblRef::getJSONObj() {
	throw CastException("can not convert from basic type to JSONObj");
}

zapata::JSONArrRef& zapata::JSONDblRef::getJSONArr() {
	throw CastException("can not convert from basic type to JSONArr");
}

void zapata::JSONDblRef::stringify(ostream& _out, short _flags, string _tabs) {
	_out << this->__value << flush;
}

void zapata::JSONDblRef::stringify(string& _out, short _flags, string _tabs) {
	zapata::tostr(_out, this->__value);
}
