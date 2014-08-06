/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <json/JSONObj.h>

#include <iostream>
#include <text/convert.h>
#include <exceptions/CastException.h>
#include <exceptions/NoAttributeNameException.h>

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
