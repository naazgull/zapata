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
#include <zapata/json/JSONParser.h>
#include <ostream>
#include <cstdarg>
#include <regex>

namespace zpt {
	JSONPtr undefined;
	JSONPtr nilptr = undefined;
	JSONPtr array = zpt::mkptr("1b394520-2fed-4118-b622-940f25b8b35e");
	symbol_table __lambdas = zpt::symbol_table(new std::map< std::string, std::tuple< std::string, unsigned short, zpt::symbol > >());
}

zpt::JSONElementT::JSONElementT() : __parent( nullptr ) {
	this->type(zpt::JSNil);
	this->__target.__nil = nullptr;
}

zpt::JSONElementT::JSONElementT(JSONElementT& _element) : __parent( nullptr ) {
	this->type( _element.type());
	switch(this->__target.__type) {
		case zpt::JSObject : {
			if (_element.obj().get() != nullptr) {
				this->__target.__object = _element.obj();
			}
			break;
		}
		case zpt::JSArray : {
			if (_element.arr().get() != nullptr) {
				this->__target.__array = _element.arr();
			}
			break;
		}
		case zpt::JSString : {
			this->__target.__string = make_shared< std::string >(std::string(_element.str().data()));
			break;
		}
		case zpt::JSInteger : {
			this->__target.__integer = _element.intr();
			break;
		}
		case zpt::JSDouble : {
			this->__target.__double = _element.dbl();
			break;
		}
		case zpt::JSBoolean : {
			this->__target.__boolean = _element.bln();
			break;
		}
		case zpt::JSNil : {
			this->__target.__nil = nullptr;
			break;
		}
		case zpt::JSDate : {
			this->__target.__date = _element.date();
			break;
		}
		case zpt::JSLambda : {
			if (_element.__target.__lambda.get() != nullptr) {
				this->__target.__lambda = _element.__target.__lambda;
			}
			break;
		}
	}
}

zpt::JSONElementT::JSONElementT(std::initializer_list<JSONElementT> _list) : __parent( nullptr ) {
	bool _is_array = (_list.size() > 1 && _list.begin()->__target.__type == zpt::JSString && (*_list.begin()->__target.__string) == "1b394520-2fed-4118-b622-940f25b8b35e");
	bool _is_object = (!_is_array && _list.size() % 2 == 0 && _list.begin()->__target.__type == zpt::JSString);
	if (_is_object) {
		size_t _idx = 0;
		size_t* _pidx = &_idx;
		this->type(zpt::JSObject);
		std::for_each(_list.begin(), _list.end(),
			[ & ] (const zpt::JSONElementT& _element) {
				if ((*_pidx) % 2 == 0) {
					this->__target.__object->push(std::string(_element.__target.__string->data()));
				}
				else {
					zpt::JSONElementT* _other = new zpt::JSONElementT();
					_other->type( _element.__target.__type);
					switch(_other->__target.__type) {
						case zpt::JSObject : {
							if (_element.__target.__object.get() != nullptr) {
								_other->__target.__object = _element.__target.__object;
							}
							break;
						}
						case zpt::JSArray : {
							if (_element.__target.__array.get() != nullptr) {
								_other->__target.__array = _element.__target.__array;
							}
							break;
						}
						case zpt::JSString : {
							_other->__target.__string = make_shared<std::string>(std::string(_element.__target.__string->data()));
							break;
						}
						case zpt::JSInteger : {
							_other->__target.__integer = _element.__target.__integer;
							break;
						}
						case zpt::JSDouble : {
							_other->__target.__double = _element.__target.__double;
							break;
						}
						case zpt::JSBoolean : {
							_other->__target.__boolean = _element.__target.__boolean;
							break;
						}
						case zpt::JSNil : {
							_other->__target.__nil = nullptr;
							break;
						}
						case zpt::JSDate : {
							_other->__target.__date = _element.__target.__date;
							break;
						}
						case zpt::JSLambda : {
							if (_element.__target.__lambda.get() != nullptr) {
								_other->__target.__lambda = _element.__target.__lambda;
							}
							break;
						}
					}
					this->__target.__object->push(_other);
				}
				(*_pidx)++;
			}
		);
	}
	else {
		this->type(zpt::JSArray);
		std::for_each((_is_array ? _list.begin() + 1 : _list.begin()), _list.end(),
			[ & ] (const zpt::JSONElementT& _element) {
				zpt::JSONElementT* _other = new zpt::JSONElementT();
				_other->type( _element.__target.__type);
				switch(_other->__target.__type) {
					case zpt::JSObject : {
						if (_element.__target.__object.get() != nullptr) {
							_other->__target.__object = _element.__target.__object;
						}
						break;
					}
					case zpt::JSArray : {
						if (_element.__target.__array.get() != nullptr) {
							_other->__target.__array = _element.__target.__array;
						}
						break;
					}
					case zpt::JSString : {
						_other->__target.__string = make_shared<std::string>(std::string(_element.__target.__string->data()));
						break;
					}
					case zpt::JSInteger : {
						_other->__target.__integer = _element.__target.__integer;
						break;
					}
					case zpt::JSDouble : {
						_other->__target.__double = _element.__target.__double;
						break;
					}
					case zpt::JSBoolean : {
						_other->__target.__boolean = _element.__target.__boolean;
						break;
					}
					case zpt::JSNil : {
						_other->__target.__nil = nullptr;
						break;
					}
					case zpt::JSDate : {
						_other->__target.__date = _element.__target.__date;
						break;
					}
					case zpt::JSLambda : {
						if (_element.__target.__lambda.get() != nullptr) {
							_other->__target.__lambda = _element.__target.__lambda;
						}
						break;
					}
				}
				this->__target.__array->push(_other);
			}
		);
	}
}

zpt::JSONElementT::JSONElementT(JSONPtr _value) {
	this->type(_value->type());
	switch(this->__target.__type) {
		case zpt::JSObject : {
			if (_value->obj().get() != nullptr) {
				this->__target.__object = _value->obj();
			}
			break;
		}
		case zpt::JSArray : {
			if (_value->arr().get() != nullptr) {
				this->__target.__array = _value->arr();
			}
			break;
		}
		case zpt::JSString : {
			this->__target.__string = make_shared< std::string >(std::string(_value->str().data()));
			break;
		}
		case zpt::JSInteger : {
			this->__target.__integer = _value->intr();
			break;
		}
		case zpt::JSDouble : {
			this->__target.__double = _value->dbl();
			break;
		}
		case zpt::JSBoolean : {
			this->__target.__boolean = _value->bln();
			break;
		}
		case zpt::JSNil : {
			this->__target.__nil = nullptr;
			break;
		}
		case zpt::JSDate : {
			this->__target.__date = _value->date();
			break;
		}
		case zpt::JSLambda : {
			if (_value->lbd().get() != nullptr) {
				this->__target.__lambda = _value->lbd();
			}
			break;
		}
	}
}

zpt::JSONElementT::JSONElementT(JSONObj& _value) : __parent( nullptr ) {
	this->type( zpt::JSObject);
	if (_value.get() != nullptr) {
		this->__target.__object = _value;
	}
}

zpt::JSONElementT::JSONElementT(JSONArr& _value) : __parent( nullptr ) {
	this->type( zpt::JSArray);
	if (_value.get() != nullptr) {
		this->__target.__array = _value;
	}
}

zpt::JSONElementT::JSONElementT(std::string _value) : __parent( nullptr ) {
	this->type( zpt::JSString);
	this->__target.__string = make_shared< std::string >(std::string(_value.data()));
}

zpt::JSONElementT::JSONElementT(const char* _value) : __parent( nullptr ) {
	this->type( zpt::JSString);
	this->__target.__string = make_shared< std::string >(std::string(_value));
}

zpt::JSONElementT::JSONElementT(long long _value) : __parent( nullptr ) {
	this->type( zpt::JSInteger);
	this->__target.__integer = _value;
}

zpt::JSONElementT::JSONElementT(double _value) : __parent( nullptr ) {
	this->type( zpt::JSDouble);
	this->__target.__double = _value;
}

zpt::JSONElementT::JSONElementT(bool _value) : __parent( nullptr ) {
	this->type( zpt::JSBoolean);
	this->__target.__boolean = _value;
}

zpt::JSONElementT::JSONElementT(zpt::timestamp_t _value) : __parent( nullptr ) {
	this->type( zpt::JSDate);
	this->__target.__date = _value;
}

zpt::JSONElementT::JSONElementT(int _value) : __parent( nullptr ) {
	this->type( zpt::JSInteger);
	this->__target.__integer = _value;
}

zpt::JSONElementT::JSONElementT(size_t _value) : __parent( nullptr ) {
	this->type( zpt::JSInteger);
	this->__target.__integer = _value;
}

#ifdef __LP64__
zpt::JSONElementT::JSONElementT(unsigned int _value) : __parent( nullptr ) {
	this->type( zpt::JSInteger);
	this->__target.__integer = _value;
}
#endif

zpt::JSONElementT::JSONElementT(zpt::lambda _value) {
	this->type( zpt::JSLambda );
	this->__target.__lambda = _value;
}

zpt::JSONElementT::~JSONElementT() {
}

zpt::JSONType zpt::JSONElementT::type() {
	return (zpt::JSONType)  this->__target.__type;
}

string zpt::JSONElementT::demangle() {
	switch(this->__target.__type) {
		case zpt::JSObject : {
			return "object";
		}
		case zpt::JSArray : {
			return "array";
		}
		case zpt::JSString : {
			return "string";
		}
		case zpt::JSInteger : {
			return "integer";
		}
		case zpt::JSDouble : {
			return "number";
		}
		case zpt::JSBoolean : {
			return "boolean";
		}
		case zpt::JSNil : {
			return "null";
		}
		case zpt::JSDate : {
			return "date";
		}
		case zpt::JSLambda : {
			return "lambda";
		}
	}
	return "null";
}

void zpt::JSONElementT::type(zpt::JSONType _in) {
	assertz(_in >= 0, "the type must be a valid value", 500, 0);
	
	if (_in == this->__target.__type) {
		return;
	}

	switch(this->__target.__type) {
		case zpt::JSObject : {
			if (this->__target.__object.get() != nullptr) {
				this->__target.__object.~JSONObj();
			}
			break;
		}
		case zpt::JSArray : {
			if (this->__target.__array.get() != nullptr) {
				this->__target.__array.~JSONArr();
			}
			break;
		}
		case zpt::JSString : {
			if (this->__target.__string.get() != nullptr) {
				this->__target.__string.~JSONStr();
			}
			break;
		}
		case zpt::JSLambda : {
			if (this->__target.__lambda.get() != nullptr) {
				this->__target.__lambda.~lambda();
			}
			break;
		}
		default : {
			break;
		}
	}
	switch(_in) {
		case zpt::JSObject : {
			new(& this->__target.__object) JSONObj();
			break;
		}
		case zpt::JSArray : {
			new(& this->__target.__array) JSONArr();
			break;
		}
		case zpt::JSString : {
			new(& this->__target.__string) JSONStr();
			break;
		}
		case zpt::JSLambda : {
			new(& this->__target.__lambda) lambda();
			break;
		}
		default : {
			break;
		}		
	}

	this->__target.__type = _in;
}

zpt::JSONUnion& zpt::JSONElementT::value() {
	return this->__target;
}

bool zpt::JSONElementT::ok() {
	return this->__target.__type != zpt::JSNil;
}

bool zpt::JSONElementT::empty() {
	switch(this->__target.__type) {
		case zpt::JSObject : {
			if (this->__target.__object.get() != nullptr) {
				return this->__target.__object->size() == 0;
			}
			return true;
		}
		case zpt::JSArray : {
			if (this->__target.__array.get() != nullptr) {
				return this->__target.__array->size() == 0;
			}
			return true;
		}
		case zpt::JSString : {
			if (this->__target.__string.get() != nullptr) {
				return (*this->__target.__string) == "";
			}
			return true;
		}
		case zpt::JSInteger : {
			return false;
		}
		case zpt::JSDouble : {
			return false;
		}
		case zpt::JSBoolean : {
			return false;
		}
		case zpt::JSNil : {
			return true;
		}
		case zpt::JSDate : {
			return false;
		}
		case zpt::JSLambda : {
			return false;
		}
	}
	return true;
}

bool zpt::JSONElementT::nil() {
	return this->__target.__type == zpt::JSNil;
}

void zpt::JSONElementT::assign(JSONElementT& _rhs) {
	this->type( _rhs.type());
	switch(this->__target.__type) {
		case zpt::JSObject : {
			if (this->__target.__object.get() != nullptr) {
				this->__target.__object.~JSONObj();
			}
			if (_rhs.obj().get() != nullptr) {
				this->__target.__object = _rhs.obj();
			}
			break;
		}
		case zpt::JSArray : {
			if (this->__target.__array.get() != nullptr) {
				this->__target.__array.~JSONArr();
			}
			if (_rhs.arr().get() != nullptr) {
				this->__target.__array = _rhs.arr();
			}
			break;
		}
		case zpt::JSString : {
			if (this->__target.__string.get() != nullptr) {
				this->__target.__string.~JSONStr();
			}
			this->__target.__string = make_shared< std::string >(std::string(_rhs.str().data()));
			break;
		}
		case zpt::JSInteger : {
			this->__target.__integer = _rhs.intr();
			break;
		}
		case zpt::JSDouble : {
			this->__target.__double = _rhs.dbl();
			break;
		}
		case zpt::JSBoolean : {
			this->__target.__boolean = _rhs.bln();
			break;
		}
		case zpt::JSNil : {
			this->__target.__nil = nullptr;
			break;
		}
		case zpt::JSDate : {
			this->__target.__date = _rhs.date();
			break;
		}
		case zpt::JSLambda : {
			if (this->__target.__lambda.get() != nullptr) {
				this->__target.__lambda.~lambda();
			}
			this->__target.__lambda = _rhs.lbd();
			break;
		}
	}
}

zpt::JSONElementT * zpt::JSONElementT::parent() {
	return this->__parent;
}

void zpt::JSONElementT::parent(JSONElementT* _parent) {
	this->__parent = _parent;
}

auto zpt::JSONElementT::is_object() -> bool {
	return this->__target.__type == zpt::JSObject;
}

auto zpt::JSONElementT::is_array() -> bool {
	return this->__target.__type == zpt::JSArray;
}

auto zpt::JSONElementT::is_string() -> bool {
	return this->__target.__type == zpt::JSString;
}

auto zpt::JSONElementT::is_integer() -> bool {
	return this->__target.__type == zpt::JSInteger;
}

auto zpt::JSONElementT::is_double() -> bool {
	return this->__target.__type == zpt::JSDouble;
}

auto zpt::JSONElementT::is_number() -> bool {
	return this->__target.__type == zpt::JSInteger ||  this->__target.__type == zpt::JSDouble;
}

auto zpt::JSONElementT::is_bool() -> bool {
	return this->__target.__type == zpt::JSBoolean;
}

auto zpt::JSONElementT::is_date() -> bool {
	return this->__target.__type == zpt::JSDate || this->__target.__type == zpt::JSString;
}

auto zpt::JSONElementT::is_lambda() -> bool {
	return this->__target.__type == zpt::JSLambda;
}

auto zpt::JSONElementT::is_nil() -> bool {
	return this->__target.__type == zpt::JSNil;
}

zpt::JSONObj& zpt::JSONElementT::obj() {
	assertz(this->__target.__type == zpt::JSObject, std::string("this element is not of type JSObject: ") + this->stringify(), 0, 0);
	return this->__target.__object;
}

zpt::JSONArr& zpt::JSONElementT::arr() {
	assertz(this->__target.__type == zpt::JSArray, std::string("this element is not of type JSArray: ") + this->stringify(), 0, 0);
	return this->__target.__array;
}

string zpt::JSONElementT::str() {
	assertz(this->__target.__type == zpt::JSString, std::string("this element is not of type JSString: ") + this->stringify(), 0, 0);
	return *(this->__target.__string.get());
}

long long zpt::JSONElementT::intr() {
	assertz(this->__target.__type == zpt::JSInteger, std::string("this element is not of type JSInteger: ") + this->stringify(), 0, 0);
	return this->__target.__integer;
}

double zpt::JSONElementT::dbl() {
	assertz(this->__target.__type == zpt::JSDouble, std::string("this element is not of type JSDouble: ") + this->stringify(), 0, 0);
	return this->__target.__double;
}

bool zpt::JSONElementT::bln() {
	assertz(this->__target.__type == zpt::JSBoolean, std::string("this element is not of type JSBoolean: ") + this->stringify(), 0, 0);
	return this->__target.__boolean;
}

zpt::timestamp_t zpt::JSONElementT::date() {
	assertz(this->__target.__type == zpt::JSDate || this->__target.__type == zpt::JSString, std::string("this element is not of type JSDate: ") + this->stringify(), 0, 0);
	if (this->__target.__type == zpt::JSString) {
		return zpt::timestamp(*(this->__target.__string.get()));
	}
	return this->__target.__date;
}

zpt::lambda& zpt::JSONElementT::lbd() {
	assertz(this->__target.__type == zpt::JSLambda, std::string("this element is not of type JSLambda: ") + this->stringify(), 0, 0);
	return this->__target.__lambda;
}

double zpt::JSONElementT::number() {
	assertz(this->__target.__type == zpt::JSDate || this->__target.__type == zpt::JSInteger || this->__target.__type == zpt::JSDouble || this->__target.__type == zpt::JSBoolean, std::string("this element is not of type JSInteger, JSDouble or JSBoolean: ") + this->stringify(), 0, 0);
	switch(this->__target.__type) {
		case zpt::JSInteger : {
			return (double) this->__target.__integer;
		}
		case zpt::JSDouble : {
			return this->__target.__double;
		}
		case zpt::JSBoolean : {
			return (double) this->__target.__boolean;
		}
		case zpt::JSDate : {
			return (double) this->__target.__date;
		}
		default : {
			return 0;
		}
	}
	return 0;
}

zpt::JSONPtr zpt::JSONElementT::clone() {
	switch(this->type()) {
		case zpt::JSObject : {
			return this->obj()->clone();
		}
		case zpt::JSArray : {
			return this->arr()->clone();
		}
		case zpt::JSString : {
			std::string _v = this->str();
			return zpt::mkptr(_v);
		}
		case zpt::JSInteger : {
			int _v = this->intr();
			return zpt::mkptr(_v);
		}
		case zpt::JSDouble : {
			double _v = this->dbl();
			return zpt::mkptr(_v);
		}
		case zpt::JSBoolean : {
			bool _v = this->bln();
			return zpt::mkptr(_v);
		}
		case zpt::JSNil : {
			return zpt::undefined;
		}
		case zpt::JSDate : {
			zpt::timestamp_t _v = this->date();
			return zpt::mkptr(_v);
		}
		case zpt::JSLambda : {
			return zpt::json::lambda(this->lbd()->name(), this->lbd()->n_args());
		}
	}
	return zpt::undefined;	
}

zpt::JSONElementT& zpt::JSONElementT::operator<<(const char* _in) {
	(* this) << std::string(_in);
	return * this;
}

zpt::JSONElementT& zpt::JSONElementT::operator<<(std::string _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			this->__target.__object->push(_in);
			break;
		}
		case zpt::JSArray : {
			this->__target.__array->push(new zpt::JSONElementT(std::string(_in)));
			break;
		}
		case zpt::JSString : {
			this->__target.__string.get()->assign(_in);
			break;
		}
		default : {
			assertz(this->__target.__type == zpt::JSObject || this->__target.__type == zpt::JSArray || this->__target.__type == zpt::JSString, "the type must be a JSObject, JSArray or JSString in order to push a std::string", 500, 0);
			break;
		}
	}
	return * this;
}

zpt::JSONElementT& zpt::JSONElementT::operator<<(JSONElementT* _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	if (this->__target.__type == _in->type() && _in->type() != zpt::JSObject && _in->type() != zpt::JSArray) {
		this->assign(* _in);
		return * this;
	}
	switch(this->__target.__type) {
		case zpt::JSObject : {
			this->__target.__object->push(_in);
			break;
		}
		case zpt::JSArray : {
			this->__target.__array->push(_in);
			break;
		}
		default : {
			assertz(this->__target.__type == zpt::JSObject || this->__target.__type == zpt::JSArray, "the type must be a JSObject, JSArray or the same type of the target, in order to push JSONElementT*", 500, 0);
			break;
		}
	}
	return * this;
}

zpt::JSONElementT& zpt::JSONElementT::operator<<(std::initializer_list<JSONElementT> _list) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	zpt::json _other(_list);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			if (_other->type() == zpt::JSObject) {
				for (auto _attribute : _other->obj()) {
					this->__target.__object->push(_attribute.first);
					this->__target.__object->push(_attribute.second);
				}
			}
			break;
		}
		case zpt::JSArray : {
			this->__target.__array->push(_other.get());
			break;
		}
		default : {
			assertz(this->__target.__type == zpt::JSObject || this->__target.__type == zpt::JSArray, "the type must be a JSObject, JSArray or the same type of the target, in order to push JSONElementT*", 500, 0);
			break;
		}
	}
	return * this;
}

bool zpt::JSONElementT::operator==(zpt::JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__object) == *(_in.obj());
		}
		case zpt::JSArray : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__array) == *(_in.arr());
		}
		case zpt::JSString : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__string.get()) == _in.str();
		}
		case zpt::JSInteger : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__integer == _in.number();
		}
		case zpt::JSDouble : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__double == _in.number();
		}
		case zpt::JSBoolean : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__boolean == _in.number();
		}
		case zpt::JSNil : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return true;
		}
		case zpt::JSDate : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__date == _in.number();
		}
		case zpt::JSLambda : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return this->__target.__lambda->signature() == _in.lbd()->signature();
		}
	}
	return false;
}

bool zpt::JSONElementT::operator==(zpt::json _rhs) {
	return * this == (*_rhs);
}

bool zpt::JSONElementT::operator==(zpt::JSONPtr _rhs) {
	return * this == (*_rhs);
}

bool zpt::JSONElementT::operator!=(JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	if (this->__target.__type != _in.type()) {
		return true;
	}
	switch(this->__target.__type) {
		case zpt::JSObject : {
			return this->__target.__object != _in.obj();
		}
		case zpt::JSArray : {
			return this->__target.__array != _in.arr();
		}
		case zpt::JSString : {
			return *(this->__target.__string.get()) != _in.str();
		}
		case zpt::JSInteger : {
			return this->__target.__integer != _in.intr();
		}
		case zpt::JSDouble : {
			return this->__target.__double != _in.dbl();
		}
		case zpt::JSBoolean : {
			return this->__target.__boolean != _in.bln();
		}
		case zpt::JSNil : {
			return true;
		}
		case zpt::JSDate : {
			return this->__target.__date != _in.date();
		}
		case zpt::JSLambda : {
			return this->__target.__lambda->signature() != _in.lbd()->signature();
		}
	}
	return false;
}

bool zpt::JSONElementT::operator!=(zpt::json _rhs) {
	return * this != * _rhs;
}

bool zpt::JSONElementT::operator!=(zpt::JSONPtr _rhs) {
	return * this != * _rhs;
}

bool zpt::JSONElementT::operator<(zpt::JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__object) < *(_in.obj());
		}
		case zpt::JSArray : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__array) < *(_in.arr());
		}
		case zpt::JSString : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__string.get()) < _in.str();
		}
		case zpt::JSInteger : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__integer < _in.number();
		}
		case zpt::JSDouble : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__double < _in.number();
		}
		case zpt::JSBoolean : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__boolean < _in.number();
		}
		case zpt::JSNil : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return true;
		}
		case zpt::JSDate : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__date < _in.number();
		}
		case zpt::JSLambda : {
			return this->__target.__lambda->n_args() < _in.lbd()->n_args();
		}
	}
	return false;
}

bool zpt::JSONElementT::operator<(zpt::json _rhs) {
	return * this < * _rhs;
}

bool zpt::JSONElementT::operator<(zpt::JSONPtr _rhs) {
	return * this < * _rhs;
}

bool zpt::JSONElementT::operator>(zpt::JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__object) > *(_in.obj());
		}
		case zpt::JSArray : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__array) > *(_in.arr());
		}
		case zpt::JSString : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__string.get()) > _in.str();
		}
		case zpt::JSInteger : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__integer > _in.number();
		}
		case zpt::JSDouble : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__double > _in.number();
		}
		case zpt::JSBoolean : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__boolean > _in.number();
		}
		case zpt::JSNil : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return true;
		}
		case zpt::JSDate : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__date > _in.number();
		}
		case zpt::JSLambda : {
			return this->__target.__lambda->n_args() > _in.lbd()->n_args();
		}
	}
	return false;
}

bool zpt::JSONElementT::operator>(zpt::json _rhs) {
	return * this > * _rhs;
}

bool zpt::JSONElementT::operator>(zpt::JSONPtr _rhs) {
	return * this > * _rhs;
}

bool zpt::JSONElementT::operator<=(zpt::JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__object) <= *(_in.obj());
		}
		case zpt::JSArray : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__array) <= *(_in.arr());
		}
		case zpt::JSString : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__string.get()) <= _in.str();
		}
		case zpt::JSInteger : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__integer <= _in.number();
		}
		case zpt::JSDouble : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__double <= _in.number();
		}
		case zpt::JSBoolean : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__boolean <= _in.number();
		}
		case zpt::JSNil : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return true;
		}
		case zpt::JSDate : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__date <= _in.number();
		}
		case zpt::JSLambda : {
			return this->__target.__lambda->n_args() <= _in.lbd()->n_args();
		}
	}
	return false;
}

bool zpt::JSONElementT::operator<=(zpt::json _rhs) {
	return * this <= * _rhs;
}

bool zpt::JSONElementT::operator<=(zpt::JSONPtr _rhs) {
	return * this <= * _rhs;
}

bool zpt::JSONElementT::operator>=(zpt::JSONElementT& _in) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__object) >= *(_in.obj());
		}
		case zpt::JSArray : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__array) >= *(_in.arr());
		}
		case zpt::JSString : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return *(this->__target.__string.get()) >= _in.str();
		}
		case zpt::JSInteger : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__integer >= _in.number();
		}
		case zpt::JSDouble : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__double >= _in.number();
		}
		case zpt::JSBoolean : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__boolean >= _in.number();
		}
		case zpt::JSNil : {
			if (this->__target.__type != _in.type()) {
				return false;
			}
			return true;
		}
		case zpt::JSDate : {
			if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
				return false;
			}
			return this->__target.__date >= _in.number();
		}
		case zpt::JSLambda : {
			return this->__target.__lambda->n_args() >= _in.lbd()->n_args();
		}
	}
	return false;
}

bool zpt::JSONElementT::operator>=(zpt::json _rhs) {
	return * this >= * _rhs;
}

bool zpt::JSONElementT::operator>=(zpt::JSONPtr _rhs) {
	return * this >= * _rhs;
}

zpt::JSONPtr zpt::JSONElementT::operator+(zpt::json _rhs) {
	return (* this) + (* _rhs);
}

zpt::JSONPtr zpt::JSONElementT::operator+(zpt::JSONPtr _rhs) {
	return (* this) + (* _rhs);
}

zpt::JSONPtr zpt::JSONElementT::operator+(zpt::JSONElementT& _rhs) {
	if (this->__target.__type == zpt::JSNil) {
		zpt::JSONPtr _rrhs = _rhs.clone();
		return _rrhs;
	}
	if (_rhs.__target.__type == zpt::JSNil) {
		zpt::JSONPtr _lhs = this->clone();
		return _lhs;
	}
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	assertz(this->__target.__type == zpt::JSArray || _rhs.__target.__type == zpt::JSArray || this->__target.__type == _rhs.__target.__type, "can't add JSON objects of different types", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			zpt::JSONPtr _lhs = this->clone();
			for (auto _e : _rhs.obj()) {
				if (_lhs[_e.first]->type() == zpt::JSObject || _lhs[_e.first]->type() == zpt::JSArray) {
					_lhs << _e.first << (_lhs[_e.first] + _e.second);
				}
				else {
					_lhs << _e.first << _e.second;
				}
			}
			return _lhs;
		}
		case zpt::JSArray : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = this->clone();
				for (auto _e : _rhs.arr()) {
					_lhs << _e;
				}
				return _lhs;
			}
			else {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : this->arr()) {
					_lhs << (_e + _rhs);
				}
				return _lhs;
			}
		}
		case zpt::JSString : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) + _e);
				}
				return _lhs;
			}
			else {
				std::string _lhs((*(this->__target.__string.get())) + _rhs.str());
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSInteger : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) + _e);
				}
				return _lhs;
			}
			else {
				int _lhs = this->__target.__integer + _rhs.intr();
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSDouble : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) + _e);
				}
				return _lhs;
			}
			else {
				double _lhs = this->__target.__double + _rhs.dbl();
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSBoolean : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) + _e);
				}
				return _lhs;
			}
			else {
				bool _lhs = this->__target.__boolean || _rhs.bln();
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSNil : {
			return zpt::undefined;
		}
		case zpt::JSDate : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) + _e);
				}
				return _lhs;
			}
			else {
				int _lhs = this->__target.__date + _rhs.number();
				return zpt::mkptr((zpt::timestamp_t) _lhs);
			}
		}
		case zpt::JSLambda : {
			return zpt::undefined;
		}
	}
	return zpt::undefined;
}

zpt::JSONPtr zpt::JSONElementT::operator-(zpt::json _rhs) {
	return (* this) - (* _rhs);
}

zpt::JSONPtr zpt::JSONElementT::operator-(zpt::JSONPtr _rhs) {
	return (* this) - (* _rhs);
}

zpt::JSONPtr zpt::JSONElementT::operator-(zpt::JSONElementT& _rhs) {
	if (this->__target.__type == zpt::JSNil) {
		zpt::JSONPtr _rrhs = _rhs.clone();
		return _rrhs;
	}
	if (_rhs.__target.__type == zpt::JSNil) {
		zpt::JSONPtr _lhs = this->clone();
		return _lhs;
	}
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	assertz(this->__target.__type == zpt::JSArray || _rhs.__target.__type == zpt::JSArray || this->__target.__type == _rhs.__target.__type, "can't substract JSON objects of different types", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			zpt::JSONPtr _lhs = this->clone();
			for (auto _e : _rhs.obj()) {
				_lhs >> _e.first;
			}
			return _lhs;
		}
		case zpt::JSArray : {
			if (_rhs.__target.__type == zpt::JSArray) {
				assertz(this->arr()->size() == _rhs.arr()->size(), "both arrays must have the same size", 500, 0);
				zpt::JSONPtr _lhs = zpt::json::array();
				for (size_t _e = 0; _e != _rhs.arr()->size(); _e++) {
					_lhs << (this[_e] - _rhs[_e]);
				}
				return _lhs;
			}
			else {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : this->arr()) {
					_lhs << (_e - _rhs);
				}
				return _lhs;
			}
		}
		case zpt::JSString : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) - _e);
				}
				return _lhs;
			}
			else {
				std::string _lhs(this->__target.__string.get()->data());
				std::size_t _idx = 0;
				while ((_idx = _lhs.find(_rhs.str(), _idx)) != std::string::npos) {
					_lhs.erase(_idx, _rhs.str().length());
				}
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSInteger : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) - _e);
				}
				return _lhs;
			}
			else {
				int _lhs = this->__target.__integer - _rhs.intr();
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSDouble : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) - _e);
				}
				return _lhs;
			}
			else {
				double _lhs = this->__target.__double - _rhs.dbl();
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSBoolean : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) - _e);
				}
				return _lhs;
			}
			else {
				bool _lhs = this->__target.__boolean & _rhs.bln();
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSNil : {
			return zpt::undefined;
		}
		case zpt::JSDate : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) - _e);
				}
				return _lhs;
			}
			else {
				int _lhs = this->__target.__date - _rhs.number();
				return zpt::mkptr((zpt::timestamp_t) _lhs);
			}
		}
		case zpt::JSLambda : {
			return zpt::undefined;
		}
	}
	return zpt::undefined;
}

zpt::JSONPtr zpt::JSONElementT::operator/(zpt::json _rhs) {
	return (* this) / (* _rhs);
}

zpt::JSONPtr zpt::JSONElementT::operator/(zpt::JSONPtr _rhs) {
	return (* this) / (* _rhs);
}

zpt::JSONPtr zpt::JSONElementT::operator/(zpt::JSONElementT& _rhs) {
	if (this->__target.__type == zpt::JSNil) {
		zpt::JSONPtr _rrhs = _rhs.clone();
		return _rrhs;
	}
	if (_rhs.__target.__type == zpt::JSNil) {
		zpt::JSONPtr _lhs = this->clone();
		return _lhs;
	}
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	assertz(this->__target.__type == zpt::JSArray || _rhs.__target.__type == zpt::JSArray || this->__target.__type == _rhs.__target.__type, "can't divide JSON objects of different types", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			assertz(this->__target.__type == zpt::JSObject, "can't divide JSON objects", 500, 0);
		}
		case zpt::JSArray : {
			if (_rhs.__target.__type == zpt::JSArray) {
				assertz(this->arr()->size() == _rhs.arr()->size(), "both arrays must have the same size", 500, 0);
				zpt::JSONPtr _lhs = zpt::json::array();
				for (size_t _e = 0; _e != _rhs.arr()->size(); _e++) {
					_lhs << (this[_e] / _rhs[_e]);
				}
				return _lhs;
			}
			else {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : this->arr()) {
					_lhs << (_e / _rhs);
				}
				return _lhs;
			}
		}
		case zpt::JSString : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) / _e);
				}
				return _lhs;
			}
			else {
				std::string _lhs(this->__target.__string.get()->data());
				std::size_t _idx = 0;
				while ((_idx = _lhs.find(_rhs.str(), _idx)) != std::string::npos) {
					_lhs.erase(_idx, _rhs.str().length());
				}
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSInteger : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) / _e);
				}
				return _lhs;
			}
			else {
				int _lhs = this->__target.__integer / _rhs.intr();
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSDouble : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) / _e);
				}
				return _lhs;
			}
			else {
				double _lhs = this->__target.__double / _rhs.dbl();
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSBoolean : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) / _e);
				}
				return _lhs;
			}
			else {
				bool _lhs = this->__target.__boolean / _rhs.bln();
				return zpt::mkptr(_lhs);
			}
		}
		case zpt::JSNil : {
			return zpt::undefined;
		}
		case zpt::JSDate : {
			if (_rhs.__target.__type == zpt::JSArray) {
				zpt::JSONPtr _lhs = zpt::json::array();
				for (auto _e : _rhs.arr()) {
					_lhs << ((*this) / _e);
				}
				return _lhs;
			}
			else {
				int _lhs = this->__target.__date / _rhs.number();
				return zpt::mkptr((zpt::timestamp_t) _lhs);
			}
		}
		case zpt::JSLambda : {
			return zpt::undefined;
		}
	}
	return zpt::undefined;
}

zpt::JSONPtr zpt::JSONElementT::getPath(std::string _path, std::string _separator) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			return this->__target.__object->getPath(_path, _separator);
		}
		case zpt::JSArray : {
			return this->__target.__array->getPath(_path, _separator);
		}
		case zpt::JSString :
		case zpt::JSInteger :
		case zpt::JSDouble :
		case zpt::JSBoolean :
		case zpt::JSNil :
		case zpt::JSLambda :
		case zpt::JSDate : {
			return zpt::undefined;
		}
	}
	return zpt::undefined;
}

void zpt::JSONElementT::setPath(std::string _path, zpt::JSONPtr _value, std::string _separator) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			return this->__target.__object->setPath(_path, _value, _separator);
		}
		case zpt::JSArray : {
			return this->__target.__array->setPath(_path, _value, _separator);
		}
		case zpt::JSString :
		case zpt::JSInteger :
		case zpt::JSDouble :
		case zpt::JSBoolean :
		case zpt::JSNil :
		case zpt::JSLambda :
		case zpt::JSDate : {
			return;
		}
	}
	return;
}

void zpt::JSONElementT::delPath(std::string _path, std::string _separator) {
	assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
	switch(this->__target.__type) {
		case zpt::JSObject : {
			this->__target.__object->delPath(_path, _separator);
		}
		case zpt::JSArray : {
			this->__target.__array->delPath(_path, _separator);
		}
		case zpt::JSString :
		case zpt::JSInteger :
		case zpt::JSDouble :
		case zpt::JSBoolean :
		case zpt::JSNil :
		case zpt::JSLambda :
		case zpt::JSDate : {
			return;
		}
	}
}

zpt::JSONPtr zpt::JSONElementT::flatten() {
	if (this->type() == zpt::JSObject || this->type() == zpt::JSArray) {
		zpt::JSONPtr _return = zpt::json::object();
		this->inspect({ "$any", "type" },
			[ & ] (std::string _object_path, std::string _key, zpt::JSONElementT& _parent) -> void {
				zpt::JSONPtr _self = this->getPath(_object_path);
				if (_self->type() != zpt::JSObject && _self->type() != zpt::JSArray) {
					_return << _object_path << _self;
				}
			}
		);
		return _return;
	}
	else {
		return this->clone();
	}
}

void zpt::JSONElementT::inspect(zpt::JSONPtr _pattern, std::function< void (std::string, std::string, zpt::JSONElementT&) > _callback, zpt::JSONElementT * _parent, std::string _key, std::string _parent_path) {
	switch(this->type()) {
		case zpt::JSObject: {
			for (auto _o : this->obj()) {
				if (_pattern->type() == zpt::JSObject && _pattern[_o.first]->ok()) {
					_o.second->inspect(_pattern[_o.first], _callback, this, _o.first, (_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key));
					continue;
				}
				_o.second->inspect(_pattern, _callback, this, _o.first, (_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key));
			}
			if (_pattern["$any"]->ok()) {
				_callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key), _key, * _parent);
			}
			else {
				if (*this == _pattern) {
					_callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key), _key, * _parent);
				}
			}
			break;
		}
		case zpt::JSArray: {
			for (size_t _i = 0; _i != this->arr()->size(); _i++) {
				this->arr()[_i]->inspect(_pattern, _callback, this, std::to_string(_i), (_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key));
			}
			if (_pattern["$any"]->ok()) {
				_callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key), _key, * _parent);
			}
			else {
				if (*this == _pattern) {
					_callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key), _key, * _parent);
				}
			}
			break;
		}
		default: {
			if (_pattern["$regexp"]->ok()) {
				std::regex _rgx(((std::string) _pattern["$regexp"]));
				std::string _exp;
				this->stringify(_exp);
				if (std::regex_match(_exp, _rgx)) {
					_callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key), _key, * _parent);
				}
			}
			else if (_pattern["$any"]->ok()) {
				_callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key), _key, * _parent);
			}
			else {
				if (*this == _pattern) {
					_callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key), _key, * _parent);
				}
			}
			break;
		}
	}	
}

void zpt::JSONElementT::stringify(ostream& _out) {
	switch(this->__target.__type) {
		case zpt::JSObject : {
			this->__target.__object->stringify(_out);
			break;
		}
		case zpt::JSArray : {
			this->__target.__array->stringify(_out);
			break;
		}
		case zpt::JSString : {
			std::string _str(this->str());
			zpt::json::stringify(_str);
			_out << "\"" << _str << "\"" << flush;
			break;
		}
		case zpt::JSInteger : {
			_out << this->__target.__integer << flush;
			break;
		}
		case zpt::JSDouble : {
			_out << this->__target.__double << flush;
			break;
		}
		case zpt::JSBoolean : {
			_out << (this->__target.__boolean ? "true" : "false") << flush;
			break;
		}
		case zpt::JSNil : {
			_out <<  "null" << flush;
			break;
		}
		case zpt::JSDate : {
			_out << "\"" << zpt::timestamp(this->__target.__date) << "\"" << flush;
			break;
		}
		case zpt::JSLambda : {
			_out << this->__target.__lambda->signature() << flush;
			break;
		}
	}
}

void zpt::JSONElementT::stringify(std::string& _out) {
	switch(this->__target.__type) {
		case zpt::JSObject : {
			this->__target.__object->stringify(_out);
			break;
		}
		case zpt::JSArray : {
			this->__target.__array->stringify(_out);
			break;
		}
		case zpt::JSString : {
			std::string _str(this->str());
			zpt::json::stringify(_str);
			_out.insert(_out.length(), "\"");
			_out.insert(_out.length(), _str);
			_out.insert(_out.length(), "\"");
			break;
		}
		case zpt::JSInteger : {
			zpt::tostr(_out, this->__target.__integer);
			break;
		}
		case zpt::JSDouble : {
			zpt::tostr(_out, this->__target.__double);
			break;
		}
		case zpt::JSBoolean : {
			zpt::tostr(_out, this->__target.__boolean);
			break;
		}
		case zpt::JSNil : {
			_out.insert(_out.length(), "null");
			break;
		}
		case zpt::JSDate : {
			_out.insert(_out.length(), "\"");
			_out.insert(_out.length(), zpt::timestamp(this->__target.__date));
			_out.insert(_out.length(), "\"");
			break;
		}
		case zpt::JSLambda : {
			_out.insert(_out.length(), this->__target.__lambda->signature());
			break;
		}
	}
}

std::string zpt::JSONElementT::stringify() {
	string _out;
	this->stringify(_out);
	return _out;
}

void zpt::JSONElementT::prettify(ostream& _out, uint _n_tabs) {
	switch(this->__target.__type) {
		case zpt::JSObject : {
			this->__target.__object->prettify(_out, _n_tabs);
			break;
		}
		case zpt::JSArray : {
			this->__target.__array->prettify(_out, _n_tabs);
			break;
		}
		case zpt::JSString : {
			std::string _str(this->str());
			zpt::json::stringify(_str);
			_out << "\"" << _str << "\"" << flush;
			break;
		}
		case zpt::JSInteger : {
			_out << this->__target.__integer << flush;
			break;
		}
		case zpt::JSDouble : {
			_out << this->__target.__double << flush;
			break;
		}
		case zpt::JSBoolean : {
			_out << (this->__target.__boolean ? "true" : "false") << flush;
			break;
		}
		case zpt::JSNil : {
			_out <<  "null" << flush;
			break;
		}
		case zpt::JSDate : {
			_out << "\"" << zpt::timestamp(this->__target.__date) << "\"" << flush;
			break;
		}
		case zpt::JSLambda : {
			_out << this->__target.__lambda->signature() << flush;
			break;
		}
	}
	if (_n_tabs == 0) {
		_out << endl << flush;
	}
}

void zpt::JSONElementT::prettify(std::string& _out, uint _n_tabs) {
	switch(this->__target.__type) {
		case zpt::JSObject : {
			this->__target.__object->prettify(_out, _n_tabs);
			break;
		}
		case zpt::JSArray : {
			this->__target.__array->prettify(_out, _n_tabs);
			break;
		}
		case zpt::JSString : {
			std::string _str(this->str());
			zpt::json::stringify(_str);
			_out.insert(_out.length(), "\"");
			_out.insert(_out.length(), _str);
			_out.insert(_out.length(), "\"");
			break;
		}
		case zpt::JSInteger : {
			zpt::tostr(_out, this->__target.__integer);
			break;
		}
		case zpt::JSDouble : {
			zpt::tostr(_out, this->__target.__double);
			break;
		}
		case zpt::JSBoolean : {
			zpt::tostr(_out, this->__target.__boolean);
			break;
		}
		case zpt::JSNil : {
			_out.insert(_out.length(), "null");
			break;
		}
		case zpt::JSDate : {
			_out.insert(_out.length(), "\"");
			_out.insert(_out.length(), zpt::timestamp(this->__target.__date));
			_out.insert(_out.length(), "\"");
			break;
		}
		case zpt::JSLambda : {
			_out.insert(_out.length(), this->__target.__lambda->signature());
			break;
		}
	}
	if (_n_tabs == 0) {
		_out.insert(_out.length(), "\n");
	}
}

/*JSON CONTEXT*/
zpt::JSONContext::JSONContext(void* _target) : __target(_target) {
}

zpt::JSONContext::~JSONContext() {
	this->__target = nullptr;
}

void* zpt::JSONContext::unpack() {
	return this->__target;
}
		
zpt::context::context(void* _target) : std::shared_ptr< zpt::JSONContext >(new zpt::JSONContext(_target)) {
}

zpt::context::~context() {
}

/*JSON LAMBDA */
zpt::lambda::lambda() : std::shared_ptr< zpt::JSONLambda >(std::make_shared< zpt::JSONLambda >(zpt::JSONLambda())) {
}

zpt::lambda::lambda(std::shared_ptr< zpt::JSONLambda > _target) : std::shared_ptr< zpt::JSONLambda >(_target) {
}

zpt::lambda::lambda(zpt::lambda& _target) : std::shared_ptr< zpt::JSONLambda >(_target) {
}

zpt::lambda::lambda(zpt::JSONLambda* _target) : std::shared_ptr< zpt::JSONLambda >(_target) {
}

zpt::lambda::lambda(std::string _signature) : std::shared_ptr< zpt::JSONLambda >(new zpt::JSONLambda(_signature)) {
}

zpt::lambda::lambda(std::string _name, unsigned short _n_args) : std::shared_ptr< zpt::JSONLambda >(new zpt::JSONLambda(_name, _n_args)) {
}

zpt::lambda::~lambda() {
}

zpt::json zpt::lambda::operator ()(zpt::json _args, zpt::context _ctx) {
	return this->get()->call(_args, _ctx);
}

std::tuple< std::string, unsigned short > zpt::lambda::parse(std::string _signature) {
	size_t _lpar = _signature.find("(");
	size_t _rpar = _signature.find(")");
	size_t _comma = _signature.find(",");

	assertz(_lpar != std::string::npos && _rpar != std::string::npos && _comma != std::string::npos, "lambda signature format not recognized", 412, 0);

	std::string _name(_signature.substr(_lpar + 1, _comma - _lpar - 1));
	std::string _args(_signature.substr(_comma + 1, _rpar - _comma - 1));
	zpt::replace(_name, "\"", "");
	zpt::trim(_name);
	zpt::trim(_args);
	unsigned short _n_args = std::stoi(_args);
	return std::make_tuple(_name, _n_args);
}

std::string zpt::lambda::stringify(std::string _name, unsigned short _n_args) {
	return std::string("lambda(\"") + _name + std::string("\",") + std::to_string(_n_args) + std::string(")");
 }

void zpt::lambda::add(std::string _signature, zpt::symbol _lambda) {
	try {
		zpt::lambda::find(_signature);
		assertz(true, "lambda already defined", 412, 0);
	}
	catch(zpt::assertion& _e) {}
	std::tuple< std::string, unsigned short > _parsed = zpt::lambda::parse(_signature);
	zpt::__lambdas->insert(std::make_pair(_signature, std::make_tuple(std::get<0>(_parsed), std::get<1>(_parsed), _lambda)));
}

void zpt::lambda::add(std::string _name, unsigned short _n_args, zpt::symbol _lambda) {
	try {
		zpt::lambda::find(_name, _n_args);
		assertz(true, "lambda already defined", 412, 0);
	}
	catch(zpt::assertion& _e) {}
	std::string _signature(zpt::lambda::stringify(_name, _n_args));
	zpt::__lambdas->insert(std::make_pair(_signature, std::make_tuple(_name, _n_args, _lambda)));
}

zpt::json zpt::lambda::call(std::string _name, zpt::json _args, zpt::context  _ctx) {
	assertz(_args->type() == zpt::JSArray, "second argument must be a JSON array", 412, 0);
	zpt::symbol _f = zpt::lambda::find(_name, _args->arr()->size());
	return _f(_args, _args->arr()->size(), _ctx);
}

zpt::symbol zpt::lambda::find(std::string _signature) {
	auto _found = zpt::__lambdas->find(_signature);
	assertz(_found != zpt::__lambdas->end(), std::string("symbol for ") + _signature + std::string(" was not found"), 404, 0);
	return std::get<2>(_found->second);
}

zpt::symbol zpt::lambda::find(std::string _name, unsigned short _n_args) {
	std::string _signature = zpt::lambda::stringify(_name, _n_args);
	return zpt::lambda::find(_signature);
}

zpt::JSONLambda::JSONLambda() : __name(""), __n_args(0) {
}

zpt::JSONLambda::JSONLambda(std::string _signature) {
	std::tuple< std::string, unsigned short > _parsed = zpt::lambda::parse(_signature);
	this->__name = std::get<0>(_parsed);
	this->__n_args = std::get<1>(_parsed);
}

zpt::JSONLambda::JSONLambda(std::string _name, unsigned short _n_args) : __name(_name), __n_args(_n_args) {
}

zpt::JSONLambda::~JSONLambda() {
}

std::string zpt::JSONLambda::name() {
	return this->__name;
}

unsigned short zpt::JSONLambda::n_args() {
	return this->__n_args;
}

std::string zpt::JSONLambda::signature() {
	return zpt::lambda::stringify(this->__name, this->__n_args);
}

zpt::json zpt::JSONLambda::call(zpt::json _args, zpt::context _ctx) {
	return zpt::lambda::call(this->__name, _args, _ctx);
}		

/*JSON OBJECT*/
zpt::JSONObjT::JSONObjT() {
}

zpt::JSONObjT::~JSONObjT(){
}

void zpt::JSONObjT::push(std::string _name) {
	if (this->__name.length() == 0) {
		this->__name.assign(_name.data());
	}
	else {
		this->pop(this->__name);
		this->insert(pair<string, JSONPtr>(std::string(this->__name.data()), JSONPtr(new JSONElementT(std::string(_name.data())))));
		this->__name.clear();
	}
}

void zpt::JSONObjT::push(JSONElementT& _value) {
	assertz(this->__name.length() != 0, "you must pass a field name first", 500, 0);
	this->pop(this->__name);
	this->insert(pair<string, JSONPtr>(this->__name, JSONPtr(new JSONElementT(_value))));
	this->__name.clear();
}

void zpt::JSONObjT::push(JSONElementT* _value) {
	assertz(this->__name.length() != 0, "you must pass a field name first", 500, 0);
	this->pop(this->__name);
	this->insert(pair<string, JSONPtr>(this->__name, JSONPtr(_value)));
	this->__name.clear();
}

void zpt::JSONObjT::pop(int _name) {
	this->pop(zpt::tostr(_name));
}

void zpt::JSONObjT::pop(size_t _name) {
	this->pop(zpt::tostr(_name));
}

void zpt::JSONObjT::pop(const char* _name) {
	this->pop(std::string(_name));
}

void zpt::JSONObjT::pop(std::string _name) {
	auto _found = this->find(_name);
	if (_found != this->end()) {
		this->erase(_found);
	}
}

zpt::JSONPtr zpt::JSONObjT::getPath(std::string _path, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;
	std::string _remainder;

	getline(_iss, _part, _separator[0]);
	getline(_iss, _remainder);
	zpt::trim(_remainder);
	zpt::JSONPtr _current = (* this)[_part];
	if (!_current->ok()) {
		if (_part == "*" && _remainder.length() != 0) {
			for (auto _a : (* this)) {
				_current = _a.second->getPath(_remainder, _separator);
				if (_current->ok()) {
					return _current;
				}
			}
		}
		return zpt::undefined;
	}

	if (_remainder.length() == 0) {
		return _current;
	}
	return _current->getPath(_remainder, _separator);
}

void zpt::JSONObjT::setPath(std::string _path, zpt::JSONPtr _value, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;

	getline(_iss, _part, _separator[0]);
	zpt::JSONPtr _current = (* this)[_part];
	if (!_current->ok()) {
		if (_iss.good()) {
			zpt::JSONObj _new;
			_current = mkptr(_new);
			this->insert(pair<string, JSONPtr>(std::string(_part.data()), _current));
			_current->setPath(_path.substr(_part.length() + 1), _value, _separator);
		}
		else {
			this->insert(pair<string, JSONPtr>(std::string(_part.data()), _value));
		}
	}
	else {
		if (_iss.good()) {
			_current->setPath(_path.substr(_part.length() + 1), _value, _separator);
		}
		else {
			this->pop(_part);
			this->insert(pair<string, JSONPtr>(std::string(_part.data()), _value));
		}
	}
}

void zpt::JSONObjT::delPath(std::string _path, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;

	getline(_iss, _part, _separator[0]);
	zpt::JSONPtr _current = (* this)[_part];
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

zpt::JSONPtr zpt::JSONObjT::clone() {
	zpt::JSONObj _return;
	for (auto _f : * this) {
		_return << _f.first << _f.second->clone();
	}	
	return zpt::mkptr(_return);
}

bool zpt::JSONObjT::operator==(zpt::JSONObjT& _rhs) {
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

bool zpt::JSONObjT::operator==(zpt::JSONObj& _rhs) {
	return * this == * _rhs;
}

bool zpt::JSONObjT::operator!=(JSONObjT& _rhs) {
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

bool zpt::JSONObjT::operator!=(zpt::JSONObj& _rhs) {
	return * this != * _rhs;
}

bool zpt::JSONObjT::operator<(zpt::JSONObjT& _rhs) {
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

bool zpt::JSONObjT::operator<(zpt::JSONObj& _rhs) {
	return * this < * _rhs;
}

bool zpt::JSONObjT::operator>(zpt::JSONObjT& _rhs) {
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

bool zpt::JSONObjT::operator>(zpt::JSONObj& _rhs) {
	return * this > * _rhs;
}

bool zpt::JSONObjT::operator<=(zpt::JSONObjT& _rhs) {
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

bool zpt::JSONObjT::operator<=(zpt::JSONObj& _rhs) {
	return * this <= * _rhs;
}

bool zpt::JSONObjT::operator>=(zpt::JSONObjT& _rhs) {
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

bool zpt::JSONObjT::operator>=(zpt::JSONObj& _rhs) {
	return * this >= * _rhs;
}

zpt::JSONPtr& zpt::JSONObjT::operator[](int _idx) {
	return (* this)[(size_t) _idx];
}

zpt::JSONPtr& zpt::JSONObjT::operator[](size_t _idx) {
	return (* this)[zpt::tostr(_idx)];
}

zpt::JSONPtr& zpt::JSONObjT::operator[](const char* _idx) {
	return (* this)[string(_idx)];
}

zpt::JSONPtr& zpt::JSONObjT::operator[](std::string _idx) {
	auto _found = this->find(_idx);
	if (_found != this->end()) {
		return _found->second;
	}
	return zpt::undefined;
}

void zpt::JSONObjT::stringify(std::string& _out) {
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

void zpt::JSONObjT::stringify(ostream& _out) {
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

void zpt::JSONObjT::prettify(std::string& _out, uint _n_tabs) {
	_out.insert(_out.length(), "{");
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out.insert(_out.length(), ",");
		}
		_out.insert(_out.length(), "\n");
		_first = false;
		_out.insert(_out.length(), std::string(_n_tabs + 1, '\t'));
		_out.insert(_out.length(), "\"");
		_out.insert(_out.length(), _i.first);
		_out.insert(_out.length(), "\" : ");
		_i.second->prettify(_out, _n_tabs + 1);
	}
	if (!_first) {
		_out.insert(_out.length(), "\n");
		_out.insert(_out.length(), std::string(_n_tabs, '\t'));
	}
	_out.insert(_out.length(), "}");
}

void zpt::JSONObjT::prettify(ostream& _out, uint _n_tabs) {
	_out << "{" << flush;
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out << ",";
		}
		_out << "\n ";
		_first = false;
		_out << std::string(_n_tabs + 1, '\t') << "\"" << _i.first << "\" : " << flush;
		_i.second->prettify(_out, _n_tabs + 1);
	}
	if (!_first) {		
		_out << "\n" << std::string(_n_tabs, '\t') << flush;
	}
	_out << "}" << flush;
}

/*JSON ARRAY*/
zpt::JSONArrT::JSONArrT() {
}

zpt::JSONArrT::~JSONArrT(){
}

void zpt::JSONArrT::push(JSONElementT& _value) {
	this->push_back(JSONPtr(new JSONElementT(_value)));
}

void zpt::JSONArrT::push(JSONElementT* _value) {
	this->push_back(JSONPtr(_value));
}

void zpt::JSONArrT::pop(int _idx) {
	this->pop((size_t) _idx);
}

void zpt::JSONArrT::pop(const char* _idx) {
	this->pop(std::string(_idx));
}

void zpt::JSONArrT::pop(std::string _idx) {
	size_t _i = 0;
	zpt::fromstr(_idx, &_i);

	assertz(_i < this->size(), "the index of the element you want to remove must be lower than the array size", 500, 0);
	this->erase(this->begin() + _i);
}

void zpt::JSONArrT::pop(size_t _idx) {
	assertz(_idx >= 0, "the index of the element you want to remove must be higher then 0", 500, 0);
	assertz(_idx < this->size(), "the index of the element you want to remove must be lower than the array size", 500, 0);
	this->erase(this->begin() + _idx);
}

void zpt::JSONArrT::sort() {
	std::sort(this->begin(), this->end(), [] (zpt::JSONPtr _lhs, zpt::JSONPtr _rhs) -> bool {
		return _lhs < _rhs;
	});
}

void zpt::JSONArrT::sort(std::function< bool (zpt::JSONPtr, zpt::JSONPtr) > _comparator) {
	std::sort(this->begin(), this->end(), _comparator);
}

zpt::JSONPtr zpt::JSONArrT::getPath(std::string _path, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;
	std::string _remainder;

	getline(_iss, _part, _separator[0]);
	getline(_iss, _remainder);
	zpt::trim(_remainder);
	zpt::JSONPtr _current = (* this)[_part];
	if (!_current->ok()) {
		if (_part == "*" && _remainder.length() != 0) {
			for (auto _a : (* this)) {
				_current = _a->getPath(_remainder, _separator);
				if (_current->ok()) {
					return _current;
				}
			}
		}
		return zpt::undefined;
	}

	if (_remainder.length() == 0) {
		return _current;
	}
	return _current->getPath(_remainder, _separator);
}

void zpt::JSONArrT::setPath(std::string _path, zpt::JSONPtr _value, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;

	getline(_iss, _part, _separator[0]);
	zpt::JSONPtr _current = (* this)[_part];
	if (!_current->ok()) {
		if (_iss.good()) {
			zpt::JSONObj _new;
			_current = mkptr(_new);
			this->push_back(_current);
			_current->setPath(_path.substr(_part.length() + 1), _value, _separator);
		}
		else {
			this->push_back(_value);
		}
	}
	else {
		if (_iss.good()) {
			_current->setPath(_path.substr(_part.length() + 1), _value, _separator);
		}
		else {
			this->pop(_part);
			(* this)[std::stoi(_part)] = _value;			
		}
	}
}

void zpt::JSONArrT::delPath(std::string _path, std::string _separator) {
	std::istringstream _iss(_path);
	std::string _part;

	getline(_iss, _part, _separator[0]);
	zpt::JSONPtr _current = (* this)[_part];
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

zpt::JSONPtr zpt::JSONArrT::clone() {
	zpt::JSONArr _return;
	for (auto _f : * this) {
		_return << _f->clone();
	}	
	return zpt::mkptr(_return);
}

bool zpt::JSONArrT::operator==(zpt::JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f]  == _rhs[_f]) {
			continue;
		}
		return false;
	}
	return true;
}

bool zpt::JSONArrT::operator==(zpt::JSONArr& _rhs) {
	return * this == * _rhs;
}

bool zpt::JSONArrT::operator!=(JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f]  != _rhs[_f]) {
			return true;
		}
	}
	return false;
}

bool zpt::JSONArrT::operator!=(zpt::JSONArr& _rhs) {
	return * this != * _rhs;
}

bool zpt::JSONArrT::operator<(zpt::JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f] < _rhs[_f]) {
			continue;
		}
		return false;
	}
	return true;
}

bool zpt::JSONArrT::operator<(zpt::JSONArr& _rhs) {
	return * this < * _rhs;
}

bool zpt::JSONArrT::operator>(zpt::JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f]  > _rhs[_f]) {
			continue;
		}
		return false;
	}
	return true;
}

bool zpt::JSONArrT::operator>(zpt::JSONArr& _rhs) {
	return * this > * _rhs;
}

bool zpt::JSONArrT::operator<=(zpt::JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f]  <= _rhs[_f]) {
			continue;
		}
		return false;
	}
	return true;
}

bool zpt::JSONArrT::operator<=(zpt::JSONArr& _rhs) {
	return * this <= * _rhs;
}

bool zpt::JSONArrT::operator>=(zpt::JSONArrT& _rhs) {
	for (size_t _f  = 0; _f != this->size(); _f++) {
		if ((* this)[_f]  >= _rhs[_f]) {
			continue;
		}
		return false;
	}
	return true;
}

bool zpt::JSONArrT::operator>=(zpt::JSONArr& _rhs) {
	return * this >= * _rhs;
}

zpt::JSONPtr& zpt::JSONArrT::operator[](int _idx) {
	return (* this)[(size_t) _idx];
}

zpt::JSONPtr& zpt::JSONArrT::operator[](size_t _idx) {
	if (_idx < 0 ||_idx >= this->size()) {
		return zpt::undefined;
	}
	return this->at(_idx);
}

zpt::JSONPtr& zpt::JSONArrT::operator[](const char* _idx) {
	return (* this)[string(_idx)];
}

zpt::JSONPtr& zpt::JSONArrT::operator[](std::string _idx) {
	long _i = -1;
	zpt::fromstr(_idx, & _i);

	if (_i < 0 ||_i >= (long) this->size()) {
		return zpt::undefined;
	}

	return this->at((size_t) _i);
}

void zpt::JSONArrT::stringify(std::string& _out) {
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

void zpt::JSONArrT::stringify(ostream& _out) {
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

void zpt::JSONArrT::prettify(std::string& _out, uint _n_tabs) {
	_out.insert(_out.length(), "[");
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out.insert(_out.length(), ",");
		}
		_out.insert(_out.length(), "\n");
		_first = false;
		_out.insert(_out.length(), std::string(_n_tabs + 1, '\t'));
		_i->prettify(_out, _n_tabs + 1);
	}
	if (!_first) {
		_out.insert(_out.length(), "\n");	
		_out.insert(_out.length(), std::string(_n_tabs, '\t'));
	}
	_out.insert(_out.length(), "]");
}

void zpt::JSONArrT::prettify(ostream& _out, uint _n_tabs) {
	_out << "[" << flush;
	bool _first = true;
	for (auto _i : * this) {
		if (!_first) {
			_out << ",";
		}
		_out << "\n ";
		_first = false;
		_out << std::string(_n_tabs + 1, '\t')<< flush;
		_i->prettify(_out, _n_tabs + 1);
	}
	if (!_first) {
		_out << "\n" << std::string(_n_tabs, '\t');
	}
	_out << "]" << flush;
}

/*JSON POINTER TO ELEMENT*/
zpt::JSONPtr::JSONPtr()  : shared_ptr<JSONElementT>(make_shared<JSONElementT>()) {
}

zpt::JSONPtr::JSONPtr(JSONElementT* _target) : shared_ptr<JSONElementT>(_target) {
}

zpt::JSONPtr::JSONPtr(std::initializer_list<JSONElementT> _init) : shared_ptr<JSONElementT>(new zpt::JSONElementT(_init)) {
}

zpt::JSONPtr::~JSONPtr(){
}

zpt::JSONElementT& zpt::JSONPtr::value() {
	if (this->get() == nullptr) {
		return *(zpt::undefined.get());
	}
	return *(this->get());
}

zpt::JSONPtr::operator std::string() {
	if (this->get() == nullptr) {
		return "";
	}
	string _out;
	switch(this->get()->type()) {
		case zpt::JSObject : {
			this->get()->obj()->stringify(_out);
			break;
		}
		case zpt::JSArray : {
			this->get()->arr()->stringify(_out);
			break;
		}
		case zpt::JSString : {
			_out.assign(this->get()->str().data());
			break;
		}
		case zpt::JSInteger : {
			zpt::tostr(_out, this->get()->intr());
			break;
		}
		case zpt::JSDouble : {
			zpt::tostr(_out, this->get()->dbl());
			break;
		}
		case zpt::JSBoolean : {
			zpt::tostr(_out, this->get()->bln());
			break;
		}
		case zpt::JSNil : {
			_out.assign("");
			break;
		}
		case zpt::JSDate : {
			_out.insert(_out.length(), zpt::timestamp(this->get()->date()));
			break;
		}
		case zpt::JSLambda : {
			_out.assign(this->get()->lbd()->signature());
			break;
		}
	}
	return _out;
}

zpt::JSONPtr::operator zpt::pretty() {
	if (this->get() == nullptr) {
		return zpt::pretty("");
	}
	string _out;
	switch(this->get()->type()) {
		case zpt::JSObject : {
			this->get()->obj()->prettify(_out);
			break;
		}
		case zpt::JSArray : {
			this->get()->arr()->prettify(_out);
			break;
		}
		case zpt::JSString : {
			_out.assign(this->get()->str().data());
			break;
		}
		case zpt::JSInteger : {
			zpt::tostr(_out, this->get()->intr());
			break;
		}
		case zpt::JSDouble : {
			zpt::tostr(_out, this->get()->dbl());
			break;
		}
		case zpt::JSBoolean : {
			zpt::tostr(_out, this->get()->bln());
			break;
		}
		case zpt::JSNil : {
			_out.assign("");
			break;
		}
		case zpt::JSDate : {
			_out.insert(_out.length(), zpt::timestamp(this->get()->date()));
			break;
		}
		case zpt::JSLambda : {
			_out.assign(this->get()->lbd()->signature());
			break;
		}
	}
	return zpt::pretty(_out);
}

zpt::JSONPtr::operator bool() {
	if (this->get() == nullptr) {
		return false;
	}
	switch(this->get()->type()) {
		case zpt::JSObject : {
			return true;
		}
		case zpt::JSArray : {
			return true;
		}
		case zpt::JSString : {
			return this->get()->str().length() != 0;
		}
		case zpt::JSInteger : {
			return (bool) this->get()->intr();
		}
		case zpt::JSDouble : {
			return (bool) this->get()->dbl();
		}
		case zpt::JSBoolean : {
			return this->get()->bln();
		}
		case zpt::JSNil : {
			return false;
		}
		case zpt::JSDate : {
			return (bool) this->get()->date();
		}
		case zpt::JSLambda : {
			return true;
		}
	}
	return false;
}

zpt::JSONPtr::operator int() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zpt::JSObject : {
			return this->get()->obj()->size();
		}
		case zpt::JSArray : {
			return this->get()->arr()->size();
		}
		case zpt::JSString : {
			int _n = 0;
			string _s(this->get()->str().data());
			zpt::fromstr(_s, &_n);
			return _n;
		}
		case zpt::JSInteger : {
			return (int) this->get()->intr();
		}
		case zpt::JSDouble : {
			return (int) this->get()->dbl();
		}
		case zpt::JSBoolean : {
			return (int) this->get()->bln();
		}
		case zpt::JSNil : {
			return 0;
		}
		case zpt::JSDate : {
			return (int) this->get()->date();
		}
		case zpt::JSLambda : {
			return 0;
		}
	}
	return 0;
}

zpt::JSONPtr::operator long() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zpt::JSObject : {
			return this->get()->obj()->size();
		}
		case zpt::JSArray : {
			return this->get()->arr()->size();
		}
		case zpt::JSString : {
			long _n = 0;
			string _s(this->get()->str().data());
			zpt::fromstr(_s, &_n);
			return _n;
		}
		case zpt::JSInteger : {
			return (long) this->get()->intr();
		}
		case zpt::JSDouble : {
			return (long) this->get()->dbl();
		}
		case zpt::JSBoolean : {
			return (long) this->get()->bln();
		}
		case zpt::JSNil : {
			return 0;
		}
		case zpt::JSDate : {
			return (long) this->get()->date();
		}
		case zpt::JSLambda : {
			return 0;
		}
	}
	return 0;
}

zpt::JSONPtr::operator long long() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zpt::JSObject : {
			return this->get()->obj()->size();
		}
		case zpt::JSArray : {
			return this->get()->arr()->size();
		}
		case zpt::JSString : {
			long long _n = 0;
			string _s(this->get()->str().data());
			zpt::fromstr(_s, &_n);
			return _n;
		}
		case zpt::JSInteger : {
			return (long long) this->get()->intr();
		}
		case zpt::JSDouble : {
			return (long long) this->get()->dbl();
		}
		case zpt::JSBoolean : {
			return (long long) this->get()->bln();
		}
		case zpt::JSNil : {
			return 0;
		}
		case zpt::JSDate : {
			return (long long) this->get()->date();
		}
		case zpt::JSLambda : {
			return 0;
		}
	}
	return 0;
}

#ifdef __LP64__
zpt::JSONPtr::operator unsigned int() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zpt::JSObject : {
			return this->get()->obj()->size();
		}
		case zpt::JSArray : {
			return this->get()->arr()->size();
		}
		case zpt::JSString : {
			unsigned int _n = 0;
			string _s(this->get()->str().data());
			zpt::fromstr(_s, &_n);
			return _n;
		}
		case zpt::JSInteger : {
			return (unsigned int) this->get()->intr();
		}
		case zpt::JSDouble : {
			return (unsigned int) this->get()->dbl();
		}
		case zpt::JSBoolean : {
			return (unsigned int) this->get()->bln();
		}
		case zpt::JSNil : {
			return 0;
		}
		case zpt::JSDate : {
			return (unsigned int) this->get()->date();
		}
		case zpt::JSLambda : {
			return 0;
		}
	}
	return 0;
}
#endif

zpt::JSONPtr::operator size_t() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zpt::JSObject : {
			return this->get()->obj()->size();
		}
		case zpt::JSArray : {
			return this->get()->arr()->size();
		}
		case zpt::JSString : {
			size_t _n = 0;
			string _s(this->get()->str().data());
			zpt::fromstr(_s, &_n);
			return _n;
		}
		case zpt::JSInteger : {
			return (size_t) this->get()->intr();
		}
		case zpt::JSDouble : {
			return (size_t) this->get()->dbl();
		}
		case zpt::JSBoolean : {
			return (size_t) this->get()->bln();
		}
		case zpt::JSNil : {
			return 0;
		}
		case zpt::JSDate : {
			return (size_t) this->get()->date();
		}
		case zpt::JSLambda : {
			return 0;
		}
	}
	return 0;
}

zpt::JSONPtr::operator double() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zpt::JSObject : {
			return (double) this->get()->obj()->size();
		}
		case zpt::JSArray : {
			return (double) this->get()->arr()->size();
		}
		case zpt::JSString : {
			double _n = 0;
			string _s(this->get()->str().data());
			zpt::fromstr(_s, &_n);
			return _n;
		}
		case zpt::JSInteger : {
			return (double) this->get()->intr();
		}
		case zpt::JSDouble : {
			return (double) this->get()->dbl();
		}
		case zpt::JSBoolean : {
			return (double) this->get()->bln();
		}
		case zpt::JSNil : {
			return 0;
		}
		case zpt::JSDate : {
			return (double) this->get()->date();
		}
		case zpt::JSLambda : {
			return 0;
		}
	}
	return 0;
}

zpt::JSONPtr::operator zpt::timestamp_t() {
	if (this->get() == nullptr) {
		return 0;
	}
	switch(this->get()->type()) {
		case zpt::JSObject : {
			struct timeval _tp;
			gettimeofday(& _tp, nullptr);
			return _tp.tv_sec * 1000 + _tp.tv_usec / 1000;
		}
		case zpt::JSArray : {
			struct timeval _tp;
			gettimeofday(& _tp, nullptr);
			return _tp.tv_sec * 1000 + _tp.tv_usec / 1000;
		}
		case zpt::JSString : {
			return this->get()->date();
		}
		case zpt::JSInteger : {
			return (zpt::timestamp_t) this->get()->intr();
		}
		case zpt::JSDouble : {
			return (zpt::timestamp_t) this->get()->dbl();
		}
		case zpt::JSBoolean : {
			return (zpt::timestamp_t) this->get()->bln();
		}
		case zpt::JSNil : {
			return 0;
		}
		case zpt::JSDate : {
			return this->get()->date();
		}
		case zpt::JSLambda : {
			return 0;
		}
	}
	return 0;
}

zpt::JSONPtr::operator JSONObj() {
	assertz(this->get() != nullptr && this->get()->type() == zpt::JSObject, std::string("this element is not of type JSObject: ") + ((std::string) * this), 0, 0);
	return this->get()->obj();
}

zpt::JSONPtr::operator JSONArr() {
	assertz(this->get() != nullptr && this->get()->type() == zpt::JSArray, std::string("this element is not of type JSArray: ") + ((std::string) * this), 0, 0);
	return this->get()->arr();
}

zpt::JSONPtr::operator JSONObj&() {
	assertz(this->get() != nullptr && this->get()->type() == zpt::JSObject, std::string("this element is not of type JSObject: ") + ((std::string) * this), 0, 0);
	return this->get()->obj();
}

zpt::JSONPtr::operator JSONArr&() {
	assertz(this->get() != nullptr && this->get()->type() == zpt::JSArray, std::string("this element is not of type JSArray: ") + ((std::string) * this), 0, 0);
	return this->get()->arr();
}

zpt::JSONPtr::operator zpt::lambda() {
	assertz(this->get() != nullptr && this->get()->type() == zpt::JSLambda, std::string("this element is not of type JSLambda: ") + ((std::string) * this), 0, 0);
	return this->get()->lbd();	
}

void zpt::JSONPtr::parse(istream& _in) {
	zpt::JSONParser _p;
	_p.switchRoots(* this);
	_p.switchStreams(_in);
	_p.parse();
}

/*JSON POINTER TO OBJECT*/
zpt::JSONObj::JSONObj() : shared_ptr<JSONObjT>(make_shared<JSONObjT>(JSONObjT())) {
}

zpt::JSONObj::JSONObj(JSONObj& _rhs)  : shared_ptr<JSONObjT>(_rhs) {
}

zpt::JSONObj::JSONObj(JSONObjT* _target) : shared_ptr<JSONObjT>(_target) {
}

zpt::JSONObj::~JSONObj(){
}

zpt::JSONObjT::iterator zpt::JSONObj::begin() {
	return (* this)->begin();
}

zpt::JSONObjT::iterator zpt::JSONObj::end() {
	return (* this)->end();
}

zpt::JSONObj::operator std::string() {
	if (this->get() == nullptr) {
		return "";
	}
	string _out;
	(* this)->stringify(_out);
	return _out;
}


zpt::JSONObj::operator zpt::pretty() {
	if (this->get() == nullptr) {
		return "";
	}
	string _out;
	(* this)->prettify(_out);
	return _out;
}

zpt::JSONObj& zpt::JSONObj::operator<<(std::string _in) {
	(* this)->push(_in);
	return * this;
}

zpt::JSONObj& zpt::JSONObj::operator<<(const char* _in) {
	(* this)->push(_in);
	return * this;
}

zpt::JSONObj& zpt::JSONObj::operator<<(JSONElementT& _in) {
	(* this)->push(_in);
	return * this;
}

/*JSON POINTER TO ARRAY*/
zpt::JSONArr::JSONArr() : shared_ptr<zpt::JSONArrT>(make_shared<zpt::JSONArrT>(JSONArrT())) {
}

zpt::JSONArr::JSONArr(zpt::JSONArr& _rhs)  : shared_ptr<zpt::JSONArrT>(_rhs){
}

zpt::JSONArr::JSONArr(zpt::JSONArrT* _target) : shared_ptr<zpt::JSONArrT>(_target) {
}

zpt::JSONArr::~JSONArr(){
}

zpt::JSONArr::operator std::string() {
	if (this->get() == nullptr) {
		return "";
	}
	string _out;
	(* this)->stringify(_out);
	return _out;
}

zpt::JSONArr::operator zpt::pretty() {
	if (this->get() == nullptr) {
		return "";
	}
	string _out;
	(* this)->prettify(_out);
	return _out;
}

zpt::JSONArrT::iterator zpt::JSONArr::begin() {
	return (* this)->begin();
}

zpt::JSONArrT::iterator zpt::JSONArr::end() {
	return (* this)->end();
}

zpt::JSONArr& zpt::JSONArr::operator<<(JSONElementT& _in) {
	(* this)->push(_in);
	return * this;
}

void zpt::json::stringify(std::string& _str) {
	zpt::utf8::encode(_str, true);
}

zpt::json zpt::get(std::string _path, zpt::json _source) {
	return _source->getPath(_path);
}

zpt::timestamp_t zpt::timestamp(std::string _json_date) {
	if (_json_date.length() == 0) {
		return (zpt::timestamp_t) std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	time_t _n = 0;
	int _ms = 0;
	std::string _s(_json_date.data());
	size_t _idx = _s.rfind(".");	      
	std::string _mss;
	bool _prev_is_zero = true;
	if (_s[_idx + 1] != '0') {
		_mss.push_back(_s[_idx + 1]);
		_prev_is_zero = false;
	}
	if (!_prev_is_zero || _s[_idx + 2] != '0') {
		_mss.push_back(_s[_idx + 2]);
	}
	_mss.push_back(_s[_idx + 3]);
	_s.erase(_idx, 4);
	if (_s.length() < 20) {
		zpt::fromstr(_s, &_n, "%Y-%m-%dT%H:%M:%S", true);
	}
	else if (_s[_idx] == '+' || _s[_idx] == '-') {
		zpt::fromstr(_s, &_n, "%Y-%m-%dT%H:%M:%S%z");
	}
	else {
		zpt::fromstr(_s, &_n, "%Y-%m-%dT%H:%M:%S%Z");
	}
	zpt::fromstr(_mss, &_ms);
	return _n * 1000 + _ms;

}

 zpt::timestamp_t zpt::timestamp(zpt::json _json_date) {
	 return (zpt::timestamp_t) _json_date;
}

std::string zpt::timestamp(zpt::timestamp_t _timestamp) {
	std::string _date = zpt::tostr((size_t) (_timestamp / 1000), "%Y-%m-%dT%H:%M:%S");
	_date.insert(_date.length(), ".");
	size_t _remainder = _timestamp % 1000;
	if (_remainder < 100) {
		_date.insert(_date.length(), "0");
		if (_remainder < 10) {
			_date.insert(_date.length(), "0");
		}
	}
	zpt::tostr(_date, _remainder);
	zpt::tostr(_date, (size_t) (_timestamp / 1000), "%z");
	return _date;
}
