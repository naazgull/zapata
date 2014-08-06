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
#include <exceptions/CastException.h>
#include <exceptions/NoAttributeNameException.h>

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
