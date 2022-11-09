/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdarg>
#include <ostream>
#include <regex>
#include <zapata/json/JSONClass.h>

zpt::JSONElementT::JSONElementT() {}

zpt::JSONElementT::JSONElementT(const JSONElementT& _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(JSONElementT&& _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(std::string const& _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(const char* _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(long long _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(double _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(bool _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(int _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(size_t _rhs) { (*this) = _rhs; }

#ifdef __LP64__
zpt::JSONElementT::JSONElementT(unsigned int _rhs) { (*this) = _rhs; }
#endif

zpt::JSONElementT::JSONElementT(zpt::timestamp_t _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(zpt::JSONObj& _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(zpt::JSONArr& _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(zpt::lambda _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(zpt::regex _rhs) { (*this) = _rhs; }

zpt::JSONElementT::~JSONElementT() {}

auto zpt::JSONElementT::type() const -> zpt::JSONType { return (zpt::JSONType)this->__target.__type; }

auto zpt::JSONElementT::demangle() const -> std::string {
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
        case zpt::JSRegex: {
            return "regex";
        }
    }
    return "null";
}

auto zpt::JSONElementT::type(zpt::JSONType _in) -> JSONElementT& {
    expect(_in >= 0, "the type must be a valid value");

    if (_in == this->__target.__type) { return (*this); }

    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__object.operator->() != nullptr) { this->__target.__object.~JSONObj(); }
            break;
        }
        case zpt::JSArray: {
            if (this->__target.__array.operator->() != nullptr) { this->__target.__array.~JSONArr(); }
            break;
        }
        case zpt::JSString: {
            if (this->__target.__string.get() != nullptr) { this->__target.__string.~JSONStr(); }
            break;
        }
        case zpt::JSLambda: {
            if (this->__target.__lambda.get() != nullptr) { this->__target.__lambda.~lambda(); }
            break;
        }
        case zpt::JSRegex: {
            this->__target.__regex.~JSONRegex();
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
            new (&this->__target.__lambda) zpt::lambda();
            break;
        }
        case zpt::JSRegex: {
            new (&this->__target.__regex) JSONRegex();
            break;
        }
        default: {
            break;
        }
    }

    this->__target.__type = _in;
    return (*this);
}

auto zpt::JSONElementT::value() -> zpt::JSONUnion& { return this->__target; }

auto zpt::JSONElementT::ok() const -> bool { return this->__target.__type != zpt::JSNil; }

auto zpt::JSONElementT::empty() const -> bool {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__object.operator->() != nullptr) {
                return (*this->__target.__object)->size() == 0;
            }
            return true;
        }
        case zpt::JSArray: {
            if (this->__target.__array.operator->() != nullptr) {
                return (**this->__target.__array).size() == 0;
            }
            return true;
        }
        case zpt::JSString: {
            if (this->__target.__string.get() != nullptr) { return this->__target.__string->length() == 0; }
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
        case zpt::JSRegex: {
            return false;
        }
    }
    return true;
}

auto zpt::JSONElementT::nil() const -> bool { return this->__target.__type == zpt::JSNil; }

auto zpt::JSONElementT::clear() -> void {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__object.operator->() != nullptr) { (**this->__target.__object).clear(); }
            break;
        }
        case zpt::JSArray: {
            if (this->__target.__array.operator->() != nullptr) { (**this->__target.__array).clear(); }
            break;
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda:
        case zpt::JSRegex: {
            break;
        }
    }
}

auto zpt::JSONElementT::size() const -> size_t {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__object.operator->() != nullptr) {
                return (**this->__target.__object).size();
            }
        }
        case zpt::JSArray: {
            if (this->__target.__array.operator->() != nullptr) { return (**this->__target.__array).size(); }
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda:
        case zpt::JSRegex: {
            break;
        }
    }
    return 0;
}

auto zpt::JSONElementT::hash() const -> size_t {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__object.operator->() != nullptr) {
                size_t _to_return = this->__target.__object.hash();
                return _to_return;
            }
        }
        case zpt::JSArray: {
            if (this->__target.__array.operator->() != nullptr) {
                size_t _to_return = this->__target.__array.hash();
                return _to_return;
            }
        }
        case zpt::JSString: {
            size_t _to_return = std::hash<std::string>{}(*this->__target.__string);
            return _to_return;
        }
        case zpt::JSInteger: {
            size_t _to_return = std::hash<long long>{}(this->__target.__integer);
            return _to_return;
        }
        case zpt::JSDouble: {
            size_t _to_return = std::hash<double>{}(this->__target.__double);
            return _to_return;
        }
        case zpt::JSBoolean: {
            size_t _to_return = std::hash<bool>{}(this->__target.__boolean);
            return _to_return;
        }
        case zpt::JSNil: {
            size_t _to_return = reinterpret_cast<size_t>(&zpt::undefined);
            return _to_return;
        }
        case zpt::JSDate: {
            size_t _to_return = std::hash<unsigned long long>{}(this->__target.__date);
            return _to_return;
        }
        case zpt::JSLambda: {
            size_t _to_return = std::hash<std::string>{}(this->__target.__lambda->signature());
            return _to_return;
        }
        case zpt::JSRegex: {
            size_t _to_return = std::hash<std::string>{}(this->__target.__regex.to_string());
            return _to_return;
        }
    }
    return 0;
}

auto zpt::JSONElementT::parent() -> JSONElementT* { return this->__parent; }

auto zpt::JSONElementT::parent(JSONElementT* _parent) -> JSONElementT& {
    this->__parent = _parent;
    return (*this);
}

auto zpt::JSONElementT::is_object() -> bool { return this->__target.__type == zpt::JSObject; }

auto zpt::JSONElementT::is_array() -> bool { return this->__target.__type == zpt::JSArray; }

auto zpt::JSONElementT::is_string() -> bool { return this->__target.__type == zpt::JSString; }

auto zpt::JSONElementT::is_integer() -> bool { return this->__target.__type == zpt::JSInteger; }

auto zpt::JSONElementT::is_floating() -> bool { return this->__target.__type == zpt::JSDouble; }

auto zpt::JSONElementT::is_number() -> bool {
    return this->__target.__type == zpt::JSInteger || this->__target.__type == zpt::JSDouble;
}

auto zpt::JSONElementT::is_bool() -> bool { return this->__target.__type == zpt::JSBoolean; }

auto zpt::JSONElementT::is_date() -> bool {
    return this->__target.__type == zpt::JSDate || this->__target.__type == zpt::JSString;
}

auto zpt::JSONElementT::is_lambda() -> bool { return this->__target.__type == zpt::JSLambda; }

auto zpt::JSONElementT::is_regex() -> bool { return this->__target.__type == zpt::JSRegex; }

auto zpt::JSONElementT::is_nil() -> bool { return this->__target.__type == zpt::JSNil; }

auto zpt::JSONElementT::is_iterable() -> bool { return this->__target.__type == zpt::JSNil; }

auto zpt::JSONElementT::object() -> zpt::JSONObj& {
    return const_cast<zpt::JSONObj&>(static_cast<const zpt::JSONElementT&>(*this).object());
}

auto zpt::JSONElementT::array() -> zpt::JSONArr& {
    return const_cast<zpt::JSONArr&>(static_cast<const zpt::JSONElementT&>(*this).array());
}

auto zpt::JSONElementT::string() -> std::string& {
    return const_cast<std::string&>(static_cast<const zpt::JSONElementT&>(*this).string());
}

auto zpt::JSONElementT::integer() -> long long& {
    return const_cast<long long&>(static_cast<const zpt::JSONElementT&>(*this).integer());
}

auto zpt::JSONElementT::floating() -> double& {
    return const_cast<double&>(static_cast<const zpt::JSONElementT&>(*this).floating());
}

auto zpt::JSONElementT::boolean() -> bool& {
    return const_cast<bool&>(static_cast<const zpt::JSONElementT&>(*this).boolean());
}

auto zpt::JSONElementT::date() -> zpt::timestamp_t& {
    return const_cast<zpt::timestamp_t&>(static_cast<const zpt::JSONElementT&>(*this).date());
}

auto zpt::JSONElementT::lambda() -> zpt::lambda& {
    return const_cast<zpt::lambda&>(static_cast<const zpt::JSONElementT&>(*this).lambda());
}

auto zpt::JSONElementT::regex() -> zpt::regex& {
    return const_cast<zpt::regex&>(static_cast<const zpt::JSONElementT&>(*this).regex());
}

auto zpt::JSONElementT::number() -> double { return static_cast<const zpt::JSONElementT&>(*this).number(); }

auto zpt::JSONElementT::object() const -> zpt::JSONObj const& {
    expect(this->__target.__type == zpt::JSObject,
           std::string("this element is not of type JSObject: ") + this->stringify());
    return this->__target.__object;
}

auto zpt::JSONElementT::array() const -> zpt::JSONArr const& {
    expect(this->__target.__type == zpt::JSArray,
           std::string("this element is not of type JSArray: ") + this->stringify());
    return this->__target.__array;
}

auto zpt::JSONElementT::string() const -> std::string const& {
    expect(this->__target.__type == zpt::JSString,
           std::string("this element is not of type JSString: ") + this->stringify());
    return *(this->__target.__string.get());
}

auto zpt::JSONElementT::integer() const -> long long const& {
    expect(this->__target.__type == zpt::JSInteger,
           std::string("this element is not of type JSInteger: ") + this->stringify());
    return this->__target.__integer;
}

auto zpt::JSONElementT::floating() const -> double const& {
    expect(this->__target.__type == zpt::JSDouble,
           std::string("this element is not of type JSDouble: ") + this->stringify());
    return this->__target.__double;
}

auto zpt::JSONElementT::boolean() const -> bool const& {
    expect(this->__target.__type == zpt::JSBoolean,
           std::string("this element is not of type JSBoolean: ") + this->stringify());
    return this->__target.__boolean;
}

auto zpt::JSONElementT::date() const -> zpt::timestamp_t const& {
    expect(this->__target.__type == zpt::JSDate,
           std::string("this element is not of type JSDate: ") + this->stringify());
    return this->__target.__date;
}

auto zpt::JSONElementT::lambda() const -> zpt::lambda const& {
    expect(this->__target.__type == zpt::JSLambda,
           std::string("this element is not of type JSLambda: ") + this->stringify());
    return this->__target.__lambda;
}

auto zpt::JSONElementT::regex() const -> zpt::regex const& {
    expect(this->__target.__type == zpt::JSRegex,
           std::string("this element is not of type JSRegex: ") + this->stringify());
    return this->__target.__regex;
}

auto zpt::JSONElementT::number() const -> double const {
    expect(this->__target.__type == zpt::JSDate || this->__target.__type == zpt::JSInteger ||
             this->__target.__type == zpt::JSDouble || this->__target.__type == zpt::JSBoolean,
           std::string("this element is not of type JSInteger, JSDouble or JSBoolean: ") + this->stringify());
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

auto zpt::JSONElementT::clone() const -> zpt::json {
    switch (this->type()) {
        case zpt::JSObject: {
            return this->object()->clone();
        }
        case zpt::JSArray: {
            return this->array()->clone();
        }
        case zpt::JSString: {
            std::string _v = this->string();
            return zpt::json{ _v };
        }
        case zpt::JSInteger: {
            int _v = this->integer();
            return zpt::json{ _v };
        }
        case zpt::JSDouble: {
            double _v = this->floating();
            return zpt::json{ _v };
        }
        case zpt::JSBoolean: {
            bool _v = this->boolean();
            return zpt::json{ _v };
        }
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            zpt::timestamp_t _v = this->date();
            return zpt::json{ _v };
        }
        case zpt::JSLambda: {
            return zpt::json::lambda(this->lambda()->name(), this->lambda()->n_args());
        }
        case zpt::JSRegex: {
            zpt::regex _v = this->regex();
            return zpt::json{ _v };
        }
    }
    return zpt::undefined;
}

auto zpt::JSONElementT::operator=(const JSONElementT& _rhs) -> JSONElementT& {
    this->type(_rhs.__target.__type);
    this->__parent = _rhs.__parent;

    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object = _rhs.__target.__object;
            break;
        }
        case zpt::JSArray: {
            this->__target.__array = _rhs.__target.__array;
            break;
        }
        case zpt::JSString: {
            this->__target.__string = _rhs.__target.__string;
            break;
        }
        case zpt::JSInteger: {
            this->__target.__integer = _rhs.__target.__integer;
            break;
        }
        case zpt::JSDouble: {
            this->__target.__double = _rhs.__target.__double;
            break;
        }
        case zpt::JSBoolean: {
            this->__target.__boolean = _rhs.__target.__boolean;
            break;
        }
        case zpt::JSNil: {
            this->__target.__nil = nullptr;
            break;
        }
        case zpt::JSDate: {
            this->__target.__date = _rhs.__target.__date;
            break;
        }
        case zpt::JSLambda: {
            this->__target.__lambda = _rhs.__target.__lambda;
            break;
        }
        case zpt::JSRegex: {
            this->__target.__regex = _rhs.__target.__regex;
            break;
        }
    }
    return (*this);
}

auto zpt::JSONElementT::operator=(JSONElementT&& _rhs) -> JSONElementT& {
    this->type(_rhs.__target.__type);
    this->__parent = _rhs.__parent;
    _rhs.__parent = nullptr;

    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object = std::move(_rhs.__target.__object);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array = std::move(_rhs.__target.__array);
            break;
        }
        case zpt::JSString: {
            this->__target.__string = std::move(_rhs.__target.__string);
            break;
        }
        case zpt::JSInteger: {
            this->__target.__integer = std::move(_rhs.__target.__integer);
            break;
        }
        case zpt::JSDouble: {
            this->__target.__double = std::move(_rhs.__target.__double);
            break;
        }
        case zpt::JSBoolean: {
            this->__target.__boolean = std::move(_rhs.__target.__boolean);
            break;
        }
        case zpt::JSNil: {
            this->__target.__nil = nullptr;
            break;
        }
        case zpt::JSDate: {
            this->__target.__date = std::move(_rhs.__target.__date);
            break;
        }
        case zpt::JSLambda: {
            this->__target.__lambda = std::move(_rhs.__target.__lambda);
            break;
        }
        case zpt::JSRegex: {
            this->__target.__regex = std::move(_rhs.__target.__regex);
            break;
        }
    }
    return (*this);
}

auto zpt::JSONElementT::operator=(std::string const& _rhs) -> JSONElementT& {
    this->type(zpt::JSString);
    this->__target.__string = std::make_shared<std::string>(_rhs);
    return (*this);
}

auto zpt::JSONElementT::operator=(std::nullptr_t) -> JSONElementT& {
    this->type(zpt::JSNil);
    return (*this);
}

auto zpt::JSONElementT::operator=(const char* _rhs) -> JSONElementT& {
    this->type(zpt::JSString);
    this->__target.__string = std::make_shared<std::string>(_rhs);
    return (*this);
}

auto zpt::JSONElementT::operator=(long long _rhs) -> JSONElementT& {
    this->type(zpt::JSInteger);
    this->__target.__integer = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(double _rhs) -> JSONElementT& {
    this->type(zpt::JSDouble);
    this->__target.__double = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(bool _rhs) -> JSONElementT& {
    this->type(zpt::JSBoolean);
    this->__target.__boolean = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(int _rhs) -> JSONElementT& {
    this->type(zpt::JSInteger);
    this->__target.__integer = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(size_t _rhs) -> JSONElementT& {
    this->type(zpt::JSInteger);
    this->__target.__integer = _rhs;
    return (*this);
}

#ifdef __LP64__
auto zpt::JSONElementT::operator=(unsigned int _rhs) -> JSONElementT& {
    this->type(zpt::JSInteger);
    this->__target.__integer = _rhs;
    return (*this);
}
#endif

auto zpt::JSONElementT::operator=(zpt::json _rhs) -> JSONElementT& {
    (*this) = (*_rhs);
    return (*this);
}

auto zpt::JSONElementT::operator=(zpt::timestamp_t _rhs) -> JSONElementT& {
    this->type(zpt::JSDate);
    this->__target.__date = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(zpt::JSONObj& _rhs) -> JSONElementT& {
    this->type(zpt::JSObject);
    this->__target.__object = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(zpt::JSONArr& _rhs) -> JSONElementT& {
    this->type(zpt::JSArray);
    this->__target.__array = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(zpt::lambda _rhs) -> JSONElementT& {
    this->type(zpt::JSLambda);
    this->__target.__lambda = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(zpt::regex _rhs) -> JSONElementT& {
    this->type(zpt::JSRegex);
    this->__target.__regex = _rhs;
    return (*this);
}

zpt::JSONElementT::operator std::string() {
    std::string _out;
    switch (this->type()) {
        case zpt::JSObject: {
            this->object()->stringify(_out);
            break;
        }
        case zpt::JSArray: {
            this->array()->stringify(_out);
            break;
        }
        case zpt::JSString: {
            _out.assign(this->string().data());
            break;
        }
        case zpt::JSInteger: {
            zpt::tostr(_out, this->integer());
            break;
        }
        case zpt::JSDouble: {
            zpt::tostr(_out, this->floating());
            break;
        }
        case zpt::JSBoolean: {
            zpt::tostr(_out, this->boolean());
            break;
        }
        case zpt::JSNil: {
            _out.assign("");
            break;
        }
        case zpt::JSDate: {
            _out.insert(_out.length(), zpt::timestamp(this->date()));
            break;
        }
        case zpt::JSLambda: {
            _out.assign(this->lambda()->signature());
            break;
        }
        case zpt::JSRegex: {
            _out.assign(std::string("/") + this->regex().to_string() + std::string("/"));
            break;
        }
    }
    return _out;
}

zpt::JSONElementT::operator bool() {
    switch (this->type()) {
        case zpt::JSObject: {
            return true;
        }
        case zpt::JSArray: {
            return true;
        }
        case zpt::JSString: {
            return this->string().length() != 0;
        }
        case zpt::JSInteger: {
            return (bool)this->integer();
        }
        case zpt::JSDouble: {
            return (bool)this->floating();
        }
        case zpt::JSBoolean: {
            return this->boolean();
        }
        case zpt::JSNil: {
            return false;
        }
        case zpt::JSDate: {
            return (bool)this->date();
        }
        case zpt::JSLambda: {
            return true;
        }
        case zpt::JSRegex: {
            return true;
        }
    }
    return false;
}

zpt::JSONElementT::operator int() {
    switch (this->type()) {
        case zpt::JSObject: {
            return (**this->object()).size();
        }
        case zpt::JSArray: {
            return (**this->array()).size();
        }
        case zpt::JSString: {
            int _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (int)this->integer();
        }
        case zpt::JSDouble: {
            return (int)this->floating();
        }
        case zpt::JSBoolean: {
            return (int)this->boolean();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (int)this->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
        case zpt::JSRegex: {
            return 0;
        }
    }
    return 0;
}

zpt::JSONElementT::operator long() {
    switch (this->type()) {
        case zpt::JSObject: {
            return (**this->object()).size();
        }
        case zpt::JSArray: {
            return (**this->array()).size();
        }
        case zpt::JSString: {
            long _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (long)this->integer();
        }
        case zpt::JSDouble: {
            return (long)this->floating();
        }
        case zpt::JSBoolean: {
            return (long)this->boolean();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (long)this->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
        case zpt::JSRegex: {
            return 0;
        }
    }
    return 0;
}

zpt::JSONElementT::operator long long() {
    switch (this->type()) {
        case zpt::JSObject: {
            return (**this->object()).size();
        }
        case zpt::JSArray: {
            return (**this->array()).size();
        }
        case zpt::JSString: {
            long long _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (long long)this->integer();
        }
        case zpt::JSDouble: {
            return (long long)this->floating();
        }
        case zpt::JSBoolean: {
            return (long long)this->boolean();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (long long)this->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
        case zpt::JSRegex: {
            return 0;
        }
    }
    return 0;
}

#ifdef __LP64__
zpt::JSONElementT::operator unsigned int() {
    switch (this->type()) {
        case zpt::JSObject: {
            return (**this->object()).size();
        }
        case zpt::JSArray: {
            return (**this->array()).size();
        }
        case zpt::JSString: {
            unsigned int _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (unsigned int)this->integer();
        }
        case zpt::JSDouble: {
            return (unsigned int)this->floating();
        }
        case zpt::JSBoolean: {
            return (unsigned int)this->boolean();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (unsigned int)this->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
        case zpt::JSRegex: {
            return 0;
        }
    }
    return 0;
}
#endif

zpt::JSONElementT::operator size_t() {
    switch (this->type()) {
        case zpt::JSObject: {
            return (**this->object()).size();
        }
        case zpt::JSArray: {
            return (**this->array()).size();
        }
        case zpt::JSString: {
            size_t _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (size_t)this->integer();
        }
        case zpt::JSDouble: {
            return (size_t)this->floating();
        }
        case zpt::JSBoolean: {
            return (size_t)this->boolean();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (size_t)this->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
        case zpt::JSRegex: {
            return 0;
        }
    }
    return 0;
}

zpt::JSONElementT::operator double() {
    switch (this->type()) {
        case zpt::JSObject: {
            return (double)(**this->object()).size();
        }
        case zpt::JSArray: {
            return (double)(**this->array()).size();
        }
        case zpt::JSString: {
            double _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (double)this->integer();
        }
        case zpt::JSDouble: {
            return (double)this->floating();
        }
        case zpt::JSBoolean: {
            return (double)this->boolean();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (double)this->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
        case zpt::JSRegex: {
            return 0;
        }
    }
    return 0;
}

zpt::JSONElementT::operator zpt::timestamp_t() {
    switch (this->type()) {
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
            return this->date();
        }
        case zpt::JSInteger: {
            return (zpt::timestamp_t)this->integer();
        }
        case zpt::JSDouble: {
            return (zpt::timestamp_t)this->floating();
        }
        case zpt::JSBoolean: {
            return (zpt::timestamp_t)this->boolean();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return this->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
        case zpt::JSRegex: {
            return 0;
        }
    }
    return 0;
}

zpt::JSONElementT::operator JSONObj() {
    expect(this->type() == zpt::JSObject,
           std::string("this element is not of type JSObject: ") + static_cast<std::string>(*this));
    return this->object();
}

zpt::JSONElementT::operator JSONArr() {
    expect(this->type() == zpt::JSArray,
           std::string("this element is not of type JSArray: ") + static_cast<std::string>(*this));
    return this->array();
}

zpt::JSONElementT::operator JSONObj&() {
    expect(this->type() == zpt::JSObject,
           std::string("this element is not of type JSObject: ") + static_cast<std::string>(*this));
    return this->object();
}

zpt::JSONElementT::operator JSONArr&() {
    expect(this->type() == zpt::JSArray,
           std::string("this element is not of type JSArray: ") + static_cast<std::string>(*this));
    return this->array();
}

zpt::JSONElementT::operator zpt::lambda() {
    expect(this->type() == zpt::JSLambda,
           std::string("this element is not of type JSLambda: ") + static_cast<std::string>(*this));
    return this->lambda();
}

zpt::JSONElementT::operator zpt::regex() {
    expect(this->type() == zpt::JSRegex,
           std::string("this element is not of type JSRegex: ") + static_cast<std::string>(*this));
    return this->regex();
}

zpt::JSONElementT::operator zpt::regex&() {
    expect(this->type() == zpt::JSRegex,
           std::string("this element is not of type JSRegex: ") + static_cast<std::string>(*this));
    return this->regex();
}

auto zpt::JSONElementT::operator<<(const char* _in) -> zpt::JSONElementT& {
    (*this) << std::string(_in);
    return (*this);
}

auto zpt::JSONElementT::operator<<(std::string const& _in) -> zpt::JSONElementT& {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->push(_in);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->push(_in);
            break;
        }
        case zpt::JSString: {
            this->__target.__string->assign(_in);
            break;
        }
        case zpt::JSInteger: {
            int _converted{ 0 };
            zpt::fromstr(_in, &_converted);
            this->__target.__integer = _converted;
            break;
        }
        case zpt::JSDouble: {
            double _converted{ 0 };
            zpt::fromstr(_in, &_converted);
            this->__target.__double = _converted;
            break;
        }
        case zpt::JSBoolean: {
            bool _converted{ false };
            zpt::fromstr(_in, &_converted);
            this->__target.__boolean = _converted;
            break;
        }
        case zpt::JSNil: {
            break;
        }
        case zpt::JSDate: {
            this->__target.__date = zpt::timestamp(_in);
            break;
        }
        case zpt::JSLambda: {
            break;
        }
        case zpt::JSRegex: {
            this->__target.__regex = zpt::regex{ _in };
            break;
        }
    }
    return (*this);
}

auto zpt::JSONElementT::operator<<(zpt::json _in) -> zpt::JSONElementT& {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->push(_in);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->push(_in);
            break;
        }
        case zpt::JSString: {
            this->__target.__string->assign(*_in->__target.__string.get());
            break;
        }
        case zpt::JSInteger: {
            this->__target.__integer = _in->__target.__integer;
            break;
        }
        case zpt::JSDouble: {
            this->__target.__double = _in->__target.__double;
            break;
        }
        case zpt::JSBoolean: {
            this->__target.__boolean = _in->__target.__boolean;
            break;
        }
        case zpt::JSNil: {
            break;
        }
        case zpt::JSDate: {
            this->__target.__date = _in->__target.__date;
            break;
        }
        case zpt::JSLambda: {
            break;
        }
        case zpt::JSRegex: {
            this->__target.__regex = _in->__target.__regex;
            break;
        }
    }
    return *this;
}

auto zpt::JSONElementT::operator==(zpt::JSONElementT const& _in) const -> bool {
    expect(this->__target.__type >= 0, "the type must be a valid value");
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) { return false; }
            return *(this->__target.__object) == *(_in.object());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) { return false; }
            return *(this->__target.__array) == *(_in.array());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) { return false; }
            return *(this->__target.__string.get()) == _in.string();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__integer == _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__double == _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__boolean == _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) { return false; }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__date == _in.number();
        }
        case zpt::JSLambda: {
            if (this->__target.__type != _in.type()) { return false; }
            return this->__target.__lambda->signature() == _in.lambda()->signature();
        }
        case zpt::JSRegex: {
            if (this->__target.__type != _in.type() && _in.type() != zpt::JSString) { return false; }
            if (_in.type() == zpt::JSRegex) { return this->__target.__regex == _in.regex(); }
            if (_in.type() == zpt::JSString) { return this->__target.__regex == _in.string(); }
        }
    }
    return false;
}

auto zpt::JSONElementT::operator==(zpt::json _rhs) const -> bool { return *this == *_rhs; }

auto zpt::JSONElementT::operator!=(JSONElementT const& _in) const -> bool {
    expect(this->__target.__type >= 0, "the type must be a valid value");
    return !(*this == _in);
}

auto zpt::JSONElementT::operator!=(zpt::json _rhs) const -> bool { return *this != (*_rhs); }

auto zpt::JSONElementT::operator<(zpt::JSONElementT const& _in) const -> bool {
    expect(this->__target.__type >= 0, "the type must be a valid value");
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) { return this->__target.__type < _in.type(); }
            return *(this->__target.__object) < *(_in.object());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) { return this->__target.__type < _in.type(); }
            return *(this->__target.__array) < *(_in.array());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) { return this->__target.__type < _in.type(); }
            return *(this->__target.__string.get()) < _in.string();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->__target.__type < _in.type();
            }
            return this->__target.__integer < _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->__target.__type < _in.type();
            }
            return this->__target.__double < _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->__target.__type < _in.type();
            }
            return this->__target.__boolean < _in.number();
        }
        case zpt::JSNil: {
            return this->__target.__type < _in.type();
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->__target.__type < _in.type();
            }
            return this->__target.__date < _in.number();
        }
        case zpt::JSLambda: {
            if (this->__target.__type != _in.type()) { return this->__target.__type < _in.type(); }
            return this->__target.__lambda->n_args() < _in.lambda()->n_args();
        }
        case zpt::JSRegex: {
            return this->__target.__type < _in.type();
        }
    }
    return false;
}

auto zpt::JSONElementT::operator<(zpt::json _rhs) const -> bool { return *this < *_rhs; }

auto zpt::JSONElementT::operator>(zpt::JSONElementT const& _in) const -> bool {
    expect(this->__target.__type >= 0, "the type must be a valid value");
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) { return this->hash() > _in.hash(); }
            return *(this->__target.__object) > *(_in.object());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) { return this->hash() > _in.hash(); }
            return *(this->__target.__array) > *(_in.array());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) { return this->hash() > _in.hash(); }
            return *(this->__target.__string.get()) > _in.string();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() > _in.hash();
            }
            return this->__target.__integer > _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() > _in.hash();
            }
            return this->__target.__double > _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() > _in.hash();
            }
            return this->__target.__boolean > _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) { return this->hash() > _in.hash(); }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() > _in.hash();
            }
            return this->__target.__date > _in.number();
        }
        case zpt::JSLambda: {
            return this->__target.__lambda->n_args() > _in.lambda()->n_args();
        }
        case zpt::JSRegex: {
            return this->hash() > _in.hash();
        }
    }
    return false;
}

auto zpt::JSONElementT::operator>(zpt::json _rhs) const -> bool { return (*this) > _rhs; }

auto zpt::JSONElementT::operator<=(zpt::JSONElementT const& _in) const -> bool {
    return (*this) == _in || (*this) < _in;
}

auto zpt::JSONElementT::operator<=(zpt::json _rhs) const -> bool { return (*this) <= _rhs; }

auto zpt::JSONElementT::operator>=(zpt::JSONElementT const& _in) const -> bool {
    return (*this) == _in || (*this) > _in;
}

auto zpt::JSONElementT::operator>=(zpt::json _rhs) const -> bool { return (*this) >= _rhs; }

auto zpt::JSONElementT::get_path(std::string const& _path, std::string const& _separator) -> zpt::json {
    expect(this->__target.__type >= 0, "the type must be a valid value");
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
        case zpt::JSDate:
        case zpt::JSRegex: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto zpt::JSONElementT::set_path(std::string const& _path, zpt::json _value, std::string const& _separator)
  -> JSONElementT& {
    expect(this->__target.__type >= 0, "the type must be a valid value");
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->set_path(_path, _value, _separator);
            return (*this);
        }
        case zpt::JSArray: {
            this->__target.__array->set_path(_path, _value, _separator);
            return (*this);
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSDate:
        case zpt::JSRegex: {
            break;
        }
    }
    return (*this);
}

auto zpt::JSONElementT::del_path(std::string const& _path, std::string const& _separator) -> JSONElementT& {
    expect(this->__target.__type >= 0, "the type must be a valid value");
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->del_path(_path, _separator);
            return (*this);
        }
        case zpt::JSArray: {
            this->__target.__array->del_path(_path, _separator);
            return (*this);
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSDate:
        case zpt::JSRegex: {
            break;
        }
    }
    return (*this);
}

auto zpt::JSONElementT::stringify(std::string& _out) -> zpt::JSONElementT& {
    static_cast<zpt::JSONElementT const&>(*this).stringify(_out);
    return (*this);
}

auto zpt::JSONElementT::stringify(std::ostream& _out) -> zpt::JSONElementT& {
    static_cast<zpt::JSONElementT const&>(*this).stringify(_out);
    return (*this);
}

auto zpt::JSONElementT::stringify(std::ostream& _out) const -> zpt::JSONElementT const& {
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
            std::string _str{ this->string() };
            zpt::json::to_unicode(_str);
            _out << "\"" << _str << "\"" << std::flush;
            break;
        }
        case zpt::JSInteger: {
            _out << this->__target.__integer << std::flush;
            break;
        }
        case zpt::JSDouble: {
            _out << std::fixed << std::setprecision(3) << this->__target.__double << std::flush;
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
        case zpt::JSRegex: {
            _out << "/" << this->__target.__regex.to_string() << "/" << std::flush;
            break;
        }
    }
    return (*this);
}

auto zpt::JSONElementT::stringify(std::string& _out) const -> JSONElementT const& {
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
            std::string _str{ this->string() };
            zpt::json::to_unicode(_str);
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
        case zpt::JSRegex: {
            _out.insert(_out.length(), "/");
            _out.insert(_out.length(), this->__target.__regex.to_string());
            _out.insert(_out.length(), "/");
            break;
        }
    }
    return (*this);
}

auto zpt::JSONElementT::stringify() const -> std::string {
    std::string _out;
    this->stringify(_out);
    return _out;
}

auto zpt::JSONElementT::prettify(std::string& _out, uint _n_tabs) -> zpt::JSONElementT& {
    static_cast<zpt::JSONElementT const&>(*this).prettify(_out, _n_tabs);
    return (*this);
}

auto zpt::JSONElementT::prettify(std::ostream& _out, uint _n_tabs) -> zpt::JSONElementT& {
    static_cast<zpt::JSONElementT const&>(*this).prettify(_out, _n_tabs);
    return (*this);
}

auto zpt::JSONElementT::prettify(std::ostream& _out, uint _n_tabs) const -> JSONElementT const& {
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
            std::string _str{ this->string() };
            zpt::json::to_unicode(_str);
            _out << "\"" << _str << "\"" << std::flush;
            break;
        }
        case zpt::JSInteger: {
            _out << this->__target.__integer << std::flush;
            break;
        }
        case zpt::JSDouble: {
            _out << std::fixed << std::setprecision(3) << this->__target.__double << std::flush;
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
        case zpt::JSRegex: {
            _out << "/" << this->__target.__regex.to_string() << "/" << std::flush;
            break;
        }
    }
    if (_n_tabs == 0) { _out << std::endl << std::flush; }
    return (*this);
}

auto zpt::JSONElementT::prettify(std::string& _out, uint _n_tabs) const -> JSONElementT const& {
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
            std::string _str{ this->string() };
            zpt::json::to_unicode(_str);
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
        case zpt::JSRegex: {
            _out.insert(_out.length(), "/");
            _out.insert(_out.length(), this->__target.__regex.to_string());
            _out.insert(_out.length(), "/");
            break;
        }
    }
    if (_n_tabs == 0) { _out.insert(_out.length(), "\n"); }
    return (*this);
}

auto zpt::JSONElementT::prettify() const -> std::string {
    std::string _out{ "" };
    this->prettify(_out);
    return _out;
}

auto zpt::JSONElementT::element(size_t _pos) -> std::tuple<size_t, std::string, zpt::json> {
    switch (this->__target.__type) {
        case zpt::JSObject:
            return std::make_tuple(
              _pos, this->__target.__object->key_for(_pos), this->__target.__object[_pos]);
        case zpt::JSArray: return std::make_tuple(_pos, std::to_string(_pos), this->__target.__array[_pos]);
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda:
        case zpt::JSRegex: break;
    }
    return std::make_tuple(0, "", zpt::json{ *this });
}
