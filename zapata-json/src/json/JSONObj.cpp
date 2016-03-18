/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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
#include <zapata/json/JSONParser.h>
#include <regex.h>

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

zapata::JSONElementT::JSONElementT(zapata::timestamp_t _value) : __parent( nullptr ) {
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

string zapata::JSONElementT::demangle() {
	switch(this->__target.__type) {
		case zapata::JSObject : {
			return "object";
		}
		case zapata::JSArray : {
			return "array";
		}
		case zapata::JSString : {
			return "string";
		}
		case zapata::JSInteger : {
			return "integer";
		}
		case zapata::JSDouble : {
			return "number";
		}
		case zapata::JSBoolean : {
			return "boolean";
		}
		case zapata::JSNil : {
			return "null";
		}
		case zapata::JSDate : {
			return "date";
		}
	}
	return "undefined";
}

void zapata::JSONElementT::type(JSONType _in) {
	assertz(_in >= 0, "the type must be a valid value", 0, 0);
	
	if (_in == this->__target.__type) {
		switch(this->__target.__type) {
			case zapata::JSObject : {
				if (this->__target.__object.get() == nullptr) {
					new(& this->__target.__object) JSONObj();
				}
				break;
			}
			case zapata::JSArray : {
				if (this->__target.__array.get() == nullptr) {
					new(& this->__target.__array) JSONArr();
				}
				break;
			}
			case zapata::JSString : {
				if (this->__target.__string.get() == nullptr) {
					new(& this->__target.__string) JSONStr();
				}
				break;
			}
			default : {
				break;
			}

		}
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
	assertz(this->__target.__type == zapata::JSObject, string("this element is not of type JSObject: ") + this->stringify(), 0, 0);
	return this->__target.__object;
}

zapata::JSONArr& zapata::JSONElementT::arr() {
	assertz(this->__target.__type == zapata::JSArray, string("this element is not of type JSArray: ") + this->stringify(), 0, 0);
	return this->__target.__array;
}

string zapata::JSONElementT::str() {
	assertz(this->__target.__type == zapata::JSString, string("this element is not of type JSString: ") + this->stringify(), 0, 0);
	return *(this->__target.__string.get());
}

long long zapata::JSONElementT::intr() {
	assertz(this->__target.__type == zapata::JSInteger, string("this element is not of type JSInteger: ") + this->stringify(), 0, 0);
	return this->__target.__integer;
}

double zapata::JSONElementT::dbl() {
	assertz(this->__target.__type == zapata::JSDouble, string("this element is not of type JSDouble: ") + this->stringify(), 0, 0);
	return this->__target.__double;
}

bool zapata::JSONElementT::bln() {
	assertz(this->__target.__type == zapata::JSBoolean, string("this element is not of type JSBoolean: ") + this->stringify(), 0, 0);
	return this->__target.__boolean;
}

zapata::timestamp_t zapata::JSONElementT::date() {
	assertz(this->__target.__type == zapata::JSDate || this->__target.__type == zapata::JSString, string("this element is not of type JSDate: ") + this->stringify(), 0, 0);
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
	assertz(this->__target.__type == zapata::JSDate || this->__target.__type == zapata::JSInteger || this->__target.__type == zapata::JSDouble || this->__target.__type == zapata::JSBoolean, string("this element is not of type JSInteger, JSDouble or JSBoolean: ") + this->stringify(), 0, 0);
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
		case zapata::JSDate : {
			return (double) this->__target.__date;
		}
		default : {
			return 0;
		}
	}
	return 0;
}

zapata::JSONPtr zapata::JSONElementT::clone() {
	switch(this->type()) {
		case zapata::JSObject : {
			return this->obj()->clone();
		}
		case zapata::JSArray : {
			return this->arr()->clone();
		}
		case zapata::JSString : {
			std::string _v = this->str();
			return make_ptr(_v);
		}
		case zapata::JSInteger : {
			int _v = this->intr();
			return make_ptr(_v);
		}
		case zapata::JSDouble : {
			double _v = this->dbl();
			return make_ptr(_v);
		}
		case zapata::JSBoolean : {
			bool _v = this->bln();
			return make_ptr(_v);
		}
		case zapata::JSNil : {
			return zapata::undefined;
		}
		case zapata::JSDate : {
			zapata::timestamp_t _v = this->date();
			return make_ptr(_v);
		}
	}
	return zapata::undefined;	
}

zapata::JSONElementT& zapata::JSONElementT::operator<<(const char* _in) {
	(* this) << string(_in);
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
			this->type(zapata::JSString);
			this->__target.__string.get()->assign(_in);
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
	switch(this->__target.__type) {
		case zapata::JSObject : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__object) == *(_in.obj());
		}
		case zapata::JSArray : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__array) == *(_in.arr());
		}
		case zapata::JSString : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__string.get()) == _in.str();
		}
		case zapata::JSInteger : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__integer == _in.number();
		}
		case zapata::JSDouble : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__double == _in.number();
		}
		case zapata::JSBoolean : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__boolean == _in.number();
		}
		case zapata::JSNil : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return true;
		}
		case zapata::JSDate : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__date == _in.number();
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

bool zapata::JSONElementT::operator<(zapata::JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	switch(this->__target.__type) {
		case zapata::JSObject : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__object) < *(_in.obj());
		}
		case zapata::JSArray : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__array) < *(_in.arr());
		}
		case zapata::JSString : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__string.get()) < _in.str();
		}
		case zapata::JSInteger : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__integer < _in.number();
		}
		case zapata::JSDouble : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__double < _in.number();
		}
		case zapata::JSBoolean : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__boolean < _in.number();
		}
		case zapata::JSNil : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return true;
		}
		case zapata::JSDate : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__date < _in.number();
		}
	}
	return false;
}

bool zapata::JSONElementT::operator<(zapata::JSONPtr& _rhs) {
	return * this < * _rhs;
}

bool zapata::JSONElementT::operator>(zapata::JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	switch(this->__target.__type) {
		case zapata::JSObject : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__object) > *(_in.obj());
		}
		case zapata::JSArray : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__array) > *(_in.arr());
		}
		case zapata::JSString : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__string.get()) > _in.str();
		}
		case zapata::JSInteger : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__integer > _in.number();
		}
		case zapata::JSDouble : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__double > _in.number();
		}
		case zapata::JSBoolean : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__boolean > _in.number();
		}
		case zapata::JSNil : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return true;
		}
		case zapata::JSDate : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__date > _in.number();
		}
	}
	return false;
}

bool zapata::JSONElementT::operator>(zapata::JSONPtr& _rhs) {
	return * this > * _rhs;
}

bool zapata::JSONElementT::operator<=(zapata::JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	switch(this->__target.__type) {
		case zapata::JSObject : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__object) <= *(_in.obj());
		}
		case zapata::JSArray : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__array) <= *(_in.arr());
		}
		case zapata::JSString : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__string.get()) <= _in.str();
		}
		case zapata::JSInteger : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__integer <= _in.number();
		}
		case zapata::JSDouble : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__double <= _in.number();
		}
		case zapata::JSBoolean : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__boolean <= _in.number();
		}
		case zapata::JSNil : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return true;
		}
		case zapata::JSDate : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__date <= _in.number();
		}
	}
	return false;
}

bool zapata::JSONElementT::operator<=(zapata::JSONPtr& _rhs) {
	return * this <= * _rhs;
}

bool zapata::JSONElementT::operator>=(zapata::JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	switch(this->__target.__type) {
		case zapata::JSObject : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__object) >= *(_in.obj());
		}
		case zapata::JSArray : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__array) >= *(_in.arr());
		}
		case zapata::JSString : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__string.get()) >= _in.str();
		}
		case zapata::JSInteger : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__integer >= _in.number();
		}
		case zapata::JSDouble : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__double >= _in.number();
		}
		case zapata::JSBoolean : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__boolean >= _in.number();
		}
		case zapata::JSNil : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return true;
		}
		case zapata::JSDate : {
			if (_in.type() != zapata::JSDate && _in.type() != zapata::JSInteger && _in.type() != zapata::JSDouble && _in.type() != zapata::JSBoolean) {
				return false;
			}
			return this->__target.__date >= _in.number();
		}
	}
	return false;
}

bool zapata::JSONElementT::operator>=(zapata::JSONPtr& _rhs) {
	return * this >= * _rhs;
}

zapata::JSONPtr zapata::JSONElementT::operator+(zapata::JSONPtr _rhs) {
	return (* this) + (* _rhs);
}

zapata::JSONPtr zapata::JSONElementT::operator+(zapata::JSONElementT& _rhs) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	assertz(this->__target.__type == _rhs.__target.__type, "can't add JSON objects of different types", 0, 0);
	switch(this->__target.__type) {
		case zapata::JSObject : {
			zapata::JSONPtr _lhs = this->clone();
			for (auto _e : _rhs.obj()) {
				_lhs << _e.first  << _e.second;
			}
			return _lhs;
		}
		case zapata::JSArray : {
			zapata::JSONPtr _lhs = this->clone();
			for (auto _e : _rhs.arr()) {
				_lhs << _e;
			}
			return _lhs;
		}
		case zapata::JSString : {
			std::string _lhs((*(this->__target.__string.get())) + _rhs.str());
			return zapata::make_ptr(_lhs);
		}
		case zapata::JSInteger : {
			int _lhs = this->__target.__integer + _rhs.intr();
			return zapata::make_ptr(_lhs);
		}
		case zapata::JSDouble : {
			double _lhs = this->__target.__double + _rhs.dbl();
			return zapata::make_ptr(_lhs);
		}
		case zapata::JSBoolean : {
			bool _lhs = this->__target.__boolean || _rhs.bln();
			return zapata::make_ptr(_lhs);
		}
		case zapata::JSNil : {
			return zapata::undefined;
		}
		case zapata::JSDate : {
			int _lhs = this->__target.__date + _rhs.number();
			return zapata::make_ptr((zapata::timestamp_t) _lhs);
		}
	}
	return zapata::undefined;
}

zapata::JSONPtr zapata::JSONElementT::getPath(std::string _path, std::string _separator) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	switch(this->__target.__type) {
		case zapata::JSObject : {
			return this->__target.__object->getPath(_path, _separator);
		}
		case zapata::JSArray : {
			return this->__target.__array->getPath(_path, _separator);
		}
		case zapata::JSString :
		case zapata::JSInteger :
		case zapata::JSDouble :
		case zapata::JSBoolean :
		case zapata::JSNil :
		case zapata::JSDate : {
			return zapata::undefined;
		}
	}
	return zapata::undefined;
}

void zapata::JSONElementT::setPath(std::string _path, zapata::JSONPtr _value, std::string _separator) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	switch(this->__target.__type) {
		case zapata::JSObject : {
			return this->__target.__object->setPath(_path, _value, _separator);
		}
		case zapata::JSArray : {
			return this->__target.__array->setPath(_path, _value, _separator);
		}
		case zapata::JSString :
		case zapata::JSInteger :
		case zapata::JSDouble :
		case zapata::JSBoolean :
		case zapata::JSNil :
		case zapata::JSDate : {
			return;
		}
	}
	return;
}

void zapata::JSONElementT::delPath(std::string _path, std::string _separator) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
	switch(this->__target.__type) {
		case zapata::JSObject : {
			this->__target.__object->delPath(_path, _separator);
		}
		case zapata::JSArray : {
			this->__target.__array->delPath(_path, _separator);
		}
		case zapata::JSString :
		case zapata::JSInteger :
		case zapata::JSDouble :
		case zapata::JSBoolean :
		case zapata::JSNil :
		case zapata::JSDate : {
			return;
		}
	}
}

void zapata::JSONElementT::inspect(zapata::JSONPtr _pattern, std::function< void (zapata::JSONElementT *, std::string, zapata::JSONElementT *, zapata::JSONPtr) > _callback, std::string _key, zapata::JSONElementT * _parent) {
	switch(this->type()) {
		case zapata::JSObject: {
			for (auto _o : this->obj()) {
				if (_pattern->type() == zapata::JSObject && _pattern[_o.first]->ok()) {
					_o.second->inspect(_pattern[_o.first], _callback, _o.first, this);
					continue;
				}
				_o.second->inspect(_pattern, _callback, _o.first, this);
			}
			break;
		}
		case zapata::JSArray: {
			for (size_t _i = 0; _i != this->arr()->size(); _i++) {
				this->arr()[_i]->inspect(_pattern, _callback, std::to_string(_i), this);
			}
			break;
		}
		default: {
			if (_pattern["$regexp"]->ok()) {
				::regex_t * _rgx = new ::regex_t();
				if (regcomp(_rgx, ((string) _pattern["$regexp"]).c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
					std::string _exp;
					this->stringify(_exp);
					if (regexec(_rgx, _exp.c_str(), (size_t) (0), nullptr, 0) == 0) {
						_callback(this, _key, _parent, _pattern);
					}
				}
				regfree(_rgx);
				delete _rgx;				
			}
			else {
				if (* this == _pattern) {
					_callback(this, _key, _parent, _pattern);
				}
			}
			break;
		}
	}	
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
			zapata::replace(_str, "\\", "\\\\");
			zapata::replace(_str, "\"", "\\\"");
			zapata::replace(_str, "\n", "\\n");
			zapata::replace(_str, "\r", "\\b");
			zapata::replace(_str, "\t", "\\t");
			_out << "\"" << _str << "\"" << flush;
			break;
		}
		case zapata::JSInteger : {
			_out << this->__target.__integer << flush;
			break;
		}
		case zapata::JSDouble : {
			_out << fixed << this->__target.__double << flush;
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
			size_t _remainder = this->__target.__date % 1000;
			if (_remainder < 100) {
				_date.insert(_date.length(), "0");
				if (_remainder < 10) {
					_date.insert(_date.length(), "0");
				}
			}
			zapata::tostr(_date, _remainder);
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
			zapata::replace(_str, "\\", "\\\\");
			zapata::replace(_str, "\"", "\\\"");
			zapata::replace(_str, "\n", "\\n");
			zapata::replace(_str, "\r", "\\b");
			zapata::replace(_str, "\t", "\\t");
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
			size_t _remainder = this->__target.__date % 1000;
			if (_remainder < 100) {
				_out.insert(_out.length(), "0");
				if (_remainder < 10) {
					_out.insert(_out.length(), "0");
				}
			}
			zapata::tostr(_out, _remainder);
			_out.insert(_out.length(), "Z");
			_out.insert(_out.length(), "\"");
			break;
		}
	}
}

std::string zapata::JSONElementT::stringify() {
	string _out;
	this->stringify(_out);
	return _out;
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
			zapata::replace(_str, "\\", "\\\\");
			zapata::replace(_str, "\"", "\\\"");
			zapata::replace(_str, "\n", "\\n");
			zapata::replace(_str, "\r", "\\b");
			zapata::replace(_str, "\t", "\\t");
			_out << "\"" << _str << "\"" << flush;
			break;
		}
		case zapata::JSInteger : {
			_out << this->__target.__integer << flush;
			break;
		}
		case zapata::JSDouble : {
			_out << fixed << this->__target.__double << flush;
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
			size_t _remainder = this->__target.__date % 1000;
			if (_remainder < 100) {
				_date.insert(_date.length(), "0");
				if (_remainder < 10) {
					_date.insert(_date.length(), "0");
				}
			}
			zapata::tostr(_date, _remainder);
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
			zapata::replace(_str, "\\", "\\\\");
			zapata::replace(_str, "\"", "\\\"");
			zapata::replace(_str, "\n", "\\n");
			zapata::replace(_str, "\r", "\\b");
			zapata::replace(_str, "\t", "\\t");
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
			size_t _remainder = this->__target.__date % 1000;
			if (_remainder < 100) {
				_out.insert(_out.length(), "0");
				if (_remainder < 10) {
					_out.insert(_out.length(), "0");
				}
			}
			zapata::tostr(_out, _remainder);
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

zapata::JSONPtr zapata::JSONObjT::getPath(std::string _path, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;

	getline(_iss, _part, _separator[0]);
	zapata::JSONPtr _current = (* this)[_part];
	if (!_current->ok()) {
		return zapata::undefined;
	}
	
	while(_iss.good()) {
		getline(_iss, _part, _separator[0]);
		if (_current[_part]->ok()) {
			_current = _current[_part];
		}
		else {
			return zapata::undefined;
		}
	}
	return _current;
}

void zapata::JSONObjT::setPath(std::string _path, zapata::JSONPtr _value, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;

	getline(_iss, _part, _separator[0]);
	zapata::JSONPtr _current = (* this)[_part];
	if (!_current->ok()) {
		if (_iss.good()) {
			zapata::JSONObj _new;
			_current = make_ptr(_new);
			this->insert(pair<string, JSONPtr>(string(_part.data()), _current));
			_current->setPath(_path.substr(_part.length() + 1), _value, _separator);
		}
		else {
			this->insert(pair<string, JSONPtr>(string(_part.data()), _value));
		}
	}
	else {
		if (_iss.good()) {
			_current->setPath(_path.substr(_part.length() + 1), _value, _separator);
		}
		else {
			_current >> _part;
			_current << _part << _value;			
		}
	}
}

void zapata::JSONObjT::delPath(std::string _path, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;

	getline(_iss, _part, _separator[0]);
	zapata::JSONPtr _current = (* this)[_part];
	if (!_current->ok()) {
		return;
	}

	while(_iss.good()) {
		getline(_iss, _part, _separator[0]);
		if (_current[_part]->ok()) {
			if (_iss.good()) {
				_current = _current[_part];
			}
			else {
				_current >> _part;
			}
		}
		else {
			return;
		}
	}
}

zapata::JSONPtr zapata::JSONObjT::clone() {
	zapata::JSONObj _return;
	for (auto _f : * this) {
		_return << _f.first << _f.second->clone();
	}	
	return make_ptr(_return);
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

bool zapata::JSONObjT::operator<(zapata::JSONObjT& _rhs) {
	for (auto _f : * this) {
		auto _found = _rhs.find(_f.first);
		if (_found == _rhs.end()) {
			return false;
		}
		if (_found->second < _f.second) {
			continue;
		}
		return false;
	}
	return true;
}

bool zapata::JSONObjT::operator<(zapata::JSONObj& _rhs) {
	return * this < * _rhs;
}

bool zapata::JSONObjT::operator>(zapata::JSONObjT& _rhs) {
	for (auto _f : * this) {
		auto _found = _rhs.find(_f.first);
		if (_found == _rhs.end()) {
			return false;
		}
		if (_found->second > _f.second) {
			continue;
		}
		return false;
	}
	return true;
}

bool zapata::JSONObjT::operator>(zapata::JSONObj& _rhs) {
	return * this > * _rhs;
}

bool zapata::JSONObjT::operator<=(zapata::JSONObjT& _rhs) {
	for (auto _f : * this) {
		auto _found = _rhs.find(_f.first);
		if (_found == _rhs.end()) {
			return false;
		}
		if (_found->second <= _f.second) {
			continue;
		}
		return false;
	}
	return true;
}

bool zapata::JSONObjT::operator<=(zapata::JSONObj& _rhs) {
	return * this <= * _rhs;
}

bool zapata::JSONObjT::operator>=(zapata::JSONObjT& _rhs) {
	for (auto _f : * this) {
		auto _found = _rhs.find(_f.first);
		if (_found == _rhs.end()) {
			return false;
		}
		if (_found->second >= _f.second) {
			continue;
		}
		return false;
	}
	return true;
}

bool zapata::JSONObjT::operator>=(zapata::JSONObj& _rhs) {
	return * this >= * _rhs;
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
			_out.insert(_out.length(), ",");
		}
		_first = false;
		_out.insert(_out.length(), "\"");
		_out.insert(_out.length(), _i.first);
		_out.insert(_out.length(), "\":");
		_i.second->stringify(_out);
	}
	_out.insert(_out.length(), "}");
}

void zapata::JSONObjT::stringify(ostream& _out) {
	_out << "{" << flush;
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out << ",";
		}
		_first = false;
		_out << "\"" << _i.first << "\":" << flush;
		_i.second->stringify(_out);
	}
	_out << "}" << flush;
}

void zapata::JSONObjT::prettify(string& _out, uint _n_tabs) {
	_out.insert(_out.length(), "{");
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out.insert(_out.length(), ",");
		}
		_out.insert(_out.length(), "\n");
		_first = false;
		_out.insert(_out.length(), string(_n_tabs + 1, '\t'));
		_out.insert(_out.length(), "\"");
		_out.insert(_out.length(), _i.first);
		_out.insert(_out.length(), "\" : ");
		_i.second->prettify(_out, _n_tabs + 1);
	}
	if (!_first) {
		_out.insert(_out.length(), "\n");
		_out.insert(_out.length(), string(_n_tabs, '\t'));
	}
	_out.insert(_out.length(), "}");
}

void zapata::JSONObjT::prettify(ostream& _out, uint _n_tabs) {
	_out << "{" << flush;
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out << ",";
		}
		_out << "\n ";
		_first = false;
		_out << string(_n_tabs + 1, '\t') << "\"" << _i.first << "\" : " << flush;
		_i.second->prettify(_out, _n_tabs + 1);
	}
	if (!_first) {		
		_out << "\n" << string(_n_tabs, '\t') << flush;
	}
	_out << "}" << flush;
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

zapata::JSONPtr zapata::JSONArrT::getPath(std::string _path, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;

	getline(_iss, _part, _separator[0]);
	zapata::JSONPtr _current = (* this)[_part];
	if (!_current->ok()) {
		return zapata::undefined;
	}

	while(_iss.good()) {
		getline(_iss, _part, _separator[0]);
		if (_current[_part]->ok()) {
			_current = _current[_part];
		}
		else {
			return zapata::undefined;
		}
	}
	return _current;			
}


void zapata::JSONArrT::setPath(std::string _path, zapata::JSONPtr _value, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;

	getline(_iss, _part, _separator[0]);
	zapata::JSONPtr _current = (* this)[_part];
	if (!_current->ok()) {
		if (_iss.good()) {
			zapata::JSONObj _new;
			_current = make_ptr(_new);
			this->push_back(_current);
			_current->setPath(_path.substr(_part.length() + 1), _value, _separator);
		}
		else {
			this->push(_value.get());
		}
	}
	else {
		if (_iss.good()) {
			_current->setPath(_path.substr(_part.length() + 1), _value, _separator);
		}
		else {
			_current >> _part;
			_current << _part << _value;			
		}
	}
}

void zapata::JSONArrT::delPath(std::string _path, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;

	getline(_iss, _part, _separator[0]);
	zapata::JSONPtr _current = (* this)[_part];
	if (!_current->ok()) {
		return;
	}

	while(_iss.good()) {
		getline(_iss, _part, _separator[0]);
		if (_current[_part]->ok()) {
			if (_iss.good()) {
				_current = _current[_part];
			}
			else {
				_current >> _part;
			}
		}
		else {
			return;
		}
	}
}

zapata::JSONPtr zapata::JSONArrT::clone() {
	zapata::JSONArr _return;
	for (auto _f : * this) {
		_return << _f->clone();
	}	
	return make_ptr(_return);
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

bool zapata::JSONArrT::operator<(zapata::JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f] < _rhs[_f]) {
			continue;
		}
		return false;
	}
	return true;
}

bool zapata::JSONArrT::operator<(zapata::JSONArr& _rhs) {
	return * this < * _rhs;
}

bool zapata::JSONArrT::operator>(zapata::JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f]  > _rhs[_f]) {
			continue;
		}
		return false;
	}
	return true;
}

bool zapata::JSONArrT::operator>(zapata::JSONArr& _rhs) {
	return * this > * _rhs;
}

bool zapata::JSONArrT::operator<=(zapata::JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f]  <= _rhs[_f]) {
			continue;
		}
		return false;
	}
	return true;
}

bool zapata::JSONArrT::operator<=(zapata::JSONArr& _rhs) {
	return * this <= * _rhs;
}

bool zapata::JSONArrT::operator>=(zapata::JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f]  >= _rhs[_f]) {
			continue;
		}
		return false;
	}
	return true;
}

bool zapata::JSONArrT::operator>=(zapata::JSONArr& _rhs) {
	return * this >= * _rhs;
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
			_out.insert(_out.length(), ",");
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
			_out << ",";
		}
		_first = false;
		_i->stringify(_out);
	}
	_out << "]" << flush;
}

void zapata::JSONArrT::prettify(string& _out, uint _n_tabs) {
	_out.insert(_out.length(), "[");
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out.insert(_out.length(), ",");
		}
		_out.insert(_out.length(), "\n");
		_first = false;
		_out.insert(_out.length(), string(_n_tabs + 1, '\t'));
		_i->prettify(_out, _n_tabs + 1);
	}
	if (!_first) {
		_out.insert(_out.length(), "\n");	
		_out.insert(_out.length(), string(_n_tabs, '\t'));
	}
	_out.insert(_out.length(), "]");
}

void zapata::JSONArrT::prettify(ostream& _out, uint _n_tabs) {
	_out << "[" << flush;
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out << ",";
		}
		_out << "\n ";
		_first = false;
		_out << string(_n_tabs + 1, '\t')<< flush;
		_i->prettify(_out, _n_tabs + 1);
	}
	if (!_first) {
		_out << "\n" << string(_n_tabs, '\t');
	}
	_out << "]" << flush;
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
			size_t _remainder = this->get()->date() % 1000;
			if (_remainder < 100) {
				_out.insert(_out.length(), "0");
				if (_remainder < 10) {
					_out.insert(_out.length(), "0");
				}
			}
			zapata::tostr(_out, _remainder);
			_out.insert(_out.length(), "Z");
			break;
		}
	}
	return _out;
}

zapata::JSONPtr::operator zapata::pretty() {
	if (this->get() == nullptr) {
		return zapata::pretty("");
	}
	string _out;
	switch(this->get()->type()) {
		case zapata::JSObject : {
			this->get()->obj()->prettify(_out);
			break;
		}
		case zapata::JSArray : {
			this->get()->arr()->prettify(_out);
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
			size_t _remainder = this->get()->date() % 1000;
			if (_remainder < 100) {
				_out.insert(_out.length(), "0");
				if (_remainder < 10) {
					_out.insert(_out.length(), "0");
				}
			}
			zapata::tostr(_out, _remainder);
			_out.insert(_out.length(), "Z");
			break;
		}
	}
	return zapata::pretty(_out);
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

zapata::JSONPtr::operator zapata::timestamp_t() {
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
			return (zapata::timestamp_t) this->get()->intr();
		}
		case zapata::JSDouble : {
			return (zapata::timestamp_t) this->get()->dbl();
		}
		case zapata::JSBoolean : {
			return (zapata::timestamp_t) this->get()->bln();
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
	assertz(this->get() != nullptr && this->get()->type() == zapata::JSObject, string("this element is not of type JSObject: ") + ((string) * this), 0, 0);
	return this->get()->obj();
}

zapata::JSONPtr::operator JSONArr() {
	assertz(this->get() != nullptr && this->get()->type() == zapata::JSArray, string("this element is not of type JSArray: ") + ((string) * this), 0, 0);
	return this->get()->arr();
}

zapata::JSONPtr::operator JSONObj&() {
	assertz(this->get() != nullptr && this->get()->type() == zapata::JSObject, string("this element is not of type JSObject: ") + ((string) * this), 0, 0);
	return this->get()->obj();
}

zapata::JSONPtr::operator JSONArr&() {
	assertz(this->get() != nullptr && this->get()->type() == zapata::JSArray, string("this element is not of type JSArray: ") + ((string) * this), 0, 0);
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


zapata::JSONObj::operator zapata::pretty() {
	if (this->get() == nullptr) {
		return "";
	}
	string _out;
	(* this)->prettify(_out);
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

zapata::JSONArr::operator zapata::pretty() {
	if (this->get() == nullptr) {
		return "";
	}
	string _out;
	(* this)->prettify(_out);
	return _out;
}

zapata::JSONArrT::iterator zapata::JSONArr::begin() {
	return (* this)->begin();
}

zapata::JSONArrT::iterator zapata::JSONArr::end() {
	return (* this)->end();
}

zapata::JSONPtr zapata::make_obj() {
	zapata::JSONObj _empty;
	return zapata::JSONPtr(new zapata::JSONElementT(_empty));
}

zapata::JSONPtr zapata::make_arr() {
	zapata::JSONArr _empty;
	return zapata::JSONPtr(new zapata::JSONElementT(_empty));
}
