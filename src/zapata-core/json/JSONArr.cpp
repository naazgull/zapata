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
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/NoAttributeNameException.h>

zapata::JSONArrRef::JSONArrRef() {
}

zapata::JSONArrRef::~JSONArrRef() {
	for (JSONArrRef::iterator i = this->begin(); i != this->end(); i++) {
		delete *i;
	}
}

zapata::JSONType zapata::JSONArrRef::type() {
	return zapata::JSArray;
}

void zapata::JSONArrRef::unset(string _in) {
}

void zapata::JSONArrRef::unset(long long _in) {
	if (_in < (long long) this->size()) {
		this->erase(this->begin() + _in);
	}
}

void zapata::JSONArrRef::put(int _in) {
	JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(long _in) {
	JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(long long _in) {
	JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

#ifdef __LP64__
void zapata::JSONArrRef::put(unsigned int _in) {
	JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}
#endif

void zapata::JSONArrRef::put(size_t _in) {
	JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(double _in) {
	JSONDbl* _sp = new JSONDbl(new JSONDblRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(bool _in) {
	JSONBool* _sp = new JSONBool(new JSONBoolRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(string _in) {
	JSONStr* _sp = new JSONStr(new JSONStrRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(JSONObj& _in) {
	JSONObj* _sp = new JSONObj(_in.get());
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(JSONArr& _in) {
	JSONArr* _sp = new JSONArr(_in.get());
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(JSONBool& _in) {
	this->put((bool) *_in.get());
}

void zapata::JSONArrRef::put(JSONInt& _in) {
	this->put((long long) *_in.get());
}

void zapata::JSONArrRef::put(JSONDbl& _in) {
	this->put((double) *_in.get());
}

void zapata::JSONArrRef::put(JSONStr& _in) {
	this->put((string) *_in.get());
}

void zapata::JSONArrRef::put(JSONNil& _in) {
	JSONNil* _sp = new JSONNil(new JSONNilRef());
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

bool zapata::JSONArrRef::compare(JSONElement& _in) {
	return this->type() == _in.type() && this == &_in;
}

zapata::JSONElement& zapata::JSONArrRef::get(size_t _idx) {
	if(_idx < this->size()) {
		return *this->at(_idx)->get();
	}
	return JSON_NIL;
}

zapata::JSONElement& zapata::JSONArrRef::get(const char* _idx) {
	return *this;
}

int zapata::JSONArrRef::getInt() {
	return this->size();
}

long zapata::JSONArrRef::getLong() {
	return this->size();
}

long zapata::JSONArrRef::getLongLong() {
	return this->size();
}

unsigned int zapata::JSONArrRef::getUnsignedInt() {
	return this->size();
}

double zapata::JSONArrRef::getDouble() {
	return this->size();
}

bool zapata::JSONArrRef::getBool() {
	return true;
}

string zapata::JSONArrRef::getString() {
	string _ret;
	this->stringify(_ret, this->__flags);
	return _ret;
}

zapata::JSONObjRef& zapata::JSONArrRef::getJSONObj() {
	throw CastException("can not convert from JSONArr to JSONObj");
}

zapata::JSONArrRef& zapata::JSONArrRef::getJSONArr() {
	return *this;
}

void zapata::JSONArrRef::stringify(ostream& _out, short _flags, string _tabs) {
	string _ret;
	this->stringify(_ret, _flags, _tabs);
	_out << _ret << flush;
}

void zapata::JSONArrRef::stringify(string& _out, short _flags, string _tabs) {
	_out.insert(_out.length(), "[");
	if (_flags & zapata::pretty) {
		_tabs.insert(_tabs.length(), "\t");
	}
	bool first = true;
	for (JSONArrRef::iterator i = this->begin(); i != this->end(); i++) {
		if (!first) {
			_out.insert(_out.length(), ",");
		}
		first = false;
		if (_flags & zapata::pretty) {
			_out.insert(_out.length(), "\n");
			_out.insert(_out.length(), _tabs);
		}
		(*i)->get()->stringify(_out, _flags, _tabs);
	}
	if (_flags & zapata::pretty) {
		_out.insert(_out.length(), "\n");
		_tabs .erase(_tabs.length() - 1, 1);
	}
	_out.insert(_out.length(),  _tabs);
	_out.insert(_out.length(),  "]");
}

zapata::JSONElement& zapata::JSONArrRef::operator[](int _idx) {
	return this->get(_idx);
}

zapata::JSONElement& zapata::JSONArrRef::operator[](size_t _idx) {
	return this->get(_idx);
}
