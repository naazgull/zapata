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

zpt::JSONElementT::JSONElementT(JSONType _in) { this->type(_in); }

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

zpt::JSONElementT::JSONElementT(std::nullptr_t _rhs) { (*this) = _rhs; }

zpt::JSONElementT::JSONElementT(void* _rhs) { (*this) = _rhs; }

zpt::JSONElementT::~JSONElementT() {}

auto zpt::JSONElementT::type() const -> zpt::JSONType {
    return static_cast<zpt::JSONType>(this->__underlying.index());
}

auto zpt::JSONElementT::demangle() const -> std::string {
    switch (this->__underlying.index()) {
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
        case zpt::JSUndefined: {
            return "unefined";
        }
    }
    return "null";
}

auto zpt::JSONElementT::type(zpt::JSONType _in) -> JSONElementT& {
    switch (_in) {
        case zpt::JSObject: {
            this->__underlying = zpt::JSONObj{};
            break;
        }
        case zpt::JSArray: {
            this->__underlying = zpt::JSONArr{};
            break;
        }
        case zpt::JSString: {
            this->__underlying = std::string{};
            break;
        }
        case zpt::JSInteger: {
            this->__underlying = static_cast<long long>(0);
            break;
        }
        case zpt::JSDouble: {
            this->__underlying = static_cast<double>(0);
            break;
        }
        case zpt::JSBoolean: {
            this->__underlying = false;
            break;
        }
        case zpt::JSNil: {
            this->__underlying = nullptr;
            break;
        }
        case zpt::JSDate: {
            this->__underlying = static_cast<zpt::timestamp_t>(0);
            break;
        }
        case zpt::JSLambda: {
            this->__underlying = zpt::lambda{};
            break;
        }
        case zpt::JSRegex: {
            this->__underlying = zpt::JSONRegex{};
            break;
        }
        case zpt::JSUndefined: {
            this->__underlying = static_cast<void*>(this);
            break;
        }
    }
    return (*this);
}

auto zpt::JSONElementT::ok() const -> bool {
    return this->__underlying.index() != zpt::JSNil && this->__underlying.index() != zpt::JSUndefined;
}

auto zpt::JSONElementT::empty() const -> bool {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            return (*std::get<zpt::JSObject>(this->__underlying))->size() == 0;
        }
        case zpt::JSArray: {
            return (*std::get<zpt::JSArray>(this->__underlying))->size() == 0;
        }
        case zpt::JSString: {
            return std::get<zpt::JSString>(this->__underlying).length() == 0;
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
        case zpt::JSUndefined: {
            return false;
        }
    }
    return true;
}

auto zpt::JSONElementT::nil() const -> bool { return this->__underlying.index() == zpt::JSNil; }

auto zpt::JSONElementT::clear() -> void {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            (*std::get<zpt::JSObject>(this->__underlying))->clear();
            break;
        }
        case zpt::JSArray: {
            (*std::get<zpt::JSArray>(this->__underlying))->clear();
            break;
        }
        case zpt::JSString: {
            std::get<zpt::JSString>(this->__underlying).clear();
            break;
        }
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSDate: {
            this->__underlying = 0;
            break;
        }
        case zpt::JSNil: {
            this->__underlying = nullptr;
            break;
        }
        case zpt::JSLambda:
        case zpt::JSRegex: {
            break;
        }
    }
}

auto zpt::JSONElementT::size() const -> size_t {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            return (*std::get<zpt::JSObject>(this->__underlying))->size();
        }
        case zpt::JSArray: {
            return (*std::get<zpt::JSArray>(this->__underlying))->size();
        }
        case zpt::JSString: {
            return std::get<zpt::JSString>(this->__underlying).length();
        }
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
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            return std::get<zpt::JSObject>(this->__underlying).hash();
        }
        case zpt::JSArray: {
            return std::get<zpt::JSArray>(this->__underlying).hash();
        }
        case zpt::JSString: {
            size_t _to_return = std::hash<std::string>{}(std::get<zpt::JSString>(this->__underlying));
            return _to_return;
        }
        case zpt::JSInteger: {
            size_t _to_return = std::hash<long long>{}(std::get<zpt::JSInteger>(this->__underlying));
            return _to_return;
        }
        case zpt::JSDouble: {
            size_t _to_return = std::hash<double>{}(std::get<zpt::JSDouble>(this->__underlying));
            return _to_return;
        }
        case zpt::JSBoolean: {
            size_t _to_return = std::hash<bool>{}(std::get<zpt::JSBoolean>(this->__underlying));
            return _to_return;
        }
        case zpt::JSNil: {
            size_t _to_return = reinterpret_cast<size_t>(this);
            return _to_return;
        }
        case zpt::JSDate: {
            size_t _to_return = std::hash<unsigned long long>{}(std::get<zpt::JSDate>(this->__underlying));
            return _to_return;
        }
        case zpt::JSLambda: {
            size_t _to_return =
              std::hash<std::string>{}(std::get<zpt::JSLambda>(this->__underlying)->signature());
            return _to_return;
        }
        case zpt::JSRegex: {
            size_t _to_return =
              std::hash<std::string>{}(std::get<zpt::JSRegex>(this->__underlying).to_string());
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

auto zpt::JSONElementT::is_object() -> bool { return this->__underlying.index() == zpt::JSObject; }

auto zpt::JSONElementT::is_array() -> bool { return this->__underlying.index() == zpt::JSArray; }

auto zpt::JSONElementT::is_string() -> bool { return this->__underlying.index() == zpt::JSString; }

auto zpt::JSONElementT::is_integer() -> bool { return this->__underlying.index() == zpt::JSInteger; }

auto zpt::JSONElementT::is_floating() -> bool { return this->__underlying.index() == zpt::JSDouble; }

auto zpt::JSONElementT::is_number() -> bool {
    return this->__underlying.index() == zpt::JSInteger || this->__underlying.index() == zpt::JSDouble;
}

auto zpt::JSONElementT::is_bool() -> bool { return this->__underlying.index() == zpt::JSBoolean; }

auto zpt::JSONElementT::is_date() -> bool {
    return this->__underlying.index() == zpt::JSDate || this->__underlying.index() == zpt::JSString;
}

auto zpt::JSONElementT::is_lambda() -> bool { return this->__underlying.index() == zpt::JSLambda; }

auto zpt::JSONElementT::is_regex() -> bool { return this->__underlying.index() == zpt::JSRegex; }

auto zpt::JSONElementT::is_nil() -> bool { return this->__underlying.index() == zpt::JSNil; }

auto zpt::JSONElementT::is_undefined() -> bool { return this->__underlying.index() == zpt::JSUndefined; }

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
    expect(this->__underlying.index() == zpt::JSObject,
           std::string("this element is not of type JSObject: ") + this->stringify());
    return std::get<zpt::JSObject>(this->__underlying);
}

auto zpt::JSONElementT::array() const -> zpt::JSONArr const& {
    expect(this->__underlying.index() == zpt::JSArray,
           std::string("this element is not of type JSArray: ") + this->stringify());
    return std::get<zpt::JSArray>(this->__underlying);
}

auto zpt::JSONElementT::string() const -> std::string const& {
    expect(this->__underlying.index() == zpt::JSString,
           std::string("this element is not of type JSString: ") + this->stringify());
    return std::get<zpt::JSString>(this->__underlying);
}

auto zpt::JSONElementT::integer() const -> long long const& {
    expect(this->__underlying.index() == zpt::JSInteger,
           std::string("this element is not of type JSInteger: ") + this->stringify());
    return std::get<zpt::JSInteger>(this->__underlying);
}

auto zpt::JSONElementT::floating() const -> double const& {
    expect(this->__underlying.index() == zpt::JSDouble,
           std::string("this element is not of type JSDouble: ") + this->stringify());
    return std::get<zpt::JSDouble>(this->__underlying);
}

auto zpt::JSONElementT::boolean() const -> bool const& {
    expect(this->__underlying.index() == zpt::JSBoolean,
           std::string("this element is not of type JSBoolean: ") + this->stringify());
    return std::get<zpt::JSBoolean>(this->__underlying);
}

auto zpt::JSONElementT::date() const -> zpt::timestamp_t const& {
    expect(this->__underlying.index() == zpt::JSDate,
           std::string("this element is not of type JSDate: ") + this->stringify());
    return std::get<zpt::JSDate>(this->__underlying);
}

auto zpt::JSONElementT::lambda() const -> zpt::lambda const& {
    expect(this->__underlying.index() == zpt::JSLambda,
           std::string("this element is not of type JSLambda: ") + this->stringify());
    return std::get<zpt::JSLambda>(this->__underlying);
}

auto zpt::JSONElementT::regex() const -> zpt::regex const& {
    expect(this->__underlying.index() == zpt::JSRegex,
           std::string("this element is not of type JSRegex: ") + this->stringify());
    return std::get<zpt::JSRegex>(this->__underlying);
}

auto zpt::JSONElementT::number() const -> double const {
    expect(this->__underlying.index() == zpt::JSDate || this->__underlying.index() == zpt::JSInteger ||
             this->__underlying.index() == zpt::JSDouble || this->__underlying.index() == zpt::JSBoolean,
           std::string("this element is not of type JSInteger, JSDouble or JSBoolean: ") + this->stringify());
    switch (this->__underlying.index()) {
        case zpt::JSInteger: {
            return static_cast<double>(std::get<zpt::JSInteger>(this->__underlying));
        }
        case zpt::JSDouble: {
            return std::get<zpt::JSDouble>(this->__underlying);
        }
        case zpt::JSBoolean: {
            return static_cast<double>(std::get<zpt::JSBoolean>(this->__underlying));
        }
        case zpt::JSDate: {
            return static_cast<double>(std::get<zpt::JSDate>(this->__underlying));
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
        case zpt::JSUndefined:
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
    this->__parent = _rhs.__parent;
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::JSONElementT::operator=(JSONElementT&& _rhs) -> JSONElementT& {
    this->__parent = _rhs.__parent;
    this->__underlying = std::move(_rhs.__underlying);
    _rhs.__parent = nullptr;
    return (*this);
}

auto zpt::JSONElementT::operator=(std::string const& _rhs) -> JSONElementT& {
    this->__underlying = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(std::nullptr_t) -> JSONElementT& {
    this->__underlying = nullptr;
    return (*this);
}

auto zpt::JSONElementT::operator=(const char* _rhs) -> JSONElementT& {
    this->__underlying = std::string(_rhs);
    return (*this);
}

auto zpt::JSONElementT::operator=(long long _rhs) -> JSONElementT& {
    this->__underlying = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(double _rhs) -> JSONElementT& {
    this->__underlying = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(bool _rhs) -> JSONElementT& {
    this->__underlying = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(int _rhs) -> JSONElementT& {
    this->__underlying = static_cast<long long>(_rhs);
    return (*this);
}

auto zpt::JSONElementT::operator=(size_t _rhs) -> JSONElementT& {
    this->__underlying = static_cast<long long>(_rhs);
    return (*this);
}

#ifdef __LP64__
auto zpt::JSONElementT::operator=(unsigned int _rhs) -> JSONElementT& {
    this->__underlying = static_cast<long long>(_rhs);
    return (*this);
}
#endif

auto zpt::JSONElementT::operator=(zpt::json _rhs) -> JSONElementT& {
    (*this) = (*_rhs);
    return (*this);
}

auto zpt::JSONElementT::operator=(zpt::timestamp_t _rhs) -> JSONElementT& {
    this->__underlying = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(zpt::JSONObj& _rhs) -> JSONElementT& {
    this->__underlying = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(zpt::JSONArr& _rhs) -> JSONElementT& {
    this->__underlying = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(zpt::lambda _rhs) -> JSONElementT& {
    this->__underlying = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(zpt::regex _rhs) -> JSONElementT& {
    this->__underlying = _rhs;
    return (*this);
}

auto zpt::JSONElementT::operator=(void* _rhs) -> JSONElementT& {
    this->__underlying = _rhs;
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
        case zpt::JSUndefined:
        case zpt::JSNil: {
            _out.assign("");
            break;
        }
        case zpt::JSDate: {
            _out.assign(zpt::timestamp(this->date()));
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
            return static_cast<bool>(this->integer());
        }
        case zpt::JSDouble: {
            return static_cast<bool>(this->floating());
        }
        case zpt::JSBoolean: {
            return this->boolean();
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return false;
        }
        case zpt::JSDate: {
            return static_cast<bool>(this->date());
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
            return (*this->object())->size();
        }
        case zpt::JSArray: {
            return (*this->array())->size();
        }
        case zpt::JSString: {
            int _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return static_cast<int>(this->integer());
        }
        case zpt::JSDouble: {
            return static_cast<int>(this->floating());
        }
        case zpt::JSBoolean: {
            return static_cast<int>(this->boolean());
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return static_cast<int>(this->date());
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
            return (*this->object())->size();
        }
        case zpt::JSArray: {
            return (*this->array())->size();
        }
        case zpt::JSString: {
            long _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return static_cast<long>(this->integer());
        }
        case zpt::JSDouble: {
            return static_cast<long>(this->floating());
        }
        case zpt::JSBoolean: {
            return static_cast<long>(this->boolean());
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return static_cast<long>(this->date());
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
            return (*this->object())->size();
        }
        case zpt::JSArray: {
            return (*this->array())->size();
        }
        case zpt::JSString: {
            long long _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return static_cast<long long>(this->integer());
        }
        case zpt::JSDouble: {
            return static_cast<long long>(this->floating());
        }
        case zpt::JSBoolean: {
            return static_cast<long long>(this->boolean());
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return static_cast<long long>(this->date());
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
            return (*this->object())->size();
        }
        case zpt::JSArray: {
            return (*this->array())->size();
        }
        case zpt::JSString: {
            unsigned int _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return static_cast<unsigned int>(this->integer());
        }
        case zpt::JSDouble: {
            return static_cast<unsigned int>(this->floating());
        }
        case zpt::JSBoolean: {
            return static_cast<unsigned int>(this->boolean());
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return static_cast<unsigned int>(this->date());
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
            return (*this->object())->size();
        }
        case zpt::JSArray: {
            return (*this->array())->size();
        }
        case zpt::JSString: {
            size_t _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return static_cast<size_t>(this->integer());
        }
        case zpt::JSDouble: {
            return static_cast<size_t>(this->floating());
        }
        case zpt::JSBoolean: {
            return static_cast<size_t>(this->boolean());
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return static_cast<size_t>(this->date());
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
            return static_cast<double>((*this->object())->size());
        }
        case zpt::JSArray: {
            return static_cast<double>((*this->array())->size());
        }
        case zpt::JSString: {
            double _n = 0;
            std::string _s{ this->string().data() };
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return static_cast<double>(this->integer());
        }
        case zpt::JSDouble: {
            return static_cast<double>(this->floating());
        }
        case zpt::JSBoolean: {
            return static_cast<double>(this->boolean());
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return static_cast<double>(this->date());
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
            return 0;
        }
        case zpt::JSArray: {
            return 0;
        }
        case zpt::JSString: {
            return 0;
        }
        case zpt::JSInteger: {
            return static_cast<zpt::timestamp_t>(this->integer());
        }
        case zpt::JSDouble: {
            return static_cast<zpt::timestamp_t>(this->floating());
        }
        case zpt::JSBoolean: {
            return static_cast<zpt::timestamp_t>(this->boolean());
        }
        case zpt::JSUndefined:
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
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            std::get<zpt::JSObject>(this->__underlying)->push(_in);
            break;
        }
        case zpt::JSArray: {
            std::get<zpt::JSArray>(this->__underlying)->push(_in);
            break;
        }
        case zpt::JSString: {
            this->__underlying = _in;
            break;
        }
        case zpt::JSInteger: {
            long long _converted{ 0 };
            zpt::fromstr(_in, &_converted);
            this->__underlying = _converted;
            break;
        }
        case zpt::JSDouble: {
            double _converted{ 0 };
            zpt::fromstr(_in, &_converted);
            this->__underlying = _converted;
            break;
        }
        case zpt::JSBoolean: {
            bool _converted{ false };
            zpt::fromstr(_in, &_converted);
            this->__underlying = _converted;
            break;
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            break;
        }
        case zpt::JSDate: {
            this->__underlying = zpt::timestamp(_in);
            break;
        }
        case zpt::JSLambda: {
            break;
        }
        case zpt::JSRegex: {
            this->__underlying = zpt::regex{ _in };
            break;
        }
    }
    return (*this);
}

auto zpt::JSONElementT::operator<<(zpt::json _in) -> zpt::JSONElementT& {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            std::get<zpt::JSObject>(this->__underlying)->push(_in);
            break;
        }
        case zpt::JSArray: {
            std::get<zpt::JSArray>(this->__underlying)->push(_in);
            break;
        }
        case zpt::JSString: {
            this->__underlying = _in->string();
            break;
        }
        case zpt::JSInteger: {
            this->__underlying = _in->integer();
            break;
        }
        case zpt::JSDouble: {
            this->__underlying = _in->floating();
            break;
        }
        case zpt::JSBoolean: {
            this->__underlying = _in->boolean();
            break;
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            break;
        }
        case zpt::JSDate: {
            this->__underlying = _in->date();
            break;
        }
        case zpt::JSLambda: {
            break;
        }
        case zpt::JSRegex: {
            this->__underlying = _in->regex();
            break;
        }
    }
    return *this;
}

auto zpt::JSONElementT::operator==(zpt::JSONElementT const& _in) const -> bool {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            if (this->__underlying.index() != _in.__underlying.index()) { return false; }
            return *(this->object()) == *(_in.object());
        }
        case zpt::JSArray: {
            if (this->__underlying.index() != _in.__underlying.index()) { return false; }
            return *(this->array()) == *(_in.array());
        }
        case zpt::JSString: {
            if (this->__underlying.index() != _in.__underlying.index()) { return false; }
            return this->string() == _in.string();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->integer() == _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->floating() == _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->boolean() == _in.number();
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            if (_in.__underlying.index() == zpt::JSNil || _in.__underlying.index() == zpt::JSUndefined) {
                return true;
            }
            return false;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->date() == _in.number();
        }
        case zpt::JSLambda: {
            if (this->__underlying.index() != _in.__underlying.index()) { return false; }
            return this->lambda()->signature() == _in.lambda()->signature();
        }
        case zpt::JSRegex: {
            if (this->__underlying.index() != _in.__underlying.index() && _in.type() != zpt::JSString) {
                return false;
            }
            if (_in.type() == zpt::JSRegex) { return this->regex() == _in.regex(); }
            if (_in.type() == zpt::JSString) { return this->regex() == _in.string(); }
        }
    }
    return false;
}

auto zpt::JSONElementT::operator==(zpt::json _rhs) const -> bool { return *this == *_rhs; }

auto zpt::JSONElementT::operator!=(JSONElementT const& _in) const -> bool { return !(*this == _in); }

auto zpt::JSONElementT::operator!=(zpt::json _rhs) const -> bool { return *this != (*_rhs); }

auto zpt::JSONElementT::operator<(zpt::JSONElementT const& _in) const -> bool {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            if (this->__underlying.index() != _in.__underlying.index()) { return zpt::JSObject < _in.type(); }
            return *(this->object()) < *(_in.object());
        }
        case zpt::JSArray: {
            if (this->__underlying.index() != _in.__underlying.index()) { return zpt::JSArray < _in.type(); }
            return *(this->array()) < *(_in.array());
        }
        case zpt::JSString: {
            if (this->__underlying.index() != _in.__underlying.index()) { return zpt::JSString < _in.type(); }
            return this->string() < _in.string();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() < _in.hash();
            }
            return this->integer() < _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() < _in.hash();
            }
            return this->floating() < _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() < _in.hash();
            }
            return this->boolean() < _in.number();
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return this->hash() < _in.hash();
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() < _in.hash();
            }
            return this->date() < _in.number();
        }
        case zpt::JSLambda: {
            if (this->__underlying.index() != _in.__underlying.index()) { return this->hash() < _in.hash(); }
            return this->lambda()->n_args() < _in.lambda()->n_args();
        }
        case zpt::JSRegex: {
            return this->hash() < _in.hash();
        }
    }
    return false;
}

auto zpt::JSONElementT::operator<(zpt::json _rhs) const -> bool { return *this < *_rhs; }

auto zpt::JSONElementT::operator>(zpt::JSONElementT const& _in) const -> bool {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            if (this->__underlying.index() != _in.__underlying.index()) { return this->hash() > _in.hash(); }
            return *(this->object()) > *(_in.object());
        }
        case zpt::JSArray: {
            if (this->__underlying.index() != _in.__underlying.index()) { return this->hash() > _in.hash(); }
            return *(this->array()) > *(_in.array());
        }
        case zpt::JSString: {
            if (this->__underlying.index() != _in.__underlying.index()) { return this->hash() > _in.hash(); }
            return this->string() > _in.string();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() > _in.hash();
            }
            return this->integer() > _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() > _in.hash();
            }
            return this->floating() > _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() > _in.hash();
            }
            return this->boolean() > _in.number();
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return this->hash() > _in.hash();
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger && _in.type() != zpt::JSDouble &&
                _in.type() != zpt::JSBoolean) {
                return this->hash() > _in.hash();
            }
            return this->date() > _in.number();
        }
        case zpt::JSLambda: {
            return this->lambda()->n_args() > _in.lambda()->n_args();
        }
        case zpt::JSRegex: {
            return this->hash() > _in.hash();
        }
    }
    return false;
}

auto zpt::JSONElementT::operator>(zpt::json _rhs) const -> bool { return (*this) > *_rhs; }

auto zpt::JSONElementT::operator<=(zpt::JSONElementT const& _in) const -> bool {
    return (*this) == _in || (*this) < _in;
}

auto zpt::JSONElementT::operator<=(zpt::json _rhs) const -> bool { return (*this) <= *_rhs; }

auto zpt::JSONElementT::operator>=(zpt::JSONElementT const& _in) const -> bool {
    return (*this) == _in || (*this) > _in;
}

auto zpt::JSONElementT::operator>=(zpt::json _rhs) const -> bool { return (*this) >= *_rhs; }

auto zpt::JSONElementT::get_path(std::string const& _path, std::string const& _separator) -> zpt::json {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            return this->object()->get_path(_path, _separator);
        }
        case zpt::JSArray: {
            return this->array()->get_path(_path, _separator);
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSUndefined:
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
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            this->object()->set_path(_path, _value, _separator);
            return (*this);
        }
        case zpt::JSArray: {
            this->array()->set_path(_path, _value, _separator);
            return (*this);
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSUndefined:
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
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            this->object()->del_path(_path, _separator);
            return (*this);
        }
        case zpt::JSArray: {
            this->array()->del_path(_path, _separator);
            return (*this);
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSUndefined:
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
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            this->object()->stringify(_out);
            break;
        }
        case zpt::JSArray: {
            this->array()->stringify(_out);
            break;
        }
        case zpt::JSString: {
            std::string _str{ this->string() };
            zpt::json::to_unicode(_str);
            _out << "\"" << _str << "\"" << std::flush;
            break;
        }
        case zpt::JSInteger: {
            _out << this->integer() << std::flush;
            break;
        }
        case zpt::JSDouble: {
            _out << std::fixed << std::setprecision(3) << this->floating() << std::flush;
            break;
        }
        case zpt::JSBoolean: {
            _out << std::boolalpha << this->boolean() << std::flush;
            break;
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            _out << "null" << std::flush;
            break;
        }
        case zpt::JSDate: {
            _out << "\"" << zpt::timestamp(this->date()) << "\"" << std::flush;
            break;
        }
        case zpt::JSLambda: {
            _out << this->lambda()->signature() << std::flush;
            break;
        }
        case zpt::JSRegex: {
            _out << "/" << this->regex().to_string() << "/" << std::flush;
            break;
        }
    }
    return (*this);
}

auto zpt::JSONElementT::stringify(std::string& _out) const -> JSONElementT const& {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            this->object()->stringify(_out);
            break;
        }
        case zpt::JSArray: {
            this->array()->stringify(_out);
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
        case zpt::JSUndefined:
        case zpt::JSNil: {
            _out.insert(_out.length(), "null");
            break;
        }
        case zpt::JSDate: {
            _out.insert(_out.length(), "\"");
            _out.insert(_out.length(), zpt::timestamp(this->date()));
            _out.insert(_out.length(), "\"");
            break;
        }
        case zpt::JSLambda: {
            _out.insert(_out.length(), this->lambda()->signature());
            break;
        }
        case zpt::JSRegex: {
            _out.insert(_out.length(), "/");
            _out.insert(_out.length(), this->regex().to_string());
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
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            this->object()->prettify(_out, _n_tabs);
            break;
        }
        case zpt::JSArray: {
            this->array()->prettify(_out, _n_tabs);
            break;
        }
        case zpt::JSString: {
            std::string _str{ this->string() };
            zpt::json::to_unicode(_str);
            _out << "\"" << _str << "\"" << std::flush;
            break;
        }
        case zpt::JSInteger: {
            _out << this->integer() << std::flush;
            break;
        }
        case zpt::JSDouble: {
            _out << std::fixed << std::setprecision(3) << this->floating() << std::flush;
            break;
        }
        case zpt::JSBoolean: {
            _out << std::boolalpha << this->boolean() << std::flush;
            break;
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            _out << "null" << std::flush;
            break;
        }
        case zpt::JSDate: {
            _out << "\"" << zpt::timestamp(this->date()) << "\"" << std::flush;
            break;
        }
        case zpt::JSLambda: {
            _out << this->lambda()->signature() << std::flush;
            break;
        }
        case zpt::JSRegex: {
            _out << "/" << this->regex().to_string() << "/" << std::flush;
            break;
        }
    }
    if (_n_tabs == 0) { _out << std::endl << std::flush; }
    return (*this);
}

auto zpt::JSONElementT::prettify(std::string& _out, uint _n_tabs) const -> JSONElementT const& {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            this->object()->prettify(_out, _n_tabs);
            break;
        }
        case zpt::JSArray: {
            this->array()->prettify(_out, _n_tabs);
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
        case zpt::JSUndefined:
        case zpt::JSNil: {
            _out.insert(_out.length(), "null");
            break;
        }
        case zpt::JSDate: {
            _out.insert(_out.length(), "\"");
            _out.insert(_out.length(), zpt::timestamp(this->date()));
            _out.insert(_out.length(), "\"");
            break;
        }
        case zpt::JSLambda: {
            _out.insert(_out.length(), this->lambda()->signature());
            break;
        }
        case zpt::JSRegex: {
            _out.insert(_out.length(), "/");
            _out.insert(_out.length(), this->regex().to_string());
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
    switch (this->__underlying.index()) {
        case zpt::JSObject: return std::make_tuple(_pos, this->object()->key_for(_pos), this->object()[_pos]);
        case zpt::JSArray: return std::make_tuple(_pos, std::to_string(_pos), this->array()[_pos]);
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSUndefined:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda:
        case zpt::JSRegex: break;
    }
    return std::make_tuple(0, "", zpt::json{ *this });
}
