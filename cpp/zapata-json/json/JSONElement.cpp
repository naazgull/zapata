#include <json/JSONObj.h>

#include <iostream>
#include <exceptions/CastException.h>
#include <exceptions/NoAttributeNameException.h>

namespace zapata {
	JSONElement& nil = *(new zapata::JSONElement());
}

zapata::JSONElement::JSONElement() {
	this->__flags = 0;
}

zapata::JSONElement::~JSONElement() {
}

zapata::JSONType zapata::JSONElement::type() {
	return zapata::JSNil;
}

void zapata::JSONElement::put(int _in) {
}

void zapata::JSONElement::put(long _in) {
}

void zapata::JSONElement::put(long long _in) {
}

void zapata::JSONElement::put(double _in) {
}

void zapata::JSONElement::put(bool _in) {
}

void zapata::JSONElement::put(string _in) {
}

void zapata::JSONElement::put(ObjectOp _in) {
	if (_in == zapata::undefined) {
		zapata::JSONNil _in;
		this->put(_in);
	}
	this->__flags |= _in;
}

void zapata::JSONElement::put(JSONObj& _in) {
}

void zapata::JSONElement::put(JSONArr& _in) {
}

void zapata::JSONElement::put(JSONBool& _in) {
}

void zapata::JSONElement::put(JSONInt& _in) {
}

void zapata::JSONElement::put(JSONDbl& _in) {
}

void zapata::JSONElement::put(JSONStr& _in) {
}

void zapata::JSONElement::put(JSONNil& _in) {
}

bool zapata::JSONElement::compare(JSONElement& _in) {
	return _in.type() == zapata::JSNil;
}

zapata::JSONElement& zapata::JSONElement::get(size_t _idx) {
	return *this;
}

zapata::JSONElement& zapata::JSONElement::get(const char* _idx) {
	return *this;
}

int zapata::JSONElement::getInt() {
	return 0;
}

long zapata::JSONElement::getLong() {
	return 0;
}

long zapata::JSONElement::getLongLong() {
	return 0;
}

double zapata::JSONElement::getDouble() {
	return 0;
}

bool zapata::JSONElement::getBool() {
	return false;
}

string zapata::JSONElement::getString() {
	return "";
}

zapata::JSONElement& zapata::JSONElement::getJSONElement() {
	return *this;
}

void zapata::JSONElement::stringify(ostream& _out, short _flags, string _tabs) {
#ifdef DEBUG_JSON
	_out << "(" << this << ")";
#endif
	_out << _tabs << "null" << flush;
}

void zapata::JSONElement::stringify(string& _out, short _flags, string _tabs) {
	ostringstream _ret;
	this->stringify(_ret, _flags, _tabs);
	_ret << flush;
	_out.insert(_out.length(), _ret.str());
	_ret.clear();
}

bool zapata::JSONElement::operator==(JSONElement& _in) {
	return this->compare(_in);
}

bool zapata::JSONElement::operator==(string _in) {
	return ((string) *this) == _in;
}

bool zapata::JSONElement::operator==(bool _in) {
	return ((bool) *this) == _in;
}

bool zapata::JSONElement::operator==(int _in) {
	return ((int) *this) == _in;
}

bool zapata::JSONElement::operator==(long _in) {
	return ((long) *this) == _in;
}

bool zapata::JSONElement::operator==(long long _in) {
	return ((long long) *this) == _in;
}

bool zapata::JSONElement::operator==(double _in) {
	return ((double) *this) == _in;
}

bool zapata::JSONElement::operator!=(JSONElement& _in) {
	return !(*this == _in);
}

bool zapata::JSONElement::operator!=(string _in) {
	return ((string) *this) != _in;
}

bool zapata::JSONElement::operator!=(bool _in) {
	return ((bool) *this) != _in;
}

bool zapata::JSONElement::operator!=(int _in) {
	return ((int) *this) != _in;
}

bool zapata::JSONElement::operator!=(long _in) {
	return ((long) *this) != _in;
}

bool zapata::JSONElement::operator!=(long long _in) {
	return ((long long) *this) != _in;
}

bool zapata::JSONElement::operator!=(double _in) {
	return ((double) *this) != _in;
}

zapata::JSONElement::operator string() {
	return this->getString();
}

zapata::JSONElement::operator bool() {
	return this->getBool();
}

zapata::JSONElement::operator int() {
	return this->getInt();
}

zapata::JSONElement::operator long() {
	return this->getLong();
}

zapata::JSONElement::operator long long() {
	return this->getLongLong();
}

zapata::JSONElement::operator double() {
	return this->getLongLong();
}

zapata::JSONElement::operator zapata::JSONElement&() {
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(const char* _in) {
	this->put(string(_in));
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(string _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(bool _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(int _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(long _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(long long _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(double _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(zapata::ObjectOp _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(JSONObj& _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(JSONArr& _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(JSONInt& _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(JSONBool& _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(JSONDbl& _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(JSONStr& _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator<<(JSONNil& _in) {
	this->put(_in);
	return *this;
}

zapata::JSONElement& zapata::JSONElement::operator[](size_t _idx) {
	return this->get(_idx);
}

zapata::JSONElement& zapata::JSONElement::operator[](const char* _idx) {
	return this->get(_idx);
}
