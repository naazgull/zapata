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

void zapata::JSONStrRef::put(unsigned int _in) {
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

void zapata::JSONStrRef::stringify(ostream& _out, short _flags, string _tabs) {
#ifdef DEBUG_JSON
	_out << "(" << this << ")";
#endif
	string encoded(this->__value);
	zapata::utf8_encode(encoded);
	_out << "\"" <<encoded << "\"" << flush;
}

void zapata::JSONStrRef::stringify(string& _out, short _flags, string _tabs) {
	ostringstream _ret;
	this->stringify(_ret, _flags, _tabs);
	_ret << flush;
	_out.insert(_out.length(), _ret.str());
	_ret.clear();
}
