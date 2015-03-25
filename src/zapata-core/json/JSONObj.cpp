/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without reiction, including without limitation the rights
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

#include <ostream>
#include <zapata/parsers/JSONParser.h>

namespace zapata {
	JSONPtr undefined;
	JSONPtr nilptr = undefined;
}

zapata::JSONElementT::JSONElementT() : __parent( nullptr ) {
	this->type(zapata::JSNil);
	this->__target.__nil = nullptr;
}

zapata::JSONElementT::JSONElementT(JSONElementT& _element) : __parent( nullptr ) {
	this->type( _element.type());
	switch(this->__target.__type) {
		case zapata::JSObject : {
			if (_element.obj().get() != nullptr) {
				this->__target.__object = _element.obj();
			}
			break;
		}
		case zapata::JSArray : {
			if (_element.arr().get() != nullptr) {
				this->__target.__array = _element.arr();
			}
			break;
		}
		case zapata::JSString : {
			this->__target.__string = make_shared<string>(_element.str());
			break;
		}
		case zapata::JSInteger : {
			this->__target.__integer = _element.intr();
			break;
		}
		case zapata::JSDouble : {
			this->__target.__double = _element.dbl();
			break;
		}
		case zapata::JSBoolean : {
			this->__target.__boolean = _element.bln();
			break;
		}
		case zapata::JSNil : {
			this->__target.__nil = nullptr;
			break;
		}
		case zapata::JSDate : {
			this->__target.__date = _element.date();
			break;
		}
	}
}

zapata::JSONElementT::JSONElementT(JSONPtr& _value) {
	this->type(_value->type());
	switch(this->__target.__type) {
		case zapata::JSObject : {
			if (_value->obj().get() != nullptr) {
				this->__target.__object = _value->obj();
			}
			break;
		}
		case zapata::JSArray : {
			if (_value->arr().get() != nullptr) {
				this->__target.__array = _value->arr();
			}
			break;
		}
		case zapata::JSString : {
			this->__target.__string = make_shared<string>(_value->str());
			break;
		}
		case zapata::JSInteger : {
			this->__target.__integer = _value->intr();
			break;
		}
		case zapata::JSDouble : {
			this->__target.__double = _value->dbl();
			break;
		}
		case zapata::JSBoolean : {
			this->__target.__boolean = _value->bln();
			break;
		}
		case zapata::JSNil : {
			this->__target.__nil = nullptr;
			break;
		}
		case zapata::JSDate : {
			this->__target.__date = _value->date();
			break;
		}
	}
}

zapata::JSONElementT::JSONElementT(JSONObj& _value) : __parent( nullptr ) {
	this->type( zapata::JSObject);
	if (_value.get() != nullptr) {
		this->__target.__object = _value;
	}
}

zapata::JSONElementT::JSONElementT(JSONArr& _value) : __parent( nullptr ) {
	this->type( zapata::JSArray);
	if (_value.get() != nullptr) {
		this->__target.__array = _value;
	}
}

zapata::JSONElementT::JSONElementT(string _value) : __parent( nullptr ) {
	this->type( zapata::JSString);
	this->__target.__string = make_shared<string>(_value);
}

zapata::JSONElementT::JSONElementT(const char* _value) : __parent( nullptr ) {
	this->type( zapata::JSString);
	this->__target.__string = make_shared<string>(string(_value));
}

zapata::JSONElementT::JSONElementT(long long _value) : __parent( nullptr ) {
	this->type( zapata::JSInteger);
	this->__target.__integer = _value;
}

zapata::JSONElementT::JSONElementT(double _value) : __parent( nullptr ) {
	this->type( zapata::JSDouble);
	this->__target.__double = _value;
}

zapata::JSONElementT::JSONElementT(bool _value) : __parent( nullptr ) {
	this->type( zapata::JSBoolean);
	this->__target.__boolean = _value;
}

zapata::JSONElementT::JSONElementT(zapata::mstime_t _value) : __parent( nullptr ) {
	this->type( zapata::JSDate);
	this->__target.__date = _value;
}

zapata::JSONElementT::JSONElementT(int _value) : __parent( nullptr ) {
	this->type( zapata::JSInteger);
	this->__target.__integer = _value;
}

zapata::JSONElementT::JSONElementT(size_t _value) : __parent( nullptr ) {
	this->type( zapata::JSInteger);
	this->__target.__integer = _value;
}

#ifdef __LP64__
zapata::JSONElementT::JSONElementT(unsigned int _value) : __parent( nullptr ) {
	this->type( zapata::JSInteger);
	this->__target.__integer = _value;
}
#endif

zapata::JSONElementT::~JSONElementT() {
}

zapata::JSONType zapata::JSONElementT::type() {
	return (zapata::JSONType)  this->__target.__type;
}

void zapata::JSONElementT::type(JSONType _in) {
	assertz(_in >= 0, "the type must be a valid value", 0, 0);
	
	if (_in == this->__target.__type) {
		return;
	}

	switch(this->__target.__type) {
		case zapata::JSObject : {
			this->__target.__object.~JSONObj();
			//delete & this->__target.__object;
			break;
		}
		case zapata::JSArray : {
			this->__target.__array.~JSONArr();
			//delete & this->__target.__array;
			break;
		}
		case zapata::JSString : {
			this->__target.__string.~JSONStr();
			//delete & this->__target.__string;
			break;
		}
		default : {
			break;
		}
	}
	switch(_in) {
		case zapata::JSObject : {
			new(& this->__target.__object) JSONObj();
			break;
		}
		case zapata::JSArray : {
			new(& this->__target.__array) JSONArr();
			break;
		}
		case zapata::JSString : {
			new(& this->__target.__string) JSONStr();
			break;
		}
		default : {
			break;
		}
		
	}

	this->__target.__type = _in;
}

zapata::JSONUnion& zapata::JSONElementT::value() {
	return this->__target;
}

bool zapata::JSONElementT::ok() {
	return this->__target.__type != zapata::JSNil;
}

bool zapata::JSONElementT::empty() {
	return this->__target.__type == zapata::JSNil;
}

bool zapata::JSONElementT::nil() {
	return this->__target.__type == zapata::JSNil;
}

void zapata::JSONElementT::assign(JSONElementT& _rhs) {
	this->type( _rhs.type());
	switch(this->__target.__type) {
		case zapata::JSObject : {
			if (_rhs.obj().get() != nullptr) {
				this->__target.__object = _rhs.obj();
			}
			break;
		}
		case zapata::JSArray : {
			if (_rhs.arr().get() != nullptr) {
				this->__target.__array = _rhs.arr();
			}
			break;
		}
		case zapata::JSString : {
			this->__target.__string = make_shared<string>(_rhs.str());
			break;
		}
		case zapata::JSInteger : {
			this->__target.__integer = _rhs.intr();
			break;
		}
		case zapata::JSDouble : {
			this->__target.__double = _rhs.dbl();
			break;
		}
		case zapata::JSBoolean : {
			this->__target.__boolean = _rhs.bln();
			break;
		}
		case zapata::JSNil : {
			this->__target.__nil = nullptr;
			break;
		}
		case zapata::JSDate : {
			this->__target.__date = _rhs.date();
			break;
		}
	}
}

zapata::JSONElementT * zapata::JSONElementT::parent() {
	return this->__parent;
}

void zapata::JSONElementT::parent(JSONElementT* _parent) {
	this->__parent = _parent;
}

zapata::JSONObj& zapata::JSONElementT::obj() {
	assertz(this->__target.__type == zapata::JSObject, "this element is not of type JSObject", 0, 0);
	return this->__target.__object;
}

zapata::JSONArr& zapata::JSONElementT::arr() {
	assertz(this->__target.__type == zapata::JSArray, "this element is not of type JSArray", 0, 0);
	return this->__target.__array;
}

string zapata::JSONElementT::str() {
	assertz(this->__target.__type == zapata::JSString, "this element is not of type JSString", 0, 0);
	return *(this->__target.__string.get());
}

long long zapata::JSONElementT::intr() {
	assertz(this->__target.__type == zapata::JSInteger, "this element is not of type JSInteger", 0, 0);
	return this->__target.__integer;
}

double zapata::JSONElementT::dbl() {
	assertz(this->__target.__type == zapata::JSDouble, "this element is not of type JSDouble", 0, 0);
	return this->__target.__double;
}

bool zapata::JSONElementT::bln() {
	assertz(this->__target.__type == zapata::JSBoolean, "this element is not of type JSBoolean", 0, 0);
	return this->__target.__boolean;
}

zapata::mstime_t zapata::JSONElementT::date() {
	assertz(this->__target.__type == zapata::JSDate || this->__target.__type == zapata::JSString, "this element is not of type JSDate", 0, 0);
	if (this->__target.__type == zapata::JSString) {
		time_t _n = 0;
		int _ms = 0;
		string _s(this->__target.__string.get()->data());
		size_t _idx = _s.rfind(".");
		string _mss(_s.substr(_idx + 1));
		_mss.assign(_mss.substr(0, _mss.length() - 1));
		_s.assign(_s.substr(0, _idx));
		zapata::fromstr(_s, &_n, "%Y-%m-%dT%H:%M:%S");
		zapata::fromstr(_mss, &_ms);
		return _n * 1000 + _ms;
	}
	return this->__target.__date;
}

double zapata::JSONElementT::number() {
	assertz(this->__target.__type == zapata::JSInteger || this->__target.__type == zapata::JSDouble || this->__target.__type == zapata::JSBoolean, "this element is not of type JSInteger, JSDouble or JSBoolean", 0, 0);
	switch(this->__target.__type) {
		case zapata::JSInteger : {
			return (double) this->__target.__integer;
		}
		case zapata::JSDouble : {
			return this->__target.__double;
		}
		case zapata::JSBoolean : {
			return (double) this->__target.__boolean;
		}
		default : {
			return 0;
		}
	}
	return 0;
}

zapata::JSONElementT& zapata::JSONElementT::operator<<(const char* _in) {
	(*this) << string(_in);
	return * this;
}

zapata::JSONElementT& zapata::JSONElementT::operator<<(string _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	switch(this->__target.__type) {
		case zapata::JSObject : {
			this->__target.__object->push(_in);
			break;
		}
		case zapata::JSArray : {
			this->__target.__array->push(new zapata::JSONElementT(string(_in)));
			break;
		}
		default : {
			this->__target.__string.get()->assign(_in);
			this->type( zapata::JSString);
			assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
			break;
		}
	}
	return * this;
}

zapata::JSONElementT& zapata::JSONElementT::operator<<(JSONElementT* _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	switch(this->__target.__type) {
		case zapata::JSObject : {
			this->__target.__object->push(_in);
			break;
		}
		case zapata::JSArray : {
			this->__target.__array->push(_in);
			break;
		}
		default : {
			assertz(this->__target.__type == zapata::JSObject || this->__target.__type == zapata::JSArray, "the type must be a JSObject or JSArray in order to push JSONElementT*", 0, 0);
			break;
		}
	}
	return * this;
}

bool zapata::JSONElementT::operator==(zapata::JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	if (this->__target.__type !=  _in.type()) {
		return false;
	}
	switch(this->__target.__type) {
		case zapata::JSObject : {
			return *(this->__target.__object) == *(_in.obj());
		}
		case zapata::JSArray : {
			return *(this->__target.__array) == *(_in.arr());
		}
		case zapata::JSString : {
			return *(this->__target.__string.get()) == _in.str();
		}
		case zapata::JSInteger : {
			return this->__target.__integer == _in.intr();
		}
		case zapata::JSDouble : {
			return this->__target.__double == _in.dbl();
		}
		case zapata::JSBoolean : {
			return this->__target.__boolean == _in.bln();
		}
		case zapata::JSNil : {
			return true;
		}
		case zapata::JSDate : {
			return this->__target.__date == _in.date();
		}
	}
	return false;
}

bool zapata::JSONElementT::operator==(zapata::JSONPtr& _rhs) {
	return * this == * _rhs;
}

bool zapata::JSONElementT::operator!=(JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	if (this->__target.__type != _in.type()) {
		return true;
	}
	switch(this->__target.__type) {
		case zapata::JSObject : {
			return this->__target.__object != _in.obj();
		}
		case zapata::JSArray : {
			return this->__target.__array != _in.arr();
		}
		case zapata::JSString : {
			return *(this->__target.__string.get()) != _in.str();
		}
		case zapata::JSInteger : {
			return this->__target.__integer != _in.intr();
		}
		case zapata::JSDouble : {
			return this->__target.__double != _in.dbl();
		}
		case zapata::JSBoolean : {
			return this->__target.__boolean != _in.bln();
		}
		case zapata::JSNil : {
			return true;
		}
		case zapata::JSDate : {
			return this->__target.__date != _in.date();
		}
	}
	return false;
}

bool zapata::JSONElementT::operator!=(zapata::JSONPtr& _rhs) {
	return * this != * _rhs;
}

void zapata::JSONElementT::stringify(ostream& _out) {
	switch(this->__target.__type) {
		case zapata::JSObject : {
			this->__target.__object->stringify(_out);
			break;
		}
		case zapata::JSArray : {
			this->__target.__array->stringify(_out);
			break;
		}
		case zapata::JSString : {
			string _str(this->str());
			zapata::replace(_str, "\"", "\\\"");
			zapata::replace(_str, "\n", "\\\n");
			zapata::replace(_str, "\r", "\\\b");
			zapata::replace(_str, "\t", "\\\t");
			_out << "\"" << _str << "\"" << flush;
			break;
		}
		case zapata::JSInteger : {
			_out << this->__target.__integer << flush;
			break;
		}
		case zapata::JSDouble : {
			_out << this->__target.__double << flush;
			break;
		}
		case zapata::JSBoolean : {
			_out << (this->__target.__boolean ? "true" : "false") << flush;
			break;
		}
		case zapata::JSNil : {
			_out <<  "undefined" << flush;
			break;
		}
		case zapata::JSDate : {
			string _date;
			zapata::tostr(_date, (size_t) this->__target.__date / 1000, "%Y-%m-%dT%H:%M:%S");
			_date.insert(_date.length(), ".");
			zapata::tostr(_date, (size_t) this->__target.__date % 1000);
			_date.insert(_date.length(), "Z");
			_out << "\"" << _date << "\"" << flush;
			break;
		}
	}
}

void zapata::JSONElementT::stringify(string& _out) {
	switch(this->__target.__type) {
		case zapata::JSObject : {
			this->__target.__object->stringify(_out);
			break;
		}
		case zapata::JSArray : {
			this->__target.__array->stringify(_out);
			break;
		}
		case zapata::JSString : {
			string _str(this->str());
			zapata::replace(_str, "\"", "\\\"");
			zapata::replace(_str, "\n", "\\\n");
			zapata::replace(_str, "\r", "\\\b");
			zapata::replace(_str, "\t", "\\\t");
			_out.insert(_out.length(), "\"");
			_out.insert(_out.length(), _str);
			_out.insert(_out.length(), "\"");
			break;
		}
		case zapata::JSInteger : {
			zapata::tostr(_out, this->__target.__integer);
			break;
		}
		case zapata::JSDouble : {
			zapata::tostr(_out, this->__target.__double);
			break;
		}
		case zapata::JSBoolean : {
			zapata::tostr(_out, this->__target.__boolean);
			break;
		}
		case zapata::JSNil : {
			_out.insert(_out.length(), "undefined");
			break;
		}
		case zapata::JSDate : {
			_out.insert(_out.length(), "\"");
			zapata::tostr(_out, (size_t) this->__target.__date / 1000, "%Y-%m-%dT%H:%M:%S");
			_out.insert(_out.length(), ".");
			zapata::tostr(_out, (size_t) this->__target.__date % 1000);
			_out.insert(_out.length(), "Z");
			_out.insert(_out.length(), "\"");
			break;
		}
	}
}

void zapata::JSONElementT::prettify(ostream& _out, uint _n_tabs) {
	switch(this->__target.__type) {
		case zapata::JSObject : {
			this->__target.__object->prettify(_out, _n_tabs);
			break;
		}
		case zapata::JSArray : {
			this->__target.__array->prettify(_out, _n_tabs);
			break;
		}
		case zapata::JSString : {
			string _str(this->str());
			zapata::replace(_str, "\"", "\\\"");
			zapata::replace(_str, "\n", "\\\n");
			zapata::replace(_str, "\r", "\\\b");
			zapata::replace(_str, "\t", "\\\t");
			_out << "\"" << _str << "\"" << flush;
			break;
		}
		case zapata::JSInteger : {
			_out << this->__target.__integer << flush;
			break;
		}
		case zapata::JSDouble : {
			_out << this->__target.__double << flush;
			break;
		}
		case zapata::JSBoolean : {
			_out << (this->__target.__boolean ? "true" : "false") << flush;
			break;
		}
		case zapata::JSNil : {
			_out <<  "undefined" << flush;
			break;
		}
		case zapata::JSDate : {
			string _date;
			zapata::tostr(_date, (size_t) this->__target.__date / 1000, "%Y-%m-%dT%H:%M:%S");
			_date.insert(_date.length(), ".");
			zapata::tostr(_date, (size_t) this->__target.__date % 1000);
			_date.insert(_date.length(), "Z");
			_out << "\"" << _date << "\"" << flush;
			break;
		}
	}
	if (_n_tabs == 0) {
		_out << endl << flush;
	}
}

void zapata::JSONElementT::prettify(string& _out, uint _n_tabs) {
	switch(this->__target.__type) {
		case zapata::JSObject : {
			this->__target.__object->prettify(_out, _n_tabs);
			break;
		}
		case zapata::JSArray : {
			this->__target.__array->prettify(_out, _n_tabs);
			break;
		}
		case zapata::JSString : {
			string _str(this->str());
			zapata::replace(_str, "\"", "\\\"");
			zapata::replace(_str, "\n", "\\\n");
			zapata::replace(_str, "\r", "\\\b");
			zapata::replace(_str, "\t", "\\\t");
			_out.insert(_out.length(), "\"");
			_out.insert(_out.length(), _str);
			_out.insert(_out.length(), "\"");
			break;
		}
		case zapata::JSInteger : {
			zapata::tostr(_out, this->__target.__integer);
			break;
		}
		case zapata::JSDouble : {
			zapata::tostr(_out, this->__target.__double);
			break;
		}
		case zapata::JSBoolean : {
			zapata::tostr(_out, this->__target.__boolean);
			break;
		}
		case zapata::JSNil : {
			_out.insert(_out.length(), "undefined");
			break;
		}
		case zapata::JSDate : {
			_out.insert(_out.length(), "\"");
			zapata::tostr(_out, (size_t) this->__target.__date / 1000, "%Y-%m-%dT%H:%M:%S");
			_out.insert(_out.length(), ".");
			zapata::tostr(_out, (size_t) this->__target.__date % 1000);
			_out.insert(_out.length(), "Z");
			_out.insert(_out.length(), "\"");
			break;
		}
	}
	if (_n_tabs == 0) {
		_out.insert(_out.length(), "\n");
	}
}


/*JSON OBJECT*/
zapata::JSONObjT::JSONObjT() {
}

zapata::JSONObjT::~JSONObjT(){
}

void zapata::JSONObjT::push(string _name) {
	if (this->__name.length() == 0) {
		this->__name.assign(_name);
	}
	else {
		this->pop(this->__name);
		this->insert(pair<string, JSONPtr>(string(this->__name.data()), JSONPtr(new JSONElementT(_name))));
		this->__name.clear();
	}
}

void zapata::JSONObjT::push(JSONElementT& _value) {
	assertz(this->__name.length() != 0, "you must pass a field name first", 0, 0);
	this->pop(this->__name);
	this->insert(pair<string, JSONPtr>(this->__name, JSONPtr(new JSONElementT(_value))));
	this->__name.clear();
}

void zapata::JSONObjT::push(JSONElementT* _value) {
	assertz(this->__name.length() != 0, "you must pass a field name first", 0, 0);
	this->pop(this->__name);
	this->insert(pair<string, JSONPtr>(this->__name, JSONPtr(_value)));
	this->__name.clear();
}

void zapata::JSONObjT::pop(int _name) {
	string _sname;
	zapata::tostr(_sname, _name);
	this->pop(_sname);
}

void zapata::JSONObjT::pop(size_t _name) {
	string _sname;
	zapata::tostr(_sname, _name);
	this->pop(_sname);
}

void zapata::JSONObjT::pop(const char* _name) {
	this->pop(string(_name));
}

void zapata::JSONObjT::pop(string _name) {
	auto _found = this->find(_name);
	if (_found != this->end()) {
		this->erase(_found);
	}
}

bool zapata::JSONObjT::operator==(zapata::JSONObjT& _rhs) {
	for (auto _f : * this) {
		auto _found = _rhs.find(_f.first);
		if (_found == _rhs.end()) {
			return false;
		}
		if (_found->second == _f.second) {
			continue;
		}
		return false;
	}
	return true;
}

bool zapata::JSONObjT::operator==(zapata::JSONObj& _rhs) {
	return * this == * _rhs;
}

bool zapata::JSONObjT::operator!=(JSONObjT& _rhs) {
	for (auto _f : * this) {
		auto _found = _rhs.find(_f.first);
		if (_found == _rhs.end()) {
			return true;
		}
		if (_found->second != _f.second) {
			return true;
		}
	}
	return false;
}

bool zapata::JSONObjT::operator!=(zapata::JSONObj& _rhs) {
	return * this != * _rhs;
}

zapata::JSONPtr& zapata::JSONObjT::operator[](int _idx) {
	return (* this)[(size_t) _idx];
}

zapata::JSONPtr& zapata::JSONObjT::operator[](size_t _idx) {
	string _sidx;
	zapata::tostr(_sidx, _idx);
	return (* this)[_sidx];
}

zapata::JSONPtr& zapata::JSONObjT::operator[](const char* _idx) {
	return (* this)[string(_idx)];
}

zapata::JSONPtr& zapata::JSONObjT::operator[](string _idx) {
	auto _found = this->find(_idx);
	if (_found != this->end()) {
		return _found->second;
	}
	return zapata::undefined;
}

void zapata::JSONObjT::stringify(string& _out) {
	_out.insert(_out.length(), "{");
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out.insert(_out.length(), ", ");
		}
		_first = false;
		_out.insert(_out.length(), "\"");
		_out.insert(_out.length(), _i.first);
		_out.insert(_out.length(), "\" : ");
		_i.second->stringify(_out);
	}
	_out.insert(_out.length(), "}");
}

void zapata::JSONObjT::stringify(ostream& _out) {
	_out << "{" << flush;
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out << ", ";
		}
		_first = false;
		_out << "\"" << _i.first << "\" : " << flush;
		_i.second->stringify(_out);
	}
	_out << "}" << flush;
}

void zapata::JSONObjT::prettify(string& _out, uint _n_tabs) {
	_out.insert(_out.length(), "{\n");
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out.insert(_out.length(), ",\n");
		}
		_first = false;
		_out.insert(_out.length(), string(_n_tabs + 1, '\t'));
		_out.insert(_out.length(), "\"");
		_out.insert(_out.length(), _i.first);
		_out.insert(_out.length(), "\" : ");
		_i.second->prettify(_out, _n_tabs + 1);
	}
	_out.insert(_out.length(), "\n");
	_out.insert(_out.length(), string(_n_tabs, '\t'));
	_out.insert(_out.length(), "}");
}

void zapata::JSONObjT::prettify(ostream& _out, uint _n_tabs) {
	_out << "{\n" << flush;
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out << ",\n ";
		}
		_first = false;
		_out << string(_n_tabs + 1, '\t') << "\"" << _i.first << "\" : " << flush;
		_i.second->prettify(_out, _n_tabs + 1);
	}
	_out << "\n" << string(_n_tabs, '\t') << "}" << flush;
}

/*JSON ARRAY*/
zapata::JSONArrT::JSONArrT() {
}

zapata::JSONArrT::~JSONArrT(){
}

void zapata::JSONArrT::push(JSONElementT& _value) {
	this->push_back(JSONPtr(new JSONElementT(_value)));
}

void zapata::JSONArrT::push(JSONElementT* _value) {
	this->push_back(JSONPtr(_value));
}

void zapata::JSONArrT::pop(int _idx) {
	this->pop((size_t) _idx);
}

void zapata::JSONArrT::pop(const char* _idx) {
	this->pop(string(_idx));
}

void zapata::JSONArrT::pop(string _idx) {
	size_t _i = 0;
	zapata::fromstr(_idx, &_i);

	assertz(_i >= 0, "the index of the element you want to remove must be higher then 0", 0, 0);
	assertz(_i < this->size(), "the index of the element you want to remove must be lower than the array size", 0, 0);
	this->erase(this->begin() + _i);
}

void zapata::JSONArrT::pop(size_t _idx) {
	assertz(_idx >= 0, "the index of the element you want to remove must be higher then 0", 0, 0);
	assertz(_idx < this->size(), "the index of the element you want to remove must be lower than the array size", 0, 0);
	this->erase(this->begin() + _idx);
}

bool zapata::JSONArrT::operator==(zapata::JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f]  == _rhs[_f]) {
			continue;
		}
		return false;
	}
	return true;
}

bool zapata::JSONArrT::operator==(zapata::JSONArr& _rhs) {
	return * this == * _rhs;
}

bool zapata::JSONArrT::operator!=(JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f]  != _rhs[_f]) {
			return true;
		}
	}
	return false;
}

bool zapata::JSONArrT::operator!=(zapata::JSONArr& _rhs) {
	return * this != * _rhs;
}

zapata::JSONPtr& zapata::JSONArrT::operator[](int _idx) {
	return (* this)[(size_t) _idx];
}

zapata::JSONPtr& zapata::JSONArrT::operator[](size_t _idx) {
	if (_idx < 0 ||_idx >= this->size()) {
		return zapata::undefined;
	}
	return this->at(_idx);
}

zapata::JSONPtr& zapata::JSONArrT::operator[](const char* _idx) {
	return (* this)[string(_idx)];
}

zapata::JSONPtr& zapata::JSONArrT::operator[](string _idx) {
	size_t _i = 0;
	zapata::fromstr(_idx, &_i);

	if (_i < 0 ||_i >= this->size()) {
		return zapata::undefined;
	}

	return this->at(_i);
}

void zapata::JSONArrT::stringify(string& _out) {
	_out.insert(_out.length(), "[");
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out.insert(_out.length(), ", ");
		}
		_first = false;
		_i->stringify(_out);
	}
	_out.insert(_out.length(), "]");
}

void zapata::JSONArrT::stringify(ostream& _out) {
	_out << "[" << flush;
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out << ", ";
		}
		_first = false;
		_i->stringify(_out);
	}
	_out << "]" << flush;
}

void zapata::JSONArrT::prettify(string& _out, uint _n_tabs) {
	_out.insert(_out.length(), "[\n");
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out.insert(_out.length(), ",\n");
		}
		_first = false;
		_out.insert(_out.length(), string(_n_tabs + 1, '\t'));
		_i->prettify(_out, _n_tabs + 1);
	}
	_out.insert(_out.length(), "\n]");
	_out.insert(_out.length(), string(_n_tabs + 1, '\t'));
	_out.insert(_out.length(), "]");
}

void zapata::JSONArrT::prettify(ostream& _out, uint _n_tabs) {
	_out << "[\n" << flush;
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out << ",\n ";
		}
		_first = false;
		_out << string(_n_tabs + 1, '\t')<< flush;
		_i->prettify(_out, _n_tabs + 1);
	}
	_out << "\n" << string(_n_tabs, '\t') << "]" << flush;
}

/*JSON POINTER TO ELEMENT*/
zapata::JSONPtr::JSONPtr()  : shared_ptr<JSONElementT>(make_shared<JSONElementT>()) {
}

zapata::JSONPtr::JSONPtr(JSONElementT* _target) : shared_ptr<JSONElementT>(_target) {
}

zapata::JSONPtr::~JSONPtr(){
}

zapata::JSONElementT& zapata::JSONPtr::value() {
	if (this->get() == nullptr) {
		return *(zapata::undefined.get());
	}
	return *(this->get());
}

zapata::JSONPtr::operator string() {
	if (this->get() == nullptr) {
		return "";
	}
	string _out;
	switch(this->get()->type()) {
		case zapata::JSObject : {
			this->get()->obj()->stringify(_out);
			break;
		}
		case zapata::JSArray : {
			this->get()->arr()->stringify(_out);
			break;
		}
		case zapata::JSString : {
			_out.assign(this->get()->str().data());
			break;
		}
		case zapata::JSInteger : {
			zapata::tostr(_out, this->get()->intr());
			break;
		}
		case zapata::JSDouble : {
			zapata::tostr(_out, this->get()->dbl());
			break;
		}
		case zapata::JSBoolean : {
			zapata::tostr(_out, this->get()->bln());
			break;
		}
		case zapata::JSNil : {
			_out.assign("");
			break;
		}
		case zapata::JSDate : {
			zapata::tostr(_out, (size_t) this->get()->date() / 1000, "%Y-%m-%dT%H:%M:%S");
			_out.insert(_out.length(), ".");
			zapata::tostr(_out, (size_t) this->get()->date() % 1000);
			_out.insert(_out.length(), "Z");
			break;
		}
	}
	return _out;
}

zapata::JSONPtr::operator bool() {
	if (this->get() == nullptr) {
		return false;
	}
	switch(this->get()->type()) {
		case zapata::JSObject : {
			return true;
		}
		case zapata::JSArray : {
			return true;
		}
		case zapata::JSString : {
			return this->get()->str().length() != 0;
		}
		case zapata::JSInteger : {
			return (bool) this->get()->intr();
		}
		case zapata::JSDouble : {
			return (bool) this->get()->dbl();
		}
		case zapata::JSBoolean : {
			return this->get()->bln();
		}
		case zapata::JSNil : {
			return false;
		}
		case zapata::JSDate : {
			return (bool) this->get()->date();
		}
	}
	return false;
}

zapata::JSONPtr::operator int() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zapata::JSObject : {
			return this->get()->obj()->size();
		}
		case zapata::JSArray : {
			return this->get()->arr()->size();
		}
		case zapata::JSString : {
			int _n = 0;
			string _s(this->get()->str().data());
			zapata::fromstr(_s, &_n);
			return _n;
		}
		case zapata::JSInteger : {
			return (int) this->get()->intr();
		}
		case zapata::JSDouble : {
			return (int) this->get()->dbl();
		}
		case zapata::JSBoolean : {
			return (int) this->get()->bln();
		}
		case zapata::JSNil : {
			return 0;
		}
		case zapata::JSDate : {
			return (int) this->get()->date();
		}
	}
	return 0;
}

zapata::JSONPtr::operator long() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zapata::JSObject : {
			return this->get()->obj()->size();
		}
		case zapata::JSArray : {
			return this->get()->arr()->size();
		}
		case zapata::JSString : {
			long _n = 0;
			string _s(this->get()->str().data());
			zapata::fromstr(_s, &_n);
			return _n;
		}
		case zapata::JSInteger : {
			return (long) this->get()->intr();
		}
		case zapata::JSDouble : {
			return (long) this->get()->dbl();
		}
		case zapata::JSBoolean : {
			return (long) this->get()->bln();
		}
		case zapata::JSNil : {
			return 0;
		}
		case zapata::JSDate : {
			return (long) this->get()->date();
		}
	}
	return 0;
}

zapata::JSONPtr::operator long long() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zapata::JSObject : {
			return this->get()->obj()->size();
		}
		case zapata::JSArray : {
			return this->get()->arr()->size();
		}
		case zapata::JSString : {
			long long _n = 0;
			string _s(this->get()->str().data());
			zapata::fromstr(_s, &_n);
			return _n;
		}
		case zapata::JSInteger : {
			return (long long) this->get()->intr();
		}
		case zapata::JSDouble : {
			return (long long) this->get()->dbl();
		}
		case zapata::JSBoolean : {
			return (long long) this->get()->bln();
		}
		case zapata::JSNil : {
			return 0;
		}
		case zapata::JSDate : {
			return (long long) this->get()->date();
		}
	}
	return 0;
}

#ifdef __LP64__
zapata::JSONPtr::operator unsigned int() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zapata::JSObject : {
			return this->get()->obj()->size();
		}
		case zapata::JSArray : {
			return this->get()->arr()->size();
		}
		case zapata::JSString : {
			unsigned int _n = 0;
			string _s(this->get()->str().data());
			zapata::fromstr(_s, &_n);
			return _n;
		}
		case zapata::JSInteger : {
			return (unsigned int) this->get()->intr();
		}
		case zapata::JSDouble : {
			return (unsigned int) this->get()->dbl();
		}
		case zapata::JSBoolean : {
			return (unsigned int) this->get()->bln();
		}
		case zapata::JSNil : {
			return 0;
		}
		case zapata::JSDate : {
			return (unsigned int) this->get()->date();
		}
	}
	return 0;
}
#endif

zapata::JSONPtr::operator size_t() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zapata::JSObject : {
			return this->get()->obj()->size();
		}
		case zapata::JSArray : {
			return this->get()->arr()->size();
		}
		case zapata::JSString : {
			size_t _n = 0;
			string _s(this->get()->str().data());
			zapata::fromstr(_s, &_n);
			return _n;
		}
		case zapata::JSInteger : {
			return (size_t) this->get()->intr();
		}
		case zapata::JSDouble : {
			return (size_t) this->get()->dbl();
		}
		case zapata::JSBoolean : {
			return (size_t) this->get()->bln();
		}
		case zapata::JSNil : {
			return 0;
		}
		case zapata::JSDate : {
			return (size_t) this->get()->date();
		}
	}
	return 0;
}

zapata::JSONPtr::operator double() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zapata::JSObject : {
			return (double) this->get()->obj()->size();
		}
		case zapata::JSArray : {
			return (double) this->get()->arr()->size();
		}
		case zapata::JSString : {
			double _n = 0;
			string _s(this->get()->str().data());
			zapata::fromstr(_s, &_n);
			return _n;
		}
		case zapata::JSInteger : {
			return (double) this->get()->intr();
		}
		case zapata::JSDouble : {
			return (double) this->get()->dbl();
		}
		case zapata::JSBoolean : {
			return (double) this->get()->bln();
		}
		case zapata::JSNil : {
			return 0;
		}
		case zapata::JSDate : {
			return (double) this->get()->date();
		}
	}
	return 0;
}

zapata::JSONPtr::operator zapata::mstime_t() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zapata::JSObject : {
			struct timeval _tp;
			gettimeofday(& _tp, nullptr);
			return _tp.tv_sec * 1000 + _tp.tv_usec / 1000;
		}
		case zapata::JSArray : {
			struct timeval _tp;
			gettimeofday(& _tp, nullptr);
			return _tp.tv_sec * 1000 + _tp.tv_usec / 1000;
		}
		case zapata::JSString : {
			return this->get()->date();
		}
		case zapata::JSInteger : {
			return (zapata::mstime_t) this->get()->intr();
		}
		case zapata::JSDouble : {
			double _intpart;
			double _fracpart = modf(this->get()->dbl(), &_intpart);
			return (((long long) _intpart) * 1000) + _fracpart;
		}
		case zapata::JSBoolean : {
			return (zapata::mstime_t) this->get()->bln();
		}
		case zapata::JSNil : {
			return 0;
		}
		case zapata::JSDate : {
			return this->get()->date();
		}
	}
	return 0;
}

zapata::JSONPtr::operator JSONObj() {
	assertz(this->get() != nullptr && this->get()->type() == zapata::JSObject, "this element is not of type JSObject", 0, 0);
	return this->get()->obj();
}

zapata::JSONPtr::operator JSONArr() {
	assertz(this->get() != nullptr && this->get()->type() == zapata::JSArray, "this element is not of type JSArray", 0, 0);
	return this->get()->arr();
}

zapata::JSONPtr::operator JSONObj&() {
	assertz(this->get() != nullptr && this->get()->type() == zapata::JSObject, "this element is not of type JSObject", 0, 0);
	return this->get()->obj();
}

zapata::JSONPtr::operator JSONArr&() {
	assertz(this->get() != nullptr && this->get()->type() == zapata::JSArray, "this element is not of type JSArray", 0, 0);
	return this->get()->arr();
}

void zapata::JSONPtr::parse(istream& _in) {
	zapata::JSONParser _p;
	_p.switchRoots(* this);
	_p.switchStreams(_in);
	_p.parse();
}

/*JSON POINTER TO OBJECT*/
zapata::JSONObj::JSONObj() : shared_ptr<JSONObjT>(make_shared<JSONObjT>(JSONObjT())) {
}

zapata::JSONObj::JSONObj(JSONObj& _rhs)  : shared_ptr<JSONObjT>(_rhs) {
}

zapata::JSONObj::JSONObj(JSONObjT* _target) : shared_ptr<JSONObjT>(_target) {
}

zapata::JSONObj::~JSONObj(){
}

zapata::JSONObjT::iterator zapata::JSONObj::begin() {
	return (* this)->begin();
}

zapata::JSONObjT::iterator zapata::JSONObj::end() {
	return (* this)->end();
}

zapata::JSONObj::operator string() {
	if (this->get() == nullptr) {
		return "";
	}
	string _out;
	(* this)->stringify(_out);
	return _out;
}

zapata::JSONObj& zapata::JSONObj::operator<<(string _in) {
	(* this)->push(_in);
	return * this;
}

zapata::JSONObj& zapata::JSONObj::operator<<(const char* _in) {
	(* this)->push(_in);
	return * this;
}

/*JSON POINTER TO ARRAY*/
zapata::JSONArr::JSONArr() : shared_ptr<JSONArrT>(make_shared<JSONArrT>(JSONArrT())) {
}

zapata::JSONArr::JSONArr(JSONArr& _rhs)  : shared_ptr<JSONArrT>(_rhs){
}

zapata::JSONArr::JSONArr(JSONArrT* _target) : shared_ptr<JSONArrT>(_target) {
}

zapata::JSONArr::~JSONArr(){
}

zapata::JSONArr::operator string() {
	if (this->get() == nullptr) {
		return "";
	}
	string _out;
	(* this)->stringify(_out);
	return _out;
}

zapata::JSONArrT::iterator zapata::JSONArr::begin() {
	return (* this)->begin();
}

zapata::JSONArrT::iterator zapata::JSONArr::end() {
	return (* this)->end();
}
