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

#include <zapata/json/JSONObj.h>

#include <iostream>
#include <zapata/text/convert.h>
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/NoAttributeNameException.h>

zapata::JSONStrRef::JSONStrRef(string _in) : __value(_in) {
}

zapata::JSONStrRef::~JSONStrRef() {
}

zapata::JSONType zapata::JSONStrRef::type() {
	return zapata::JSString;
}

void zapata::JSONStrRef::put(int _in) {
	zapata::tostr(this->__value, _in);
}

void zapata::JSONStrRef::put(long _in) {
	zapata::tostr(this->__value, _in);
}

void zapata::JSONStrRef::put(long long _in) {
	zapata::tostr(this->__value, _in);
}

#ifdef __LP64__
void zapata::JSONStrRef::put(unsigned int _in) {
	zapata::tostr(this->__value, _in);
}
#endif

void zapata::JSONStrRef::put(size_t _in) {
	zapata::tostr(this->__value, _in);
}

void zapata::JSONStrRef::put(double _in) {
	zapata::tostr(this->__value, _in);
}

void zapata::JSONStrRef::put(bool _in) {
	zapata::tostr(this->__value, _in);
}

void zapata::JSONStrRef::put(string _in) {
	this->__value.assign(_in);
}

void zapata::JSONStrRef::put(JSONObj& _in) {
	_in.get()->stringify(this->__value);
}

void zapata::JSONStrRef::put(JSONArr& _in) {
	_in.get()->stringify(this->__value);
}

void zapata::JSONStrRef::put(JSONBool& _in) {
	this->put((bool) *_in.get());
}

void zapata::JSONStrRef::put(JSONInt& _in) {
	this->put((long long) *_in.get());
}

void zapata::JSONStrRef::put(JSONDbl& _in) {
	this->put((double) *_in.get());
}

void zapata::JSONStrRef::put(JSONStr& _in) {
	this->put((string) *_in.get());
}

void zapata::JSONStrRef::put(JSONNil& _in) {
	this->__value.assign("null");
}

bool zapata::JSONStrRef::compare(JSONElement& _in) {
	return this->type() == _in.type() && this->__value == ((JSONStrRef) _in).__value;
}

zapata::JSONElement& zapata::JSONStrRef::get(size_t _idx) {
	return *this;
}

zapata::JSONElement& zapata::JSONStrRef::get(const char* _idx) {
	return *this;
}

int zapata::JSONStrRef::getInt() {
	int _ret = 0;
	zapata::fromstr(this->__value, &_ret);
	return _ret;
}

long zapata::JSONStrRef::getLong() {
	long _ret = 0;
	zapata::fromstr(this->__value, &_ret);
	return _ret;
}

long zapata::JSONStrRef::getLongLong() {
	long long _ret = 0;
	zapata::fromstr(this->__value, &_ret);
	return _ret;
}

unsigned int zapata::JSONStrRef::getUnsignedInt() {
	unsigned int _ret = 0;
	zapata::fromstr(this->__value, &_ret);
	return _ret;
}

double zapata::JSONStrRef::getDouble() {
	double _ret = 0;
	zapata::fromstr(this->__value, &_ret);
	return _ret;
}

bool zapata::JSONStrRef::getBool() {
	return this->__value.length() != 0;
}

string zapata::JSONStrRef::getString() {
	return this->__value;
}

zapata::JSONObjRef& zapata::JSONStrRef::getJSONObj() {
	throw CastException("can not convert from basic type to JSONObj");
}

zapata::JSONArrRef& zapata::JSONStrRef::getJSONArr() {
	throw CastException("can not convert from basic type to JSONArr");
}

void zapata::JSONStrRef::stringify(ostream& _out, short _flags, string _tabs) {
	string encoded(this->__value);
	zapata::utf8_encode(encoded);
	_out << "\"" << encoded << "\"" << flush;
}

void zapata::JSONStrRef::stringify(string& _out, short _flags, string _tabs) {
	string encoded(this->__value);
	zapata::utf8_encode(encoded);
	_out.insert(_out.length(),  "\"");
	_out.insert(_out.length(), encoded);
	_out.insert(_out.length(), "\"");
}
