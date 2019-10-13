/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

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

#include <cstdarg>
#include <ostream>
#include <regex>
#include <zapata/json/JSONObj.h>
#include <zapata/json/JSONParser.h>

namespace zpt {
zpt::json undefined;
zpt::json nilptr = undefined;
zpt::json array = zpt::mkptr("1b394520-2fed-4118-b622-940f25b8b35e");
symbol_table __lambdas = zpt::symbol_table(
  new std::map<std::string, std::tuple<std::string, unsigned short, zpt::symbol>>());
} // namespace zpt

zpt::pretty::pretty(std::string _rhs)
  : __underlying{ _rhs } {}

zpt::pretty::pretty(const char* _rhs)
  : __underlying{ _rhs } {}

zpt::pretty::pretty(const pretty& _rhs) {
    (*this) = _rhs;
}

zpt::pretty::pretty(pretty&& _rhs) {
    (*this) = _rhs;
}

auto
zpt::pretty::operator=(const pretty& _rhs) -> pretty& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::pretty::operator=(pretty&& _rhs) -> pretty& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::pretty::operator-> () -> std::string& {
    return this->__underlying;
}

auto zpt::pretty::operator*() -> std::string& {
    return this->__underlying;
}

zpt::pretty::operator std::string() {
    return this->__underlying;
}

zpt::JSONElementT::JSONElementT()
  : __parent(nullptr) {
    this->type(zpt::JSNil);
    this->__target.__nil = nullptr;
}

zpt::JSONElementT::JSONElementT(JSONElementT& _element)
  : __parent(nullptr) {
    this->type(_element.type());
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object = _element.__target.__object;
            break;
        }
        case zpt::JSArray: {
            this->__target.__array = _element.__target.__array;
            break;
        }
        case zpt::JSString: {
            this->__target.__string =
              std::make_shared<std::string>(std::string(_element.str().data()));
            break;
        }
        case zpt::JSInteger: {
            this->__target.__integer = _element.intr();
            break;
        }
        case zpt::JSDouble: {
            this->__target.__double = _element.dbl();
            break;
        }
        case zpt::JSBoolean: {
            this->__target.__boolean = _element.bln();
            break;
        }
        case zpt::JSNil: {
            this->__target.__nil = nullptr;
            break;
        }
        case zpt::JSDate: {
            this->__target.__date = _element.date();
            break;
        }
        case zpt::JSLambda: {
            if (_element.__target.__lambda.get() != nullptr) {
                this->__target.__lambda = _element.__target.__lambda;
            }
            break;
        }
    }
}

zpt::JSONElementT::JSONElementT(std::initializer_list<JSONElementT> _list)
  : __parent(nullptr) {
    bool _is_array =
      (_list.size() > 1 && _list.begin()->__target.__type == zpt::JSString &&
       (*_list.begin()->__target.__string) == "1b394520-2fed-4118-b622-940f25b8b35e");
    bool _is_object =
      (!_is_array && _list.size() % 2 == 0 && _list.begin()->__target.__type == zpt::JSString);
    if (_is_object) {
        size_t _idx = 0;
        size_t* _pidx = &_idx;
        this->type(zpt::JSObject);
        std::for_each(_list.begin(), _list.end(), [&](const zpt::JSONElementT& _element) {
            if ((*_pidx) % 2 == 0) {
                this->__target.__object->push(*_element.__target.__string);
            }
            else {
                zpt::JSONElementT* _other = new zpt::JSONElementT();
                _other->type(_element.__target.__type);
                switch (_other->__target.__type) {
                    case zpt::JSObject: {
                        _other->__target.__object = _element.__target.__object;
                        break;
                    }
                    case zpt::JSArray: {
                        _other->__target.__array = _element.__target.__array;
                        break;
                    }
                    case zpt::JSString: {
                        _other->__target.__string =
                          std::make_shared<std::string>(*_element.__target.__string);
                        break;
                    }
                    case zpt::JSInteger: {
                        _other->__target.__integer = _element.__target.__integer;
                        break;
                    }
                    case zpt::JSDouble: {
                        _other->__target.__double = _element.__target.__double;
                        break;
                    }
                    case zpt::JSBoolean: {
                        _other->__target.__boolean = _element.__target.__boolean;
                        break;
                    }
                    case zpt::JSNil: {
                        _other->__target.__nil = nullptr;
                        break;
                    }
                    case zpt::JSDate: {
                        _other->__target.__date = _element.__target.__date;
                        break;
                    }
                    case zpt::JSLambda: {
                        if (_element.__target.__lambda.get() != nullptr) {
                            _other->__target.__lambda = _element.__target.__lambda;
                        }
                        break;
                    }
                }
                this->__target.__object->push(_other);
            }
            (*_pidx)++;
        });
    }
    else {
        this->type(zpt::JSArray);
        std::for_each((_is_array ? _list.begin() + 1 : _list.begin()),
                      _list.end(),
                      [&](const zpt::JSONElementT& _element) {
                          zpt::JSONElementT* _other = new zpt::JSONElementT();
                          _other->type(_element.__target.__type);
                          switch (_other->__target.__type) {
                              case zpt::JSObject: {
                                  _other->__target.__object = _element.__target.__object;
                                  break;
                              }
                              case zpt::JSArray: {
                                  _other->__target.__array = _element.__target.__array;
                                  break;
                              }
                              case zpt::JSString: {
                                  _other->__target.__string =
                                    std::make_shared<std::string>(*_element.__target.__string);
                                  break;
                              }
                              case zpt::JSInteger: {
                                  _other->__target.__integer = _element.__target.__integer;
                                  break;
                              }
                              case zpt::JSDouble: {
                                  _other->__target.__double = _element.__target.__double;
                                  break;
                              }
                              case zpt::JSBoolean: {
                                  _other->__target.__boolean = _element.__target.__boolean;
                                  break;
                              }
                              case zpt::JSNil: {
                                  _other->__target.__nil = nullptr;
                                  break;
                              }
                              case zpt::JSDate: {
                                  _other->__target.__date = _element.__target.__date;
                                  break;
                              }
                              case zpt::JSLambda: {
                                  if (_element.__target.__lambda.get() != nullptr) {
                                      _other->__target.__lambda = _element.__target.__lambda;
                                  }
                                  break;
                              }
                          }
                          this->__target.__array->push(_other);
                      });
    }
}

zpt::JSONElementT::JSONElementT(zpt::json _value) {
    this->type(_value->type());
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object = _value->obj();
            break;
        }
        case zpt::JSArray: {
            this->__target.__array = _value->arr();
            break;
        }
        case zpt::JSString: {
            this->__target.__string =
              std::make_shared<std::string>(std::string(_value->str().data()));
            break;
        }
        case zpt::JSInteger: {
            this->__target.__integer = _value->intr();
            break;
        }
        case zpt::JSDouble: {
            this->__target.__double = _value->dbl();
            break;
        }
        case zpt::JSBoolean: {
            this->__target.__boolean = _value->bln();
            break;
        }
        case zpt::JSNil: {
            this->__target.__nil = nullptr;
            break;
        }
        case zpt::JSDate: {
            this->__target.__date = _value->date();
            break;
        }
        case zpt::JSLambda: {
            if (_value->lbd().get() != nullptr) {
                this->__target.__lambda = _value->lbd();
            }
            break;
        }
    }
}

zpt::JSONElementT::JSONElementT(JSONObj& _value)
  : __parent(nullptr) {
    this->type(zpt::JSObject);
    this->__target.__object = _value;
}

zpt::JSONElementT::JSONElementT(JSONArr& _value)
  : __parent(nullptr) {
    this->type(zpt::JSArray);
    this->__target.__array = _value;
}

zpt::JSONElementT::JSONElementT(std::string _value)
  : __parent(nullptr) {
    this->type(zpt::JSString);
    this->__target.__string = std::make_shared<std::string>(_value);
}

zpt::JSONElementT::JSONElementT(const char* _value)
  : __parent(nullptr) {
    this->type(zpt::JSString);
    this->__target.__string = std::make_shared<std::string>(std::string(_value));
}

zpt::JSONElementT::JSONElementT(long long _value)
  : __parent(nullptr) {
    this->type(zpt::JSInteger);
    this->__target.__integer = _value;
}

zpt::JSONElementT::JSONElementT(double _value)
  : __parent(nullptr) {
    this->type(zpt::JSDouble);
    this->__target.__double = _value;
}

zpt::JSONElementT::JSONElementT(bool _value)
  : __parent(nullptr) {
    this->type(zpt::JSBoolean);
    this->__target.__boolean = _value;
}

zpt::JSONElementT::JSONElementT(zpt::timestamp_t _value)
  : __parent(nullptr) {
    this->type(zpt::JSDate);
    this->__target.__date = _value;
}

zpt::JSONElementT::JSONElementT(int _value)
  : __parent(nullptr) {
    this->type(zpt::JSInteger);
    this->__target.__integer = _value;
}

zpt::JSONElementT::JSONElementT(size_t _value)
  : __parent(nullptr) {
    this->type(zpt::JSInteger);
    this->__target.__integer = _value;
}

#ifdef __LP64__
zpt::JSONElementT::JSONElementT(unsigned int _value)
  : __parent(nullptr) {
    this->type(zpt::JSInteger);
    this->__target.__integer = _value;
}
#endif

zpt::JSONElementT::JSONElementT(zpt::lambda _value) {
    this->type(zpt::JSLambda);
    this->__target.__lambda = _value;
}

zpt::JSONElementT::~JSONElementT() {}

auto
zpt::JSONElementT::type() -> zpt::JSONType {
    return (zpt::JSONType)this->__target.__type;
}

auto
zpt::JSONElementT::demangle() -> std::string {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            return "object";
        }
        case zpt::JSArray: {
            return "array";
        }
        case zpt::JSString: {
            return "string";
        }
        case zpt::JSInteger: {
            return "integer";
        }
        case zpt::JSDouble: {
            return "number";
        }
        case zpt::JSBoolean: {
            return "boolean";
        }
        case zpt::JSNil: {
            return "null";
        }
        case zpt::JSDate: {
            return "date";
        }
        case zpt::JSLambda: {
            return "lambda";
        }
    }
    return "null";
}

auto
zpt::JSONElementT::type(zpt::JSONType _in) -> void {
    assertz(_in >= 0, "the type must be a valid value", 500, 0);

    if (_in == this->__target.__type) {
        return;
    }

    switch (this->__target.__type) {
        case zpt::JSObject: {
            if ((*this->__target.__object).get() != nullptr) {
                this->__target.__object.~JSONObj();
            }
            break;
        }
        case zpt::JSArray: {
            if ((*this->__target.__array).get() != nullptr) {
                this->__target.__array.~JSONArr();
            }
            break;
        }
        case zpt::JSString: {
            if (this->__target.__string.get() != nullptr) {
                this->__target.__string.~JSONStr();
            }
            break;
        }
        case zpt::JSLambda: {
            if (this->__target.__lambda.get() != nullptr) {
                this->__target.__lambda.~lambda();
            }
            break;
        }
        default: {
            break;
        }
    }
    switch (_in) {
        case zpt::JSObject: {
            new (&this->__target.__object) JSONObj();
            break;
        }
        case zpt::JSArray: {
            new (&this->__target.__array) JSONArr();
            break;
        }
        case zpt::JSString: {
            new (&this->__target.__string) JSONStr();
            break;
        }
        case zpt::JSLambda: {
            new (&this->__target.__lambda) lambda();
            break;
        }
        default: {
            break;
        }
    }

    this->__target.__type = _in;
}

auto
zpt::JSONElementT::value() -> zpt::JSONUnion& {
    return this->__target;
}

auto
zpt::JSONElementT::ok() -> bool {
    return this->__target.__type != zpt::JSNil;
}

auto
zpt::JSONElementT::empty() -> bool {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if ((*this->__target.__object).get() != nullptr) {
                return (***this->__target.__object).size() == 0;
            }
            return true;
        }
        case zpt::JSArray: {
            if ((*this->__target.__array).get() != nullptr) {
                return (***this->__target.__array).size() == 0;
            }
            return true;
        }
        case zpt::JSString: {
            if (this->__target.__string.get() != nullptr) {
                return this->__target.__string->length() == 0;
            }
            return true;
        }
        case zpt::JSInteger: {
            return false;
        }
        case zpt::JSDouble: {
            return false;
        }
        case zpt::JSBoolean: {
            return false;
        }
        case zpt::JSNil: {
            return true;
        }
        case zpt::JSDate: {
            return false;
        }
        case zpt::JSLambda: {
            return false;
        }
    }
    return true;
}

auto
zpt::JSONElementT::nil() -> bool {
    return this->__target.__type == zpt::JSNil;
}

auto
zpt::JSONElementT::size() -> size_t {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if ((*this->__target.__object).get() != nullptr) {
                return (***this->__target.__object).size();
            }
        }
        case zpt::JSArray: {
            if ((*this->__target.__array).get() != nullptr) {
                return (***this->__target.__array).size();
            }
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda: {
            break;
        }
    }
    return 0;
}

auto
zpt::JSONElementT::assign(JSONElementT& _rhs) -> void {
    this->type(_rhs.type());
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if ((*this->__target.__object).get() != nullptr) {
                this->__target.__object.~JSONObj();
            }
            if ((*_rhs.obj()).get() != nullptr) {
                this->__target.__object = _rhs.obj();
            }
            break;
        }
        case zpt::JSArray: {
            if ((*this->__target.__array).get() != nullptr) {
                this->__target.__array.~JSONArr();
            }
            if ((*_rhs.arr()).get() != nullptr) {
                this->__target.__array = _rhs.arr();
            }
            break;
        }
        case zpt::JSString: {
            if (this->__target.__string.get() != nullptr) {
                this->__target.__string.~JSONStr();
            }
            this->__target.__string = std::make_shared<std::string>(std::string(_rhs.str().data()));
            break;
        }
        case zpt::JSInteger: {
            this->__target.__integer = _rhs.intr();
            break;
        }
        case zpt::JSDouble: {
            this->__target.__double = _rhs.dbl();
            break;
        }
        case zpt::JSBoolean: {
            this->__target.__boolean = _rhs.bln();
            break;
        }
        case zpt::JSNil: {
            this->__target.__nil = nullptr;
            break;
        }
        case zpt::JSDate: {
            this->__target.__date = _rhs.date();
            break;
        }
        case zpt::JSLambda: {
            if (this->__target.__lambda.get() != nullptr) {
                this->__target.__lambda.~lambda();
            }
            this->__target.__lambda = _rhs.lbd();
            break;
        }
    }
}

auto
zpt::JSONElementT::parent() -> zpt::JSONElementT* {
    return this->__parent;
}

auto
zpt::JSONElementT::parent(JSONElementT* _parent) -> void {
    this->__parent = _parent;
}

auto
zpt::JSONElementT::is_object() -> bool {
    return this->__target.__type == zpt::JSObject;
}

auto
zpt::JSONElementT::is_array() -> bool {
    return this->__target.__type == zpt::JSArray;
}

auto
zpt::JSONElementT::is_string() -> bool {
    return this->__target.__type == zpt::JSString;
}

auto
zpt::JSONElementT::is_integer() -> bool {
    return this->__target.__type == zpt::JSInteger;
}

auto
zpt::JSONElementT::is_double() -> bool {
    return this->__target.__type == zpt::JSDouble;
}

auto
zpt::JSONElementT::is_number() -> bool {
    return this->__target.__type == zpt::JSInteger || this->__target.__type == zpt::JSDouble;
}

auto
zpt::JSONElementT::is_bool() -> bool {
    return this->__target.__type == zpt::JSBoolean;
}

auto
zpt::JSONElementT::is_date() -> bool {
    return this->__target.__type == zpt::JSDate || this->__target.__type == zpt::JSString;
}

auto
zpt::JSONElementT::is_lambda() -> bool {
    return this->__target.__type == zpt::JSLambda;
}

auto
zpt::JSONElementT::is_nil() -> bool {
    return this->__target.__type == zpt::JSNil;
}

auto
zpt::JSONElementT::is_iterable() -> bool {
    return this->__target.__type == zpt::JSNil;
}

auto
zpt::JSONElementT::obj() -> zpt::JSONObj& {
    assertz(this->__target.__type == zpt::JSObject,
            std::string("this element is not of type JSObject: ") + this->stringify(),
            500,
            1100);
    return this->__target.__object;
}

auto
zpt::JSONElementT::arr() -> zpt::JSONArr& {
    assertz(this->__target.__type == zpt::JSArray,
            std::string("this element is not of type JSArray: ") + this->stringify(),
            500,
            1100);
    return this->__target.__array;
}

auto
zpt::JSONElementT::str() -> std::string {
    assertz(this->__target.__type == zpt::JSString,
            std::string("this element is not of type JSString: ") + this->stringify(),
            500,
            1100);
    return *(this->__target.__string.get());
}

auto
zpt::JSONElementT::intr() -> long long {
    assertz(this->__target.__type == zpt::JSInteger,
            std::string("this element is not of type JSInteger: ") + this->stringify(),
            500,
            1100);
    return this->__target.__integer;
}

auto
zpt::JSONElementT::dbl() -> double {
    assertz(this->__target.__type == zpt::JSDouble,
            std::string("this element is not of type JSDouble: ") + this->stringify(),
            500,
            1100);
    return this->__target.__double;
}

auto
zpt::JSONElementT::bln() -> bool {
    assertz(this->__target.__type == zpt::JSBoolean,
            std::string("this element is not of type JSBoolean: ") + this->stringify(),
            500,
            1100);
    return this->__target.__boolean;
}

auto
zpt::JSONElementT::date() -> zpt::timestamp_t {
    assertz(this->__target.__type == zpt::JSDate || this->__target.__type == zpt::JSString,
            std::string("this element is not of type JSDate: ") + this->stringify(),
            500,
            1100);
    if (this->__target.__type == zpt::JSString) {
        return zpt::timestamp(*(this->__target.__string.get()));
    }
    return this->__target.__date;
}

auto
zpt::JSONElementT::lbd() -> zpt::lambda& {
    assertz(this->__target.__type == zpt::JSLambda,
            std::string("this element is not of type JSLambda: ") + this->stringify(),
            500,
            1100);
    return this->__target.__lambda;
}

auto
zpt::JSONElementT::number() -> double {
    assertz(this->__target.__type == zpt::JSDate || this->__target.__type == zpt::JSInteger ||
              this->__target.__type == zpt::JSDouble || this->__target.__type == zpt::JSBoolean,
            std::string("this element is not of type JSInteger, JSDouble or JSBoolean: ") +
              this->stringify(),
            500,
            1100);
    switch (this->__target.__type) {
        case zpt::JSInteger: {
            return (double)this->__target.__integer;
        }
        case zpt::JSDouble: {
            return this->__target.__double;
        }
        case zpt::JSBoolean: {
            return (double)this->__target.__boolean;
        }
        case zpt::JSDate: {
            return (double)this->__target.__date;
        }
        default: {
            return 0;
        }
    }
    return 0;
}

auto
zpt::JSONElementT::clone() -> zpt::json {
    switch (this->type()) {
        case zpt::JSObject: {
            return this->obj()->clone();
        }
        case zpt::JSArray: {
            return this->arr()->clone();
        }
        case zpt::JSString: {
            std::string _v = this->str();
            return zpt::mkptr(_v);
        }
        case zpt::JSInteger: {
            int _v = this->intr();
            return zpt::mkptr(_v);
        }
        case zpt::JSDouble: {
            double _v = this->dbl();
            return zpt::mkptr(_v);
        }
        case zpt::JSBoolean: {
            bool _v = this->bln();
            return zpt::mkptr(_v);
        }
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            zpt::timestamp_t _v = this->date();
            return zpt::mkptr(_v);
        }
        case zpt::JSLambda: {
            return zpt::json::lambda(this->lbd()->name(), this->lbd()->n_args());
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::operator<<(const char* _in) -> zpt::JSONElementT& {
    (*this) << std::string(_in);
    return *this;
}

auto
zpt::JSONElementT::operator<<(std::string _in) -> zpt::JSONElementT& {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->push(_in);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->push(new zpt::JSONElementT(std::string(_in)));
            break;
        }
        case zpt::JSString: {
            this->__target.__string.get()->assign(_in);
            break;
        }
        default: {
            assertz(this->__target.__type == zpt::JSObject ||
                      this->__target.__type == zpt::JSArray ||
                      this->__target.__type == zpt::JSString,
                    "the type must be a JSObject, JSArray or JSString in order to push "
                    "a std::string",
                    500,
                    0);
            break;
        }
    }
    return *this;
}

auto
zpt::JSONElementT::operator<<(JSONElementT* _in) -> zpt::JSONElementT& {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    if (this->__target.__type == _in->type() && _in->type() != zpt::JSObject &&
        _in->type() != zpt::JSArray) {
        this->assign(*_in);
        return *this;
    }
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->push(_in);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->push(_in);
            break;
        }
        default: {
            assertz(this->__target.__type == zpt::JSObject || this->__target.__type == zpt::JSArray,
                    "the type must be a JSObject, JSArray or the same type of the "
                    "target, in order to push "
                    "JSONElementT*",
                    500,
                    0);
            break;
        }
    }
    return *this;
}

auto
zpt::JSONElementT::operator<<(std::initializer_list<JSONElementT> _list) -> zpt::JSONElementT& {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    zpt::json _other(_list);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (_other->type() == zpt::JSObject) {
                for (auto [_idx, _key, _attribute] : _other) {
                    this->__target.__object->push(_key);
                    this->__target.__object->push(_attribute);
                }
            }
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->push((*_other).get());
            break;
        }
        default: {
            assertz(this->__target.__type == zpt::JSObject || this->__target.__type == zpt::JSArray,
                    "the type must be a JSObject, JSArray or the same type of the "
                    "target, in order to push "
                    "JSONElementT*",
                    500,
                    0);
            break;
        }
    }
    return *this;
}

auto
zpt::JSONElementT::operator==(zpt::JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__object) == *(_in.obj());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__array) == *(_in.arr());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__string.get()) == _in.str();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__integer == _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__double == _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__boolean == _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__date == _in.number();
        }
        case zpt::JSLambda: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return this->__target.__lambda->signature() == _in.lbd()->signature();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator==(zpt::json _rhs) -> bool {
    return *this == _rhs;
}

auto
zpt::JSONElementT::operator!=(JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    if (this->__target.__type != _in.type()) {
        return true;
    }
    switch (this->__target.__type) {
        case zpt::JSObject: {
            return this->__target.__object != _in.obj();
        }
        case zpt::JSArray: {
            return this->__target.__array != _in.arr();
        }
        case zpt::JSString: {
            return *(this->__target.__string.get()) != _in.str();
        }
        case zpt::JSInteger: {
            return this->__target.__integer != _in.intr();
        }
        case zpt::JSDouble: {
            return this->__target.__double != _in.dbl();
        }
        case zpt::JSBoolean: {
            return this->__target.__boolean != _in.bln();
        }
        case zpt::JSNil: {
            return true;
        }
        case zpt::JSDate: {
            return this->__target.__date != _in.date();
        }
        case zpt::JSLambda: {
            return this->__target.__lambda->signature() != _in.lbd()->signature();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator!=(zpt::json _rhs) -> bool {
    return *this != _rhs;
}

auto
zpt::JSONElementT::operator<(zpt::JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__object) < *(_in.obj());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__array) < *(_in.arr());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__string.get()) < _in.str();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__integer < _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__double < _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__boolean < _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__date < _in.number();
        }
        case zpt::JSLambda: {
            return this->__target.__lambda->n_args() < _in.lbd()->n_args();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator<(zpt::json _rhs) -> bool {
    return *this < _rhs;
}

auto
zpt::JSONElementT::operator>(zpt::JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__object) > *(_in.obj());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__array) > *(_in.arr());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__string.get()) > _in.str();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__integer > _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__double > _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__boolean > _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__date > _in.number();
        }
        case zpt::JSLambda: {
            return this->__target.__lambda->n_args() > _in.lbd()->n_args();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator>(zpt::json _rhs) -> bool {
    return *this > _rhs;
}

auto
zpt::JSONElementT::operator<=(zpt::JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__object) <= *(_in.obj());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__array) <= *(_in.arr());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__string.get()) <= _in.str();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__integer <= _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__double <= _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__boolean <= _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__date <= _in.number();
        }
        case zpt::JSLambda: {
            return this->__target.__lambda->n_args() <= _in.lbd()->n_args();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator<=(zpt::json _rhs) -> bool {
    return *this <= _rhs;
}

auto
zpt::JSONElementT::operator>=(zpt::JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__object) >= *(_in.obj());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__array) >= *(_in.arr());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__string.get()) >= _in.str();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__integer >= _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__double >= _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__boolean >= _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__date >= _in.number();
        }
        case zpt::JSLambda: {
            return this->__target.__lambda->n_args() >= _in.lbd()->n_args();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator>=(zpt::json _rhs) -> bool {
    return *this >= _rhs;
}

auto
zpt::JSONElementT::operator+(zpt::json _rhs) -> zpt::json {
    return (*this) + (**_rhs);
}

auto
zpt::JSONElementT::operator+(zpt::JSONElementT& _rhs) -> zpt::json {
    if (this->__target.__type == zpt::JSNil) {
        zpt::json _rrhs = _rhs.clone();
        return _rrhs;
    }
    if (_rhs.__target.__type == zpt::JSNil) {
        zpt::json _lhs = this->clone();
        return _lhs;
    }
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    assertz(this->__target.__type == zpt::JSArray || _rhs.__target.__type == zpt::JSArray ||
              this->__target.__type == _rhs.__target.__type,
            "can't add JSON objects of different types",
            500,
            0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            zpt::json _lhs = this->clone();
            for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                if (_lhs[_key]->type() == zpt::JSObject || _lhs[_key]->type() == zpt::JSArray) {
                    _lhs << _key << (_lhs[_key] + _e);
                }
                else {
                    _lhs << _key << _e;
                }
            }
            return _lhs;
        }
        case zpt::JSArray: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = this->clone();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << _e;
                }
                return _lhs;
            }
            else {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ this }) {
                    _lhs << (_e + _rhs);
                }
                return _lhs;
            }
        }
        case zpt::JSString: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) + _e);
                }
                return _lhs;
            }
            else {
                std::string _lhs((*(this->__target.__string.get())) + _rhs.str());
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSInteger: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) + _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__integer + _rhs.intr();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSDouble: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) + _e);
                }
                return _lhs;
            }
            else {
                double _lhs = this->__target.__double + _rhs.dbl();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSBoolean: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) + _e);
                }
                return _lhs;
            }
            else {
                bool _lhs = this->__target.__boolean || _rhs.bln();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) + _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__date + _rhs.number();
                return zpt::mkptr((zpt::timestamp_t)_lhs);
            }
        }
        case zpt::JSLambda: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::operator-(zpt::json _rhs) -> zpt::json {
    return (*this) - (**_rhs);
}

auto
zpt::JSONElementT::operator-(zpt::JSONElementT& _rhs) -> zpt::json {
    if (this->__target.__type == zpt::JSNil) {
        zpt::json _rrhs = _rhs.clone();
        return _rrhs;
    }
    if (_rhs.__target.__type == zpt::JSNil) {
        zpt::json _lhs = this->clone();
        return _lhs;
    }
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    assertz(this->__target.__type == zpt::JSArray ||
              (this->__target.__type != zpt::JSObject && _rhs.__target.__type == zpt::JSArray) ||
              this->__target.__type == _rhs.__target.__type,
            "can't substract JSON objects of different types",
            500,
            0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            zpt::json _lhs = this->clone();
            for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                _lhs >> _key;
            }
            return _lhs;
        }
        case zpt::JSArray: {
            if (_rhs.__target.__type == zpt::JSArray) {
                assertz((***this->arr()).size() == (***_rhs.arr()).size(),
                        "both arrays must have the same size",
                        500,
                        0);
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << (this[_idx] - _rhs[_idx]);
                }
                return _lhs;
            }
            else {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << (_e - _rhs);
                }
                return _lhs;
            }
        }
        case zpt::JSString: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
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
        case zpt::JSInteger: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) - _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__integer - _rhs.intr();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSDouble: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) - _e);
                }
                return _lhs;
            }
            else {
                double _lhs = this->__target.__double - _rhs.dbl();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSBoolean: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) - _e);
                }
                return _lhs;
            }
            else {
                bool _lhs = this->__target.__boolean & _rhs.bln();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) - _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__date - _rhs.number();
                return zpt::mkptr((zpt::timestamp_t)_lhs);
            }
        }
        case zpt::JSLambda: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::operator/(zpt::json _rhs) -> zpt::json {
    return (*this) / (**_rhs);
}

auto
zpt::JSONElementT::operator/(zpt::JSONElementT& _rhs) -> zpt::json {
    if (this->__target.__type == zpt::JSNil) {
        zpt::json _rrhs = _rhs.clone();
        return _rrhs;
    }
    if (_rhs.__target.__type == zpt::JSNil) {
        zpt::json _lhs = this->clone();
        return _lhs;
    }
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    assertz(this->__target.__type == zpt::JSArray || _rhs.__target.__type == zpt::JSArray ||
              this->__target.__type == _rhs.__target.__type,
            "can't divide JSON objects of different types",
            500,
            0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            assertz(this->__target.__type == zpt::JSObject, "can't divide JSON objects", 500, 0);
        }
        case zpt::JSArray: {
            if (_rhs.__target.__type == zpt::JSArray) {
                assertz((***this->arr()).size() == (***_rhs.arr()).size(),
                        "both arrays must have the same size",
                        500,
                        0);
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << (this[_idx] / _rhs[_idx]);
                }
                return _lhs;
            }
            else {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << (_e / _rhs);
                }
                return _lhs;
            }
        }
        case zpt::JSString: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
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
        case zpt::JSInteger: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) / _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__integer / _rhs.intr();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSDouble: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) / _e);
                }
                return _lhs;
            }
            else {
                double _lhs = this->__target.__double / _rhs.dbl();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSBoolean: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) / _e);
                }
                return _lhs;
            }
            else {
                bool _lhs = this->__target.__boolean / _rhs.bln();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) / _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__date / _rhs.number();
                return zpt::mkptr((zpt::timestamp_t)_lhs);
            }
        }
        case zpt::JSLambda: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::operator|(zpt::json _rhs) -> zpt::json {
    return (*this) | (**_rhs);
}

auto
zpt::JSONElementT::operator|(zpt::JSONElementT& _rhs) -> zpt::json {
    if (this->__target.__type == zpt::JSNil) {
        zpt::json _rrhs = _rhs.clone();
        return _rrhs;
    }
    if (_rhs.__target.__type == zpt::JSNil) {
        zpt::json _lhs = this->clone();
        return _lhs;
    }
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    assertz(this->__target.__type == _rhs.__target.__type,
            "can't add JSON objects of different types",
            500,
            0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            zpt::json _lhs = this->clone();
            for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                if (_lhs[_key]->type() == zpt::JSObject) {
                    _lhs << _key << (_lhs[_key] | _e);
                }
                else {
                    _lhs << _key << _e;
                }
            }
            return _lhs;
        }
        case zpt::JSArray:
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::get_path(std::string _path, std::string _separator) -> zpt::json {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            return this->__target.__object->get_path(_path, _separator);
        }
        case zpt::JSArray: {
            return this->__target.__array->get_path(_path, _separator);
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSDate: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::set_path(std::string _path, zpt::json _value, std::string _separator) -> void {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->set_path(_path, _value, _separator);
            return;
        }
        case zpt::JSArray: {
            this->__target.__array->set_path(_path, _value, _separator);
            return;
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSDate: {
            return;
        }
    }
    return;
}

auto
zpt::JSONElementT::del_path(std::string _path, std::string _separator) -> void {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->del_path(_path, _separator);
            return;
        }
        case zpt::JSArray: {
            this->__target.__array->del_path(_path, _separator);
            return;
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSDate: {
            return;
        }
    }
}

auto
zpt::JSONElementT::flatten() -> zpt::json {
    if (this->type() == zpt::JSObject || this->type() == zpt::JSArray) {
        zpt::json _return = zpt::json::object();
        this->inspect(
          { "$any", "type" },
          [&](std::string _object_path, std::string _key, zpt::JSONElementT& _parent) -> void {
              zpt::json _self = this->get_path(_object_path);
              if (_self->type() != zpt::JSObject && _self->type() != zpt::JSArray) {
                  _return << _object_path << _self;
              }
          });
        return _return;
    }
    else {
        return this->clone();
    }
}

auto
zpt::JSONElementT::inspect(
  zpt::json _pattern,
  std::function<void(std::string, std::string, zpt::JSONElementT&)> _callback,
  zpt::JSONElementT* _parent,
  std::string _key,
  std::string _parent_path) -> void {
    switch (this->type()) {
        case zpt::JSObject: {
            for (auto [_idx, _name, _item] : zpt::json{ this }) {
                if (_pattern->type() == zpt::JSObject && _pattern[_name]->ok()) {
                    _item->inspect(_pattern[_name],
                                   _callback,
                                   this,
                                   _name,
                                   (_parent_path.length() != 0
                                      ? (_parent_path + std::string(".") + _key)
                                      : _key));
                    continue;
                }
                _item->inspect(
                  _pattern,
                  _callback,
                  this,
                  _name,
                  (_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key));
            }
            if (_pattern["$any"]->ok() && _parent_path.length() != 0) {
                _callback(_parent_path + std::string(".") + _key, _key, *_parent);
            }
            else {
                if (*this == _pattern) {
                    _callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key)
                                                          : _key),
                              _key,
                              *_parent);
                }
            }
            break;
        }
        case zpt::JSArray: {
            for (auto [_idx, _name, _item] : zpt::json{ this }) {
                _item->inspect(
                  _pattern,
                  _callback,
                  this,
                  std::to_string(_idx),
                  (_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key));
            }
            if (_pattern["$any"]->ok() && _parent_path.length() != 0) {
                _callback(
                  (_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key),
                  _key,
                  *_parent);
            }
            else {
                if (*this == _pattern) {
                    _callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key)
                                                          : _key),
                              _key,
                              *_parent);
                }
            }
            break;
        }
        default: {
            if (_pattern["$regexp"]->ok()) {
                std::regex _rgx(((std::string)_pattern["$regexp"]));
                std::string _exp;
                this->stringify(_exp);
                if (std::regex_match(_exp, _rgx)) {
                    _callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key)
                                                          : _key),
                              _key,
                              *_parent);
                }
            }
            else if (_pattern["$any"]->ok()) {
                _callback(
                  (_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key),
                  _key,
                  *_parent);
            }
            else {
                if (*this == _pattern) {
                    _callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key)
                                                          : _key),
                              _key,
                              *_parent);
                }
            }
            break;
        }
    }
}

auto
zpt::JSONElementT::stringify(std::ostream& _out) -> void {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->stringify(_out);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->stringify(_out);
            break;
        }
        case zpt::JSString: {
            std::string _str(this->str());
            zpt::json::stringify(_str);
            _out << "\"" << _str << "\"" << std::flush;
            break;
        }
        case zpt::JSInteger: {
            _out << this->__target.__integer << std::flush;
            break;
        }
        case zpt::JSDouble: {
            _out << this->__target.__double << std::flush;
            break;
        }
        case zpt::JSBoolean: {
            _out << (this->__target.__boolean ? "true" : "false") << std::flush;
            break;
        }
        case zpt::JSNil: {
            _out << "null" << std::flush;
            break;
        }
        case zpt::JSDate: {
            _out << "\"" << zpt::timestamp(this->__target.__date) << "\"" << std::flush;
            break;
        }
        case zpt::JSLambda: {
            _out << this->__target.__lambda->signature() << std::flush;
            break;
        }
    }
}

auto
zpt::JSONElementT::stringify(std::string& _out) -> void {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->stringify(_out);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->stringify(_out);
            break;
        }
        case zpt::JSString: {
            std::string _str(this->str());
            zpt::json::stringify(_str);
            _out.insert(_out.length(), "\"");
            _out.insert(_out.length(), _str);
            _out.insert(_out.length(), "\"");
            break;
        }
        case zpt::JSInteger: {
            zpt::tostr(_out, this->__target.__integer);
            break;
        }
        case zpt::JSDouble: {
            zpt::tostr(_out, this->__target.__double);
            break;
        }
        case zpt::JSBoolean: {
            zpt::tostr(_out, this->__target.__boolean);
            break;
        }
        case zpt::JSNil: {
            _out.insert(_out.length(), "null");
            break;
        }
        case zpt::JSDate: {
            _out.insert(_out.length(), "\"");
            _out.insert(_out.length(), zpt::timestamp(this->__target.__date));
            _out.insert(_out.length(), "\"");
            break;
        }
        case zpt::JSLambda: {
            _out.insert(_out.length(), this->__target.__lambda->signature());
            break;
        }
    }
}

auto
zpt::JSONElementT::stringify() -> std::string {
    std::string _out;
    this->stringify(_out);
    return _out;
}

auto
zpt::JSONElementT::prettify(std::ostream& _out, uint _n_tabs) -> void {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->prettify(_out, _n_tabs);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->prettify(_out, _n_tabs);
            break;
        }
        case zpt::JSString: {
            std::string _str(this->str());
            zpt::json::stringify(_str);
            _out << "\"" << _str << "\"" << std::flush;
            break;
        }
        case zpt::JSInteger: {
            _out << this->__target.__integer << std::flush;
            break;
        }
        case zpt::JSDouble: {
            _out << this->__target.__double << std::flush;
            break;
        }
        case zpt::JSBoolean: {
            _out << (this->__target.__boolean ? "true" : "false") << std::flush;
            break;
        }
        case zpt::JSNil: {
            _out << "null" << std::flush;
            break;
        }
        case zpt::JSDate: {
            _out << "\"" << zpt::timestamp(this->__target.__date) << "\"" << std::flush;
            break;
        }
        case zpt::JSLambda: {
            _out << this->__target.__lambda->signature() << std::flush;
            break;
        }
    }
    if (_n_tabs == 0) {
        _out << std::endl << std::flush;
    }
}

auto
zpt::JSONElementT::prettify(std::string& _out, uint _n_tabs) -> void {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->prettify(_out, _n_tabs);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->prettify(_out, _n_tabs);
            break;
        }
        case zpt::JSString: {
            std::string _str(this->str());
            zpt::json::stringify(_str);
            _out.insert(_out.length(), "\"");
            _out.insert(_out.length(), _str);
            _out.insert(_out.length(), "\"");
            break;
        }
        case zpt::JSInteger: {
            zpt::tostr(_out, this->__target.__integer);
            break;
        }
        case zpt::JSDouble: {
            zpt::tostr(_out, this->__target.__double);
            break;
        }
        case zpt::JSBoolean: {
            zpt::tostr(_out, this->__target.__boolean);
            break;
        }
        case zpt::JSNil: {
            _out.insert(_out.length(), "null");
            break;
        }
        case zpt::JSDate: {
            _out.insert(_out.length(), "\"");
            _out.insert(_out.length(), zpt::timestamp(this->__target.__date));
            _out.insert(_out.length(), "\"");
            break;
        }
        case zpt::JSLambda: {
            _out.insert(_out.length(), this->__target.__lambda->signature());
            break;
        }
    }
    if (_n_tabs == 0) {
        _out.insert(_out.length(), "\n");
    }
}

auto
zpt::JSONElementT::element(size_t _pos) -> std::tuple<size_t, std::string, zpt::json> {
    switch (this->__target.__type) {
        case zpt::JSObject:
            return std::make_tuple(
              _pos, this->__target.__object->key_for(_pos), this->__target.__object[_pos]);
        case zpt::JSArray:
            return std::make_tuple(_pos, std::to_string(_pos), this->__target.__array[_pos]);
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda:
            break;
    }
    return std::make_tuple(0, "", zpt::json{ this });
}

/*JSON CONTEXT*/
zpt::JSONContext::JSONContext(void* _target)
  : __target(_target) {}

zpt::JSONContext::~JSONContext() {
    this->__target = nullptr;
}

auto
zpt::JSONContext::unpack() -> void* {
    return this->__target;
}

zpt::context::context(void* _target)
  : __underlying{ std::make_shared<zpt::JSONContext>(zpt::JSONContext(_target)) } {}

zpt::context::~context() {}

zpt::context::context(const context& _rhs) {
    (*this) = _rhs;
}

zpt::context::context(context&& _rhs) {
    (*this) = _rhs;
}

auto zpt::context::operator-> () -> std::shared_ptr<zpt::JSONContext>& {
    return this->__underlying;
}

auto zpt::context::operator*() -> std::shared_ptr<zpt::JSONContext>& {
    return this->__underlying;
}

auto
zpt::context::operator=(const zpt::context& _rhs) -> zpt::context& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::context::operator=(zpt::context&& _rhs) -> zpt::context& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

/*JSON LAMBDA */
zpt::lambda::lambda()
  : std::shared_ptr<zpt::JSONLambda>(std::make_shared<zpt::JSONLambda>(zpt::JSONLambda())) {}

zpt::lambda::lambda(std::shared_ptr<zpt::JSONLambda> _target)
  : std::shared_ptr<zpt::JSONLambda>(_target) {}

zpt::lambda::lambda(zpt::lambda& _target)
  : std::shared_ptr<zpt::JSONLambda>(_target) {}

zpt::lambda::lambda(zpt::JSONLambda* _target)
  : std::shared_ptr<zpt::JSONLambda>(_target) {}

zpt::lambda::lambda(std::string _signature)
  : std::shared_ptr<zpt::JSONLambda>(new zpt::JSONLambda(_signature)) {}

zpt::lambda::lambda(std::string _name, unsigned short _n_args)
  : std::shared_ptr<zpt::JSONLambda>(new zpt::JSONLambda(_name, _n_args)) {}

zpt::lambda::~lambda() {}

auto
zpt::lambda::operator()(zpt::json _args, zpt::context _ctx) -> zpt::json {
    return this->get()->call(_args, _ctx);
}

auto
zpt::lambda::parse(std::string _signature) -> std::tuple<std::string, unsigned short> {
    size_t _lpar = _signature.find("(");
    size_t _rpar = _signature.find(")");
    size_t _comma = _signature.find(",");

    assertz(_lpar != std::string::npos && _rpar != std::string::npos && _comma != std::string::npos,
            "lambda signature format not recognized",
            412,
            0);

    std::string _name(_signature.substr(_lpar + 1, _comma - _lpar - 1));
    std::string _args(_signature.substr(_comma + 1, _rpar - _comma - 1));
    zpt::replace(_name, "\"", "");
    zpt::trim(_name);
    zpt::trim(_args);
    unsigned short _n_args = std::stoi(_args);
    return std::make_tuple(_name, _n_args);
}

auto
zpt::lambda::stringify(std::string _name, unsigned short _n_args) -> std::string {
    return std::string("lambda(\"") + _name + std::string("\",") + std::to_string(_n_args) +
           std::string(")");
}

auto
zpt::lambda::add(std::string _signature, zpt::symbol _lambda) -> void {
    try {
        zpt::lambda::find(_signature);
        assertz(true, "lambda already defined", 412, 0);
    }
    catch (zpt::assertion& _e) {
    }
    std::tuple<std::string, unsigned short> _parsed = zpt::lambda::parse(_signature);
    zpt::__lambdas->insert(std::make_pair(
      _signature, std::make_tuple(std::get<0>(_parsed), std::get<1>(_parsed), _lambda)));
}

auto
zpt::lambda::add(std::string _name, unsigned short _n_args, zpt::symbol _lambda) -> void {
    try {
        zpt::lambda::find(_name, _n_args);
        assertz(true, "lambda already defined", 412, 0);
    }
    catch (zpt::assertion& _e) {
    }
    std::string _signature(zpt::lambda::stringify(_name, _n_args));
    zpt::__lambdas->insert(std::make_pair(_signature, std::make_tuple(_name, _n_args, _lambda)));
}

auto
zpt::lambda::call(std::string _name, zpt::json _args, zpt::context _ctx) -> zpt::json {
    assertz(_args->type() == zpt::JSArray, "second argument must be a JSON array", 412, 0);
    zpt::symbol _f = zpt::lambda::find(_name, (***_args->arr()).size());
    return _f(_args, (***_args->arr()).size(), _ctx);
}

auto
zpt::lambda::find(std::string _signature) -> zpt::symbol {
    auto _found = zpt::__lambdas->find(_signature);
    assertz(_found != zpt::__lambdas->end(),
            std::string("symbol for ") + _signature + std::string(" was not found"),
            404,
            0);
    return std::get<2>(_found->second);
}

auto
zpt::lambda::find(std::string _name, unsigned short _n_args) -> zpt::symbol {
    std::string _signature = zpt::lambda::stringify(_name, _n_args);
    return zpt::lambda::find(_signature);
}

zpt::JSONLambda::JSONLambda()
  : __name("")
  , __n_args(0) {}

zpt::JSONLambda::JSONLambda(std::string _signature) {
    std::tuple<std::string, unsigned short> _parsed = zpt::lambda::parse(_signature);
    this->__name = std::get<0>(_parsed);
    this->__n_args = std::get<1>(_parsed);
}

zpt::JSONLambda::JSONLambda(std::string _name, unsigned short _n_args)
  : __name(_name)
  , __n_args(_n_args) {}

zpt::JSONLambda::~JSONLambda() {}

auto
zpt::JSONLambda::name() -> std::string {
    return this->__name;
}

auto
zpt::JSONLambda::n_args() -> unsigned short {
    return this->__n_args;
}

auto
zpt::JSONLambda::signature() -> std::string {
    return zpt::lambda::stringify(this->__name, this->__n_args);
}

auto
zpt::JSONLambda::call(zpt::json _args, zpt::context _ctx) -> zpt::json {
    return zpt::lambda::call(this->__name, _args, _ctx);
}

/*JSON OBJECT*/
zpt::JSONObjT::JSONObjT() {}

zpt::JSONObjT::~JSONObjT() {}

auto
zpt::JSONObjT::push(std::string _name) -> JSONObjT& {
    if (this->__name.length() == 0) {
        this->__name_to_index.push_back(_name);
        this->__name.assign(_name.data());
    }
    else {
        this->pop(this->__name);
        (**this).insert(
          std::make_pair(std::string(this->__name.data()),
                         std::make_tuple(zpt::json(new JSONElementT(std::string(_name.data()))),
                                         this->__name_to_index.size() - 1)));
        this->__name.clear();
    }
    return (*this);
}

auto
zpt::JSONObjT::push(JSONElementT& _value) -> JSONObjT& {
    assertz(this->__name.length() != 0, "you must pass a field name first", 500, 0);
    this->pop(this->__name);
    (**this).insert(std::make_pair(
      std::string(this->__name.data()),
      std::make_tuple(zpt::json(new JSONElementT(_value)), this->__name_to_index.size() - 1)));
    this->__name.clear();
    return (*this);
}

auto
zpt::JSONObjT::push(JSONElementT* _value) -> JSONObjT& {
    assertz(this->__name.length() != 0, "you must pass a field name first", 500, 0);
    this->pop(this->__name);
    (**this).insert(std::make_pair(
      std::string(this->__name.data()),
      std::make_tuple(zpt::json(new JSONElementT(_value)), this->__name_to_index.size() - 1)));
    this->__name.clear();
    return (*this);
}

auto
zpt::JSONObjT::pop(int _idx) -> JSONObjT& {
    assertz(this->__name_to_index.size() < static_cast<size_t>(_idx), "no such index", 500, 0);
    return this->pop(this->__name_to_index[_idx]);
}

auto
zpt::JSONObjT::pop(size_t _idx) -> JSONObjT& {
    assertz(this->__name_to_index.size() < _idx, "no such index", 500, 0);
    return this->pop(this->__name_to_index[_idx]);
}

auto
zpt::JSONObjT::pop(const char* _name) -> JSONObjT& {
    return this->pop(std::string(_name));
}

auto
zpt::JSONObjT::pop(std::string _name) -> JSONObjT& {
    auto _found = (**this).find(_name);
    if (_found != (**this).end()) {
        (**this).erase(_found);
    }
    this->__name_to_index.erase(
      std::remove(this->__name_to_index.begin(), this->__name_to_index.end(), _name),
      this->__name_to_index.end());
    return (*this);
}

auto
zpt::JSONObjT::key_for(size_t _idx) -> std::string {
    assertz(this->__name_to_index.size() < _idx, "no such index", 500, 0);
    return this->__name_to_index[_idx];
}

auto
zpt::JSONObjT::index_for(std::string _name) -> size_t {
    auto _found = (**this).find(_name);
    if (_found != (**this).end()) {
        return std::get<1>(_found->second);
    }
    assertz(_found != (**this).end(), "no such key", 500, 0);
    return 0;
}

auto
zpt::JSONObjT::get_path(std::string _path, std::string _separator) -> zpt::json {
    std::istringstream _iss(_path);
    std::string _part;
    std::string _remainder;

    getline(_iss, _part, _separator[0]);
    getline(_iss, _remainder);
    zpt::trim(_remainder);
    zpt::json _current = (*this)[_part];
    if (!_current->ok()) {
        if (_part == "*" && _remainder.length() != 0) {
            for (auto _a : (**this)) {
                _current = std::get<0>(_a.second)->get_path(_remainder, _separator);
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
    return _current->get_path(_remainder, _separator);
}

auto
zpt::JSONObjT::set_path(std::string _path, zpt::json _value, std::string _separator) -> JSONObjT& {
    std::istringstream _iss(_path);
    std::string _part;

    getline(_iss, _part, _separator[0]);
    zpt::json _current = (*this)[_part];
    if (!_current->ok()) {
        if (_iss.good()) {
            zpt::JSONObj _new;
            _current = zpt::mkptr(_new);
            this->push(_part);
            this->push(_current);
            _current->set_path(_path.substr(_part.length() + 1), _value, _separator);
        }
        else {
            this->push(_part);
            this->push(_value);
        }
    }
    else {
        if (_iss.good()) {
            _current->set_path(_path.substr(_part.length() + 1), _value, _separator);
        }
        else {
            this->push(_part);
            this->push(_value);
        }
    }
    return (*this);
}

auto
zpt::JSONObjT::del_path(std::string _path, std::string _separator) -> JSONObjT& {
    std::istringstream _iss(_path);
    std::string _part;

    getline(_iss, _part, _separator[0]);
    zpt::json _current = (*this)[_part];
    if (!_current->ok()) {
        return (*this);
    }

    while (_iss.good()) {
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
            return (*this);
        }
    }
    return (*this);
}

auto
zpt::JSONObjT::clone() -> zpt::json {
    zpt::JSONObj _return;
    for (auto _f : this->__underlying) {
        _return << _f.first << std::get<0>(_f.second)->clone();
    }
    return zpt::mkptr(_return);
}

auto zpt::JSONObjT::operator-> () -> std::map<std::string, std::tuple<zpt::json, size_t>>& {
    return this->__underlying;
}

auto zpt::JSONObjT::operator*() -> std::map<std::string, std::tuple<zpt::json, size_t>>& {
    return this->__underlying;
}

auto
zpt::JSONObjT::operator==(zpt::JSONObjT& _rhs) -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) {
            return false;
        }
        if (std::get<0>(_found->second) == std::get<0>(_f.second)) {
            continue;
        }
        return false;
    }
    return true;
}

auto
zpt::JSONObjT::operator==(zpt::JSONObj& _rhs) -> bool {
    return *this == *_rhs;
}

auto
zpt::JSONObjT::operator!=(JSONObjT& _rhs) -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) {
            return true;
        }
        if (std::get<0>(_found->second) != std::get<0>(_f.second)) {
            return true;
        }
    }
    return false;
}

auto
zpt::JSONObjT::operator!=(zpt::JSONObj& _rhs) -> bool {
    return *this != *_rhs;
}

auto
zpt::JSONObjT::operator<(zpt::JSONObjT& _rhs) -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) {
            return false;
        }
        if (std::get<0>(_found->second) < std::get<0>(_f.second)) {
            continue;
        }
        return false;
    }
    return true;
}

auto
zpt::JSONObjT::operator<(zpt::JSONObj& _rhs) -> bool {
    return *this < *_rhs;
}

auto
zpt::JSONObjT::operator>(zpt::JSONObjT& _rhs) -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) {
            return false;
        }
        if (std::get<0>(_found->second) > std::get<0>(_f.second)) {
            continue;
        }
        return false;
    }
    return true;
}

auto
zpt::JSONObjT::operator>(zpt::JSONObj& _rhs) -> bool {
    return *this > *_rhs;
}

auto
zpt::JSONObjT::operator<=(zpt::JSONObjT& _rhs) -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) {
            return false;
        }
        if (std::get<0>(_found->second) <= std::get<0>(_f.second)) {
            continue;
        }
        return false;
    }
    return true;
}

auto
zpt::JSONObjT::operator<=(zpt::JSONObj& _rhs) -> bool {
    return *this <= *_rhs;
}

auto
zpt::JSONObjT::operator>=(zpt::JSONObjT& _rhs) -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) {
            return false;
        }
        if (std::get<0>(_found->second) >= std::get<0>(_f.second)) {
            continue;
        }
        return false;
    }
    return true;
}

auto
zpt::JSONObjT::operator>=(zpt::JSONObj& _rhs) -> bool {
    return *this >= *_rhs;
}

auto zpt::JSONObjT::operator[](int _idx) -> zpt::json& {
    assertz(this->__name_to_index.size() < static_cast<size_t>(_idx), "no such index", 500, 0);
    return (*this)[this->__name_to_index[_idx]];
}

auto zpt::JSONObjT::operator[](size_t _idx) -> zpt::json& {
    assertz(this->__name_to_index.size() < _idx, "no such index", 500, 0);
    return (*this)[this->__name_to_index[_idx]];
}

auto zpt::JSONObjT::operator[](const char* _idx) -> zpt::json& {
    return (*this)[std::string(_idx)];
}

auto zpt::JSONObjT::operator[](std::string _idx) -> zpt::json& {
    auto _found = this->__underlying.find(_idx);
    if (_found != this->__underlying.end()) {
        return std::get<0>(_found->second);
    }
    return zpt::undefined;
}

auto
zpt::JSONObjT::stringify(std::string& _out) -> JSONObjT& {
    _out.insert(_out.length(), "{");
    bool _first = true;
    for (size_t _idx = 0; _idx != this->__name_to_index.size(); ++_idx) {
        std::string _name{ this->__name_to_index[_idx] };
        if (!_first) {
            _out.insert(_out.length(), ",");
        }
        _first = false;
        _out.insert(_out.length(), "\"");
        _out.insert(_out.length(), _name);
        _out.insert(_out.length(), "\":");
        (*this)[_name]->stringify(_out);
    }
    _out.insert(_out.length(), "}");
    return (*this);
}

auto
zpt::JSONObjT::stringify(std::ostream& _out) -> JSONObjT& {
    _out << "{" << std::flush;
    bool _first = true;
    for (size_t _idx = 0; _idx != this->__name_to_index.size(); ++_idx) {
        std::string _name{ this->__name_to_index[_idx] };
        if (!_first) {
            _out << ",";
        }
        _first = false;
        _out << "\"" << _name << "\":" << std::flush;
        (*this)[_name]->stringify(_out);
    }
    _out << "}" << std::flush;
    return (*this);
}

auto
zpt::JSONObjT::prettify(std::string& _out, uint _n_tabs) -> JSONObjT& {
    _out.insert(_out.length(), "{");
    bool _first = true;
    for (size_t _idx = 0; _idx != this->__name_to_index.size(); ++_idx) {
        std::string _name{ this->__name_to_index[_idx] };
        if (!_first) {
            _out.insert(_out.length(), ",");
        }
        _out.insert(_out.length(), "\n");
        _first = false;
        _out.insert(_out.length(), std::string(_n_tabs + 1, '\t'));
        _out.insert(_out.length(), "\"");
        _out.insert(_out.length(), _name);
        _out.insert(_out.length(), "\" : ");
        (*this)[_name]->prettify(_out, _n_tabs + 1);
    }
    if (!_first) {
        _out.insert(_out.length(), "\n");
        _out.insert(_out.length(), std::string(_n_tabs, '\t'));
    }
    _out.insert(_out.length(), "}");
    return (*this);
}

auto
zpt::JSONObjT::prettify(std::ostream& _out, uint _n_tabs) -> JSONObjT& {
    _out << "{" << std::flush;
    bool _first = true;
    for (size_t _idx = 0; _idx != this->__name_to_index.size(); ++_idx) {
        std::string _name{ this->__name_to_index[_idx] };
        if (!_first) {
            _out << ",";
        }
        _out << "\n ";
        _first = false;
        _out << std::string(_n_tabs + 1, '\t') << "\"" << _name << "\" : " << std::flush;
        (*this)[_name]->prettify(_out, _n_tabs + 1);
    }
    if (!_first) {
        _out << "\n" << std::string(_n_tabs, '\t') << std::flush;
    }
    _out << "}" << std::flush;
    return (*this);
}

/*JSON ARRAY*/
zpt::JSONArrT::JSONArrT() {}

zpt::JSONArrT::~JSONArrT() {}

auto
zpt::JSONArrT::push(JSONElementT& _value) -> JSONArrT& {
    this->__underlying.push_back(zpt::json(new JSONElementT(_value)));
    return (*this);
}

auto
zpt::JSONArrT::push(JSONElementT* _value) -> JSONArrT& {
    this->__underlying.push_back(zpt::json(_value));
    return (*this);
}

auto
zpt::JSONArrT::pop(int _idx) -> JSONArrT& {
    return this->pop((size_t)_idx);
}

auto
zpt::JSONArrT::pop(const char* _idx) -> JSONArrT& {
    return this->pop(std::string(_idx));
}

auto
zpt::JSONArrT::pop(std::string _idx) -> JSONArrT& {
    long _i = -1;
    zpt::fromstr(_idx, &_i);
    if (_i > 0) {
        assertz((size_t)_i < this->__underlying.size(),
                "the index of the element you want to "
                "remove must be lower than the array "
                "size",
                500,
                0);
        this->__underlying.erase(this->__underlying.begin() + _i);
    }
    return (*this);
}

auto
zpt::JSONArrT::pop(size_t _idx) -> JSONArrT& {
    assertz(_idx >= 0, "the index of the element you want to remove must be higher then 0", 500, 0);
    assertz(_idx < this->__underlying.size(),
            "the index of the element you want to remove "
            "must be lower than the array size",
            500,
            0);
    this->__underlying.erase(this->__underlying.begin() + _idx);
    return (*this);
}

auto
zpt::JSONArrT::sort() -> JSONArrT& {
    std::sort(this->__underlying.begin(),
              this->__underlying.end(),
              [](zpt::json _lhs, zpt::json _rhs) -> bool { return _lhs < _rhs; });
    return (*this);
}

auto
zpt::JSONArrT::sort(std::function<bool(zpt::json, zpt::json)> _comparator) -> JSONArrT& {
    std::sort(this->__underlying.begin(), this->__underlying.end(), _comparator);
    return (*this);
}

auto
zpt::JSONArrT::get_path(std::string _path, std::string _separator) -> zpt::json {
    std::istringstream _iss(_path);
    std::string _part;
    std::string _remainder;

    getline(_iss, _part, _separator[0]);
    getline(_iss, _remainder);
    zpt::trim(_remainder);
    zpt::json _current = (*this)[_part];
    if (!_current->ok()) {
        if (_part == "*" && _remainder.length() != 0) {
            zpt::json _return = zpt::json::array();
            for (auto _a : this->__underlying) {
                _current = _a->get_path(_remainder, _separator);
                if (_current->ok()) {
                    _return << _current;
                }
            }
            return _return;
        }
        return zpt::undefined;
    }

    if (_remainder.length() == 0) {
        return _current;
    }
    return _current->get_path(_remainder, _separator);
}

auto
zpt::JSONArrT::set_path(std::string _path, zpt::json _value, std::string _separator) -> JSONArrT& {
    std::istringstream _iss(_path);
    std::string _part;

    getline(_iss, _part, _separator[0]);
    zpt::json _current = (*this)[_part];
    if (!_current->ok()) {
        if (_iss.good()) {
            zpt::JSONObj _new;
            _current = mkptr(_new);
            this->__underlying.push_back(_current);
            _current->set_path(_path.substr(_part.length() + 1), _value, _separator);
        }
        else {
            this->__underlying.push_back(_value);
        }
    }
    else {
        if (_iss.good()) {
            _current->set_path(_path.substr(_part.length() + 1), _value, _separator);
        }
        else {
            this->pop(_part);
            (*this)[std::stoi(_part)] = _value;
        }
    }
    return (*this);
}

auto
zpt::JSONArrT::del_path(std::string _path, std::string _separator) -> JSONArrT& {
    std::istringstream _iss(_path);
    std::string _part;

    getline(_iss, _part, _separator[0]);
    zpt::json _current = (*this)[_part];
    if (!_current->ok()) {
        return (*this);
    }

    while (_iss.good()) {
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
            return (*this);
        }
    }
    return (*this);
}

auto
zpt::JSONArrT::clone() -> zpt::json {
    zpt::JSONArr _return;
    for (auto _f : this->__underlying) {
        _return << _f->clone();
    }
    return zpt::mkptr(_return);
}

auto zpt::JSONArrT::operator-> () -> std::vector<zpt::json>& {
    return this->__underlying;
}

auto zpt::JSONArrT::operator*() -> std::vector<zpt::json>& {
    return this->__underlying;
}

auto
zpt::JSONArrT::operator==(zpt::JSONArrT& _rhs) -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] == _rhs[_f]) {
            continue;
        }
        return false;
    }
    return true;
}

auto
zpt::JSONArrT::operator==(zpt::JSONArr& _rhs) -> bool {
    return *this == *_rhs;
}

auto
zpt::JSONArrT::operator!=(JSONArrT& _rhs) -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] != _rhs[_f]) {
            return true;
        }
    }
    return false;
}

auto
zpt::JSONArrT::operator!=(zpt::JSONArr& _rhs) -> bool {
    return *this != *_rhs;
}

auto
zpt::JSONArrT::operator<(zpt::JSONArrT& _rhs) -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] < _rhs[_f]) {
            continue;
        }
        return false;
    }
    return true;
}

auto
zpt::JSONArrT::operator<(zpt::JSONArr& _rhs) -> bool {
    return *this < *_rhs;
}

auto
zpt::JSONArrT::operator>(zpt::JSONArrT& _rhs) -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] > _rhs[_f]) {
            continue;
        }
        return false;
    }
    return true;
}

auto
zpt::JSONArrT::operator>(zpt::JSONArr& _rhs) -> bool {
    return *this > *_rhs;
}

auto
zpt::JSONArrT::operator<=(zpt::JSONArrT& _rhs) -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] <= _rhs[_f]) {
            continue;
        }
        return false;
    }
    return true;
}

auto
zpt::JSONArrT::operator<=(zpt::JSONArr& _rhs) -> bool {
    return *this <= *_rhs;
}

auto
zpt::JSONArrT::operator>=(zpt::JSONArrT& _rhs) -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] >= _rhs[_f]) {
            continue;
        }
        return false;
    }
    return true;
}

auto
zpt::JSONArrT::operator>=(zpt::JSONArr& _rhs) -> bool {
    return *this >= *_rhs;
}

auto zpt::JSONArrT::operator[](int _idx) -> zpt::json& {
    return (*this)[(size_t)_idx];
}

auto zpt::JSONArrT::operator[](size_t _idx) -> zpt::json& {
    if (_idx < 0 || _idx >= this->__underlying.size()) {
        return zpt::undefined;
    }
    return this->__underlying.at(_idx);
}

auto zpt::JSONArrT::operator[](const char* _idx) -> zpt::json& {
    return (*this)[std::string(_idx)];
}

auto zpt::JSONArrT::operator[](std::string _idx) -> zpt::json& {
    long _i = -1;
    zpt::fromstr(_idx, &_i);

    if (_i < 0 || _i >= (long)this->__underlying.size()) {
        return zpt::undefined;
    }

    return this->__underlying.at((size_t)_i);
}

auto
zpt::JSONArrT::stringify(std::string& _out) -> JSONArrT& {
    _out.insert(_out.length(), "[");
    bool _first = true;
    for (auto _i : this->__underlying) {
        if (!_first) {
            _out.insert(_out.length(), ",");
        }
        _first = false;
        _i->stringify(_out);
    }
    _out.insert(_out.length(), "]");
    return (*this);
}

auto
zpt::JSONArrT::stringify(std::ostream& _out) -> JSONArrT& {
    _out << "[" << std::flush;
    bool _first = true;
    for (auto _i : this->__underlying) {
        if (!_first) {
            _out << ",";
        }
        _first = false;
        _i->stringify(_out);
    }
    _out << "]" << std::flush;
    return (*this);
}

auto
zpt::JSONArrT::prettify(std::string& _out, uint _n_tabs) -> JSONArrT& {
    _out.insert(_out.length(), "[");
    bool _first = true;
    for (auto _i : this->__underlying) {
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
    return (*this);
}

auto
zpt::JSONArrT::prettify(std::ostream& _out, uint _n_tabs) -> JSONArrT& {
    _out << "[" << std::flush;
    bool _first = true;
    for (auto _i : this->__underlying) {
        if (!_first) {
            _out << ",";
        }
        _out << "\n ";
        _first = false;
        _out << std::string(_n_tabs + 1, '\t') << std::flush;
        _i->prettify(_out, _n_tabs + 1);
    }
    if (!_first) {
        _out << "\n" << std::string(_n_tabs, '\t');
    }
    _out << "]" << std::flush;
    return (*this);
}

/*JSON POINTER TO ELEMENT*/
zpt::json::json()
  : __underlying{ std::make_shared<JSONElementT>() } {}

zpt::json::json(JSONElementT* _target)
  : __underlying{ _target } {}

zpt::json::json(std::initializer_list<JSONElementT> _init)
  : __underlying{ std::make_shared<JSONElementT>(_init) } {}

zpt::json::json(std::string _rhs) {
    std::istringstream _iss;
    _iss.str(_rhs);
    _iss >> (*this);
}

zpt::json::json(const char* _rhs) {
    std::istringstream _iss;
    _iss.str(std::string(_rhs));
    _iss >> (*this);
}

zpt::json::json(zpt::pretty _rhs) {
    std::istringstream _iss;
    _iss.str(_rhs);
    _iss >> (*this);
}

zpt::json::json(const zpt::json& _rhs) {
    (*this) = _rhs;
}

zpt::json::json(zpt::json&& _rhs) {
    (*this) = _rhs;
}

zpt::json::~json() {}

auto
zpt::json::value() -> zpt::JSONElementT& {
    if (this->__underlying.get() == nullptr) {
        return *(zpt::undefined.__underlying.get());
    }
    return *(this->__underlying.get());
}

auto
zpt::json::operator=(const zpt::json& _rhs) -> zpt::json& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::json::operator=(zpt::json&& _rhs) -> zpt::json& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::json::operator-> () -> std::shared_ptr<JSONElementT>& {
    return this->__underlying;
}

auto zpt::json::operator*() -> std::shared_ptr<JSONElementT>& {
    return this->__underlying;
}

zpt::json::operator std::string() {
    if (this->__underlying.get() == nullptr) {
        return "";
    }
    std::string _out;
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            this->__underlying.get()->obj()->stringify(_out);
            break;
        }
        case zpt::JSArray: {
            this->__underlying.get()->arr()->stringify(_out);
            break;
        }
        case zpt::JSString: {
            _out.assign(this->__underlying.get()->str().data());
            break;
        }
        case zpt::JSInteger: {
            zpt::tostr(_out, this->__underlying.get()->intr());
            break;
        }
        case zpt::JSDouble: {
            zpt::tostr(_out, this->__underlying.get()->dbl());
            break;
        }
        case zpt::JSBoolean: {
            zpt::tostr(_out, this->__underlying.get()->bln());
            break;
        }
        case zpt::JSNil: {
            _out.assign("");
            break;
        }
        case zpt::JSDate: {
            _out.insert(_out.length(), zpt::timestamp(this->__underlying.get()->date()));
            break;
        }
        case zpt::JSLambda: {
            _out.assign(this->__underlying.get()->lbd()->signature());
            break;
        }
    }
    return _out;
}

zpt::json::operator zpt::pretty() {
    if (this->__underlying.get() == nullptr) {
        return zpt::pretty("");
    }
    std::string _out;
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            this->__underlying.get()->obj()->prettify(_out);
            break;
        }
        case zpt::JSArray: {
            this->__underlying.get()->arr()->prettify(_out);
            break;
        }
        case zpt::JSString: {
            _out.assign(this->__underlying.get()->str().data());
            break;
        }
        case zpt::JSInteger: {
            zpt::tostr(_out, this->__underlying.get()->intr());
            break;
        }
        case zpt::JSDouble: {
            zpt::tostr(_out, this->__underlying.get()->dbl());
            break;
        }
        case zpt::JSBoolean: {
            zpt::tostr(_out, this->__underlying.get()->bln());
            break;
        }
        case zpt::JSNil: {
            _out.assign("");
            break;
        }
        case zpt::JSDate: {
            _out.insert(_out.length(), zpt::timestamp(this->__underlying.get()->date()));
            break;
        }
        case zpt::JSLambda: {
            _out.assign(this->__underlying.get()->lbd()->signature());
            break;
        }
    }
    return zpt::pretty(_out);
}

zpt::json::operator bool() {
    if (this->__underlying.get() == nullptr) {
        return false;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return true;
        }
        case zpt::JSArray: {
            return true;
        }
        case zpt::JSString: {
            return this->__underlying.get()->str().length() != 0;
        }
        case zpt::JSInteger: {
            return (bool)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (bool)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return false;
        }
        case zpt::JSDate: {
            return (bool)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return true;
        }
    }
    return false;
}

zpt::json::operator int() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            int _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (int)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (int)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (int)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (int)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

zpt::json::operator long() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            long _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (long)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (long)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (long)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (long)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

zpt::json::operator long long() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            long long _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (long long)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (long long)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (long long)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (long long)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

#ifdef __LP64__
zpt::json::operator unsigned int() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            unsigned int _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (unsigned int)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (unsigned int)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (unsigned int)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (unsigned int)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}
#endif

zpt::json::operator size_t() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            size_t _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (size_t)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (size_t)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (size_t)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (size_t)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

zpt::json::operator double() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (double)(***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (double)(***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            double _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (double)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (double)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (double)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (double)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

zpt::json::operator zpt::timestamp_t() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            struct timeval _tp;
            gettimeofday(&_tp, nullptr);
            return _tp.tv_sec * 1000 + _tp.tv_usec / 1000;
        }
        case zpt::JSArray: {
            struct timeval _tp;
            gettimeofday(&_tp, nullptr);
            return _tp.tv_sec * 1000 + _tp.tv_usec / 1000;
        }
        case zpt::JSString: {
            return this->__underlying.get()->date();
        }
        case zpt::JSInteger: {
            return (zpt::timestamp_t)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (zpt::timestamp_t)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (zpt::timestamp_t)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

zpt::json::operator JSONObj() {
    assertz(this->__underlying.get() != nullptr &&
              this->__underlying.get()->type() == zpt::JSObject,
            std::string("this element is not of type JSObject: ") + ((std::string) * this),
            0,
            0);
    return this->__underlying.get()->obj();
}

zpt::json::operator JSONArr() {
    assertz(this->__underlying.get() != nullptr && this->__underlying.get()->type() == zpt::JSArray,
            std::string("this element is not of type JSArray: ") + ((std::string) * this),
            0,
            0);
    return this->__underlying.get()->arr();
}

zpt::json::operator JSONObj&() {
    assertz(this->__underlying.get() != nullptr &&
              this->__underlying.get()->type() == zpt::JSObject,
            std::string("this element is not of type JSObject: ") + ((std::string) * this),
            0,
            0);
    return this->__underlying.get()->obj();
}

zpt::json::operator JSONArr&() {
    assertz(this->__underlying.get() != nullptr && this->__underlying.get()->type() == zpt::JSArray,
            std::string("this element is not of type JSArray: ") + ((std::string) * this),
            0,
            0);
    return this->__underlying.get()->arr();
}

zpt::json::operator zpt::lambda() {
    assertz(this->__underlying.get() != nullptr &&
              this->__underlying.get()->type() == zpt::JSLambda,
            std::string("this element is not of type JSLambda: ") + ((std::string) * this),
            0,
            0);
    return this->__underlying.get()->lbd();
}

auto
zpt::json::parse(std::istream& _in) -> void {
    zpt::JSONParser _p;
    _p.switchRoots(*this);
    _p.switchStreams(_in);
    _p.parse();
}

/*JSON POINTER TO OBJECT*/
zpt::JSONObj::JSONObj()
  : __underlying{ std::make_shared<JSONObjT>() } {}

zpt::JSONObj::JSONObj(const JSONObj& _rhs) {
    (*this) = _rhs;
}

zpt::JSONObj::JSONObj(JSONObj&& _rhs) {
    (*this) = _rhs;
}

zpt::JSONObj::JSONObj(JSONObjT* _target)
  : __underlying{ _target } {}

zpt::JSONObj::~JSONObj() {}

auto
zpt::JSONObj::operator=(const zpt::JSONObj& _rhs) -> zpt::JSONObj& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::JSONObj::operator=(zpt::JSONObj&& _rhs) -> zpt::JSONObj& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::JSONObj::operator-> () -> std::shared_ptr<zpt::JSONObjT>& {
    return this->__underlying;
}

auto zpt::JSONObj::operator*() -> std::shared_ptr<zpt::JSONObjT>& {
    return this->__underlying;
}

zpt::JSONObj::operator std::string() {
    if (this->__underlying.get() == nullptr) {
        return "";
    }
    std::string _out;
    (*this)->stringify(_out);
    return _out;
}

zpt::JSONObj::operator zpt::pretty() {
    if (this->__underlying.get() == nullptr) {
        return "";
    }
    std::string _out;
    (*this)->prettify(_out);
    return _out;
}

auto
zpt::JSONObj::operator<<(std::string _in) -> zpt::JSONObj& {
    (*this)->push(_in);
    return *this;
}

auto
zpt::JSONObj::operator<<(const char* _in) -> zpt::JSONObj& {
    (*this)->push(_in);
    return *this;
}

auto
zpt::JSONObj::operator<<(JSONElementT& _in) -> zpt::JSONObj& {
    (*this)->push(_in);
    return *this;
}

/*JSON POINTER TO ARRAY*/
zpt::JSONArr::JSONArr()
  : __underlying{ std::make_shared<zpt::JSONArrT>(JSONArrT()) } {}

zpt::JSONArr::JSONArr(const JSONArr& _rhs) {
    (*this) = _rhs;
}

zpt::JSONArr::JSONArr(JSONArr&& _rhs) {
    (*this) = _rhs;
}

zpt::JSONArr::JSONArr(zpt::JSONArrT* _target)
  : __underlying{ _target } {}

zpt::JSONArr::~JSONArr() {}

auto
zpt::JSONArr::operator=(const zpt::JSONArr& _rhs) -> zpt::JSONArr& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::JSONArr::operator=(zpt::JSONArr&& _rhs) -> zpt::JSONArr& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::JSONArr::operator-> () -> std::shared_ptr<zpt::JSONArrT>& {
    return this->__underlying;
}

auto zpt::JSONArr::operator*() -> std::shared_ptr<zpt::JSONArrT>& {
    return this->__underlying;
}

zpt::JSONArr::operator std::string() {
    if (this->__underlying.get() == nullptr) {
        return "";
    }
    std::string _out;
    (*this)->stringify(_out);
    return _out;
}

zpt::JSONArr::operator zpt::pretty() {
    if (this->__underlying.get() == nullptr) {
        return "";
    }
    std::string _out;
    (*this)->prettify(_out);
    return _out;
}

zpt::JSONArr&
zpt::JSONArr::operator<<(JSONElementT& _in) {
    (*this)->push(_in);
    return *this;
}

void
zpt::json::stringify(std::string& _str) {
    zpt::utf8::encode(_str, true);
}

zpt::json
zpt::get(std::string _path, zpt::json _source) {
    return _source->get_path(_path);
}

zpt::timestamp_t
zpt::timestamp(std::string _json_date) {
    if (_json_date.length() == 0) {
        return (zpt::timestamp_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
          .count();
    }
    time_t _n = 0;
    int _ms = 0;
    std::string _s(_json_date.data());
    size_t _idx = _s.rfind(".");
    std::string _mss;
    if (_idx != std::string::npos) {
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
    }
    return _n * 1000 + _ms;
}

zpt::timestamp_t
zpt::timestamp(zpt::json _json_date) {
    return (zpt::timestamp_t)_json_date;
}

std::string
zpt::timestamp(zpt::timestamp_t _timestamp) {
    std::string _date = zpt::tostr((size_t)(_timestamp / 1000), "%Y-%m-%dT%H:%M:%S");
    _date.insert(_date.length(), ".");
    size_t _remainder = _timestamp % 1000;
    if (_remainder < 100) {
        _date.insert(_date.length(), "0");
        if (_remainder < 10) {
            _date.insert(_date.length(), "0");
        }
    }
    zpt::tostr(_date, _remainder);
    size_t _time_offset = (size_t)(_timestamp / 1000);
    zpt::tostr(_date, _time_offset, "%z");
    return _date;
}

auto
zpt::json::begin() -> zpt::json::iterator {
    return zpt::json::iterator{ *this };
}

auto
zpt::json::end() -> zpt::json::iterator {
    size_t _size{ 0 };
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            _size = (***this->__underlying.get()->obj()).size();
            break;
        }
        case zpt::JSArray: {
            _size = (***this->__underlying.get()->arr()).size();
            break;
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda: {
            break;
        }
    }
    return zpt::json::iterator{ *this, _size };
}

auto
zpt::json::object() -> zpt::json {
    zpt::JSONObj _empty;
    return zpt::json(new zpt::JSONElementT(_empty));
}

auto
zpt::json::array() -> zpt::json {
    zpt::JSONArr _empty;
    return zpt::json(new zpt::JSONElementT(_empty));
}

auto
zpt::json::date(std::string _e) -> zpt::json {
    zpt::timestamp_t _v(zpt::timestamp(_e));
    return zpt::json(new zpt::JSONElementT(_v));
}

auto
zpt::json::date() -> zpt::json {
    zpt::timestamp_t _v((zpt::timestamp_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count());
    return zpt::json(new zpt::JSONElementT(_v));
}

auto
zpt::json::lambda(std::string _name, unsigned short _n_args) -> zpt::json {
    zpt::lambda _v(_name, _n_args);
    return zpt::json(new zpt::JSONElementT(_v));
}

// zpt::json::element::element(size_t _index, zpt::json& _value)
//   : __index{ _index }
//   , __value{ _value } {}

// zpt::json::element::element(size_t _index, std::string _key, zpt::json& _value)
//   : __index{ _index }
//   , __key{ _key }
//   , __value{ _value } {}

// zpt::json::element::element(const element& _rhs)
//   : __index{ _rhs.__index }
//   , __key{ _rhs.__key }
//   , __value{ _rhs.__value } {}

// zpt::json::element::element(element&& _rhs)
//   : __index{ _rhs.__index }
//   , __key{ std::move(_rhs.__key) }
//   , __value{ _rhs.__value } {}

// auto
// zpt::json::element::index() -> size_t {
//     return this->__index;
// }

// auto
// zpt::json::element::key() -> std::string& {
//     return this->__key;
// }

// auto
// zpt::json::element::value() -> zpt::json {
//     return this->__value;
// }

// auto
// zpt::json::element::operator=(const element& _rhs) -> element& {
//     this->__index = _rhs.__index;
//     this->__key = _rhs.__key;
//     this->__value = _rhs.__value;
// }

// auto
// zpt::json::element::operator=(element&& _rhs) -> element& {
//     this->__index = _rhs.__index;
//     this->__key = std::move(_rhs.__key);
//     this->__value = _rhs.__value;
// }

zpt::json::iterator::iterator(zpt::json& _target, size_t _pos)
  : __target{ _target }
  , __pos{ _pos } {}

zpt::json::iterator::iterator(const iterator& _rhs)
  : __target{ _rhs.__target }
  , __pos{ _rhs.__pos } {}

auto
zpt::json::iterator::operator=(const iterator& _rhs) -> iterator& {
    this->__target = _rhs.__target;
    this->__pos = _rhs.__pos;
    return (*this);
}

auto
zpt::json::iterator::operator++() -> iterator& {
    switch (this->__target->type()) {
        case zpt::JSObject: {
            if (this->__pos != (***this->__target->obj()).size()) {
                ++this->__pos;
            }
            break;
        }
        case zpt::JSArray: {
            if (this->__pos != (***this->__target->arr()).size()) {
                ++this->__pos;
            }
            break;
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda: {
            break;
        }
    }
    return (*this);
}

auto zpt::json::iterator::operator*() const -> reference {
    switch (this->__target->type()) {
        case zpt::JSObject: {
            return this->__target->element(this->__pos);
        }
        case zpt::JSArray: {
            return this->__target->element(this->__pos);
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda: {
            break;
        }
    }
    return std::make_tuple(0, "", this->__target);
}

auto
zpt::json::iterator::operator++(int) -> iterator {
    zpt::json::iterator _to_return = (*this);
    ++(*this);
    return _to_return;
}

auto zpt::json::iterator::operator-> () const -> pointer {
    switch (this->__target->type()) {
        case zpt::JSObject: {
            return this->__target->element(this->__pos);
        }
        case zpt::JSArray: {
            return this->__target->element(this->__pos);
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda: {
            break;
        }
    }
    return std::make_tuple(0, "", this->__target);
}

auto
zpt::json::iterator::operator==(iterator _rhs) const -> bool {
    return this->__target == _rhs.__target && this->__pos == _rhs.__pos;
}

auto
zpt::json::iterator::operator!=(iterator _rhs) const -> bool {
    return !((*this) == _rhs);
}

auto
zpt::json::iterator::operator--() -> iterator& {
    switch (this->__target->type()) {
        case zpt::JSObject:
        case zpt::JSArray: {
            if (this->__pos != 0) {
                --this->__pos;
            }
            break;
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda: {
            break;
        }
    }
    return (*this);
}

auto
zpt::json::iterator::operator--(int) -> iterator {
    zpt::json::iterator _to_return = (*this);
    --(*this);
    return _to_return;
}
