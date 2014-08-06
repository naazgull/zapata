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

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <json/JSONObj.h>

#include <iostream>
#include <text/convert.h>
#include <exceptions/CastException.h>
#include <exceptions/NoAttributeNameException.h>

zapata::JSONBoolRef::JSONBoolRef(bool _in) : __value(_in) {
}

zapata::JSONBoolRef::~JSONBoolRef() {
}

zapata::JSONType zapata::JSONBoolRef::type() {
	return zapata::JSBoolean;
}

void zapata::JSONBoolRef::put(int _in) {
	this->__value = _in;
}

void zapata::JSONBoolRef::put(long _in) {
	this->__value = _in;
}

void zapata::JSONBoolRef::put(long long _in) {
	this->__value = _in;
}

#ifdef __LP64__
void zapata::JSONBoolRef::put(unsigned int _in) {
	this->__value = _in;
}
#endif

void zapata::JSONBoolRef::put(size_t _in) {
	this->__value = _in;
}

void zapata::JSONBoolRef::put(double _in) {
	this->__value = _in;
}

void zapata::JSONBoolRef::put(bool _in) {
	this->__value = _in;
}

void zapata::JSONBoolRef::put(string _in) {
	this->__value = (_in != string(""));
}

void zapata::JSONBoolRef::put(JSONObj& _in) {
}

void zapata::JSONBoolRef::put(JSONArr& _in) {
}

void zapata::JSONBoolRef::put(JSONBool& _in) {
	this->put((bool) *_in.get());
}

void zapata::JSONBoolRef::put(JSONInt& _in) {
	this->put((long long) *_in.get());
}

void zapata::JSONBoolRef::put(JSONDbl& _in) {
	this->put((double) *_in.get());
}

void zapata::JSONBoolRef::put(JSONStr& _in) {
	this->put((string) *_in.get());
}

void zapata::JSONBoolRef::put(JSONNil& _in) {
	this->__value = 0;
}

bool zapata::JSONBoolRef::compare(JSONElement& _in) {
	return this->type() == _in.type() && this->__value == ((JSONBoolRef) _in).__value;
}

zapata::JSONElement& zapata::JSONBoolRef::get(size_t _idx) {
	return *this;
}

zapata::JSONElement& zapata::JSONBoolRef::get(const char* _idx) {
	return *this;
}

int zapata::JSONBoolRef::getInt() {
	return (int) this->__value;
}

long zapata::JSONBoolRef::getLong() {
	return (long) this->__value;
}

long zapata::JSONBoolRef::getLongLong() {
	return (long long) this->__value;
}

unsigned int zapata::JSONBoolRef::getUnsignedInt() {
	return (unsigned int) this->__value;
}

double zapata::JSONBoolRef::getDouble() {
	return (double) this->__value;
}

bool zapata::JSONBoolRef::getBool() {
	return (bool) this->__value;
}

string zapata::JSONBoolRef::getString() {
	string _ret;
	zapata::tostr(_ret, this->__value);
	return _ret;
}

zapata::JSONObjRef& zapata::JSONBoolRef::getJSONObj() {
	throw CastException("can not convert from basic type to JSONObj");
}

zapata::JSONArrRef& zapata::JSONBoolRef::getJSONArr() {
	throw CastException("can not convert from basic type to JSONArr");
}

void zapata::JSONBoolRef::stringify(ostream& _out, short _flags, string _tabs) {
	_out << this->__value << flush;
}

void zapata::JSONBoolRef::stringify(string& _out, short _flags, string _tabs) {
	zapata::tostr(_out, this->__value);
}
