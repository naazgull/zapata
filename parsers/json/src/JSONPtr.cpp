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
#include <zapata/exceptions/SyntaxErrorException.h>
#include <zapata/json/JSONClass.h>
#include <zapata/json/JSONParser.h>

zpt::pretty::pretty(std::string const& _rhs)
  : __underlying{ _rhs } {}

zpt::pretty::pretty(const char* _rhs)
  : __underlying{ _rhs } {}

zpt::pretty::pretty(const pretty& _rhs) { (*this) = _rhs; }

zpt::pretty::pretty(pretty&& _rhs) { (*this) = _rhs; }

auto zpt::pretty::operator=(const pretty& _rhs) -> pretty& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::pretty::operator=(pretty&& _rhs) -> pretty& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::pretty::operator->() -> std::string* { return &this->__underlying; }

auto zpt::pretty::operator*() -> std::string& { return this->__underlying; }

zpt::pretty::operator std::string() { return this->__underlying; }

/*JSON POINTER TO ELEMENT*/
zpt::json::json()
  : __underlying{ std::make_shared<zpt::JSONElementT>() } {}

zpt::json::json(std::nullptr_t)
  : __underlying{ nullptr } {}

zpt::json::json(const zpt::json& _rhs) { (*this) = _rhs; }

zpt::json::json(zpt::json&& _rhs) { (*this) = _rhs; }

zpt::json::json(std::unique_ptr<zpt::JSONElementT> _target)
  : __underlying{ _target.release() } {}

zpt::json::json(std::initializer_list<zpt::json> _init) { (*this) = _init; }

zpt::json::json(std::tuple<size_t, std::string, zpt::json> _rhs) { (*this) = _rhs; }

zpt::json::~json() {}

auto zpt::json::size() const -> size_t { return this->__underlying->size(); }

auto zpt::json::hash() const -> size_t { return this->__underlying->hash(); }

auto zpt::json::value() -> zpt::JSONElementT& {
    if (this->__underlying.get() == nullptr) { return *(zpt::undefined.__underlying.get()); }
    return *(this->__underlying.get());
}

auto zpt::json::operator=(const zpt::json& _rhs) -> zpt::json& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::json::operator=(zpt::json&& _rhs) -> zpt::json& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::json::operator=(std::tuple<size_t, std::string, zpt::json> _rhs) -> zpt::json& {
    this->__underlying = std::get<2>(_rhs).__underlying;
    return (*this);
}

auto zpt::json::operator=(std::initializer_list<zpt::json> _list) -> zpt::json& {
    if (_list.size() == 0) { return (*this); }

    auto _head = *_list.begin();
    if (_list.size() == 1) {
        (*this) = _head;
        return (*this);
    }

    auto _is_array = (_list.size() > 1 && _head->type() == zpt::JSNil);

    expect(_is_array || (_list.size() % 2 == 0 && _head->type() == zpt::JSString),
           "initializer list parameter doesn't seem either an array or an object");

    this->__underlying =
      std::make_shared<zpt::JSONElementT>(_is_array ? zpt::JSArray : zpt::JSObject);

    size_t _idx{ 0 };
    for (auto _element : _list) {
        if (_is_array && _idx == 0) {
            ++_idx;
            continue;
        }

        if (!_is_array && _idx % 2 == 0) {
            this->__underlying->object()->push(_element->string());
            ++_idx;
            continue;
        }

        if (_is_array) { this->__underlying->array()->push(_element); }
        else { this->__underlying->object()->push(_element); }
        ++_idx;
    }
    return (*this);
}

auto zpt::json::operator->() -> zpt::JSONElementT* { return this->__underlying.get(); }

auto zpt::json::operator*() -> zpt::JSONElementT& { return *this->__underlying.get(); }

auto zpt::json::operator->() const -> zpt::JSONElementT const* { return this->__underlying.get(); }

auto zpt::json::operator*() const -> zpt::JSONElementT const& { return *this->__underlying.get(); }

auto zpt::json::operator==(std::tuple<size_t, std::string, zpt::json> _rhs) const -> bool {
    return (*this) == std::get<2>(_rhs);
}

auto zpt::json::operator!=(std::tuple<size_t, std::string, zpt::json> _rhs) const -> bool {
    return (*this) != std::get<2>(_rhs);
}

auto zpt::json::operator==(std::nullptr_t) const -> bool {
    return this->__underlying->type() == zpt::JSNil;
}

auto zpt::json::operator!=(std::nullptr_t) const -> bool {
    return this->__underlying->type() != zpt::JSNil;
}

auto zpt::json::operator<<(std::initializer_list<zpt::json> _in) -> zpt::json& {
    (*this->__underlying.get()) << _in;
    return (*this);
}

zpt::json::operator std::string() { return this->__underlying.get()->operator std::string(); }

zpt::json::operator bool() { return this->__underlying.get()->operator bool(); }

zpt::json::operator int() { return this->__underlying.get()->operator int(); }

zpt::json::operator long() { return this->__underlying.get()->operator long(); }

zpt::json::operator long long() { return this->__underlying.get()->operator long long(); }

zpt::json::operator size_t() { return this->__underlying.get()->operator size_t(); }

zpt::json::operator double() { return this->__underlying.get()->operator double(); }

#ifdef __LP64__
zpt::json::operator unsigned int() { return this->__underlying.get()->operator unsigned int(); }

#endif
zpt::json::operator zpt::timestamp_t() {
    return this->__underlying.get()->operator zpt::timestamp_t();
}

zpt::json::operator zpt::JSONObj() { return this->__underlying.get()->operator zpt::JSONObj(); }

zpt::json::operator zpt::JSONArr() { return this->__underlying.get()->operator zpt::JSONArr(); }

zpt::json::operator zpt::JSONObj&() { return this->__underlying.get()->operator zpt::JSONObj&(); }

zpt::json::operator zpt::JSONArr&() { return this->__underlying.get()->operator zpt::JSONArr&(); }

zpt::json::operator zpt::lambda() { return this->__underlying.get()->operator zpt::lambda(); }

zpt::json::operator zpt::regex() { return this->__underlying.get()->operator zpt::regex(); }

zpt::json::operator zpt::regex&() { return this->__underlying.get()->operator zpt::regex&(); }

zpt::json::operator std::regex&() {
    return this->__underlying.get()->regex().operator std::regex&();
}

zpt::json::operator std::string() const { return this->__underlying.get()->operator std::string(); }

zpt::json::operator bool() const { return this->__underlying.get()->operator bool(); }

zpt::json::operator int() const { return this->__underlying.get()->operator int(); }

zpt::json::operator long() const { return this->__underlying.get()->operator long(); }

zpt::json::operator long long() const { return this->__underlying.get()->operator long long(); }

zpt::json::operator size_t() const { return this->__underlying.get()->operator size_t(); }

zpt::json::operator double() const { return this->__underlying.get()->operator double(); }

#ifdef __LP64__
zpt::json::operator unsigned int() const {
    return this->__underlying.get()->operator unsigned int();
}

#endif
zpt::json::operator zpt::timestamp_t() const {
    return this->__underlying.get()->operator zpt::timestamp_t();
}

zpt::json::operator zpt::JSONObj() const {
    return this->__underlying.get()->operator zpt::JSONObj();
}

zpt::json::operator zpt::JSONArr() const {
    return this->__underlying.get()->operator zpt::JSONArr();
}

zpt::json::operator zpt::JSONObj&() const {
    return this->__underlying.get()->operator zpt::JSONObj&();
}

zpt::json::operator zpt::JSONArr&() const {
    return this->__underlying.get()->operator zpt::JSONArr&();
}

zpt::json::operator zpt::lambda() const { return this->__underlying.get()->operator zpt::lambda(); }

zpt::json::operator zpt::regex() const { return this->__underlying.get()->operator zpt::regex(); }

zpt::json::operator zpt::regex&() const { return this->__underlying.get()->operator zpt::regex&(); }

zpt::json::operator std::regex&() const {
    return this->__underlying.get()->regex().operator std::regex&();
}

auto zpt::json::operator+(std::initializer_list<zpt::json> _rhs) const -> zpt::json {
    return this->operator+(zpt::json{ _rhs });
}

auto zpt::json::operator+=(std::initializer_list<zpt::json> _rhs) -> zpt::json& {
    return this->operator+=(zpt::json{ _rhs });
}

auto zpt::json::operator-(std::initializer_list<zpt::json> _rhs) const -> zpt::json {
    return this->operator-(zpt::json{ _rhs });
}

auto zpt::json::operator-=(std::initializer_list<zpt::json> _rhs) -> zpt::json& {
    return this->operator-=(zpt::json{ _rhs });
}

auto zpt::json::operator/(std::initializer_list<zpt::json> _rhs) const -> zpt::json {
    return this->operator/(zpt::json{ _rhs });
}

auto zpt::json::operator|(std::initializer_list<zpt::json> _rhs) const -> zpt::json {
    return this->operator|(zpt::json{ _rhs });
}

auto zpt::json::operator|=(std::initializer_list<zpt::json> _rhs) -> json& {
    return this->operator|=(zpt::json{ _rhs });
}

auto zpt::json::operator&(std::initializer_list<zpt::json> _rhs) const -> json {
    return this->operator&(zpt::json{ _rhs });
}

auto zpt::json::operator&=(std::initializer_list<zpt::json> _rhs) -> json& {
    return this->operator&=(zpt::json{ _rhs });
}

auto zpt::json::operator+(zpt::json _rhs) const -> zpt::json {
    if (this->__underlying->type() == zpt::JSNil) { return _rhs->clone(); }
    if (_rhs->type() == zpt::JSNil) { return this->__underlying->clone(); }
    switch (this->__underlying->type()) {
        case zpt::JSObject: {
            auto _lhs = this->__underlying->clone();
            for (auto [_idx, _key, _e] : _rhs) {
                if (_lhs[_key]->type() == zpt::JSObject || _lhs[_key]->type() == zpt::JSArray) {
                    _lhs << _key << (_lhs[_key] + _e);
                }
                else { _lhs << _key << _e; }
            }
            return _lhs;
        }
        case zpt::JSArray: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = this->__underlying->clone();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << _e; }
                return _lhs;
            }
            else {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : *this) { _lhs << (_e + _rhs); }
                return _lhs;
            }
        }
        case zpt::JSString: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) + _e); }
                return _lhs;
            }
            else {
                return zpt::json{ this->__underlying->string() +
                                  static_cast<std::string>(_rhs->string()) };
            }
        }
        case zpt::JSInteger: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) + _e); }
                return _lhs;
            }
            else { return zpt::json{ this->__underlying->integer() + _rhs->number() }; }
        }
        case zpt::JSDouble: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) + _e); }
                return _lhs;
            }
            else { return zpt::json{ this->__underlying->floating() + _rhs->number() }; }
        }
        case zpt::JSBoolean: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) + _e); }
                return _lhs;
            }
            else { return zpt::json{ this->__underlying->boolean() || _rhs->number() }; }
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) + _e); }
                return _lhs;
            }
            else {
                return zpt::json{ static_cast<zpt::timestamp_t>(this->__underlying->date() +
                                                                _rhs->number()) };
            }
        }
        case zpt::JSLambda: {
            return zpt::undefined;
        }
        case zpt::JSRegex: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto zpt::json::operator+=(zpt::json _rhs) -> zpt::json& {
    if (this->__underlying->type() == zpt::JSNil) {
        (*this) = _rhs->clone();
        return (*this);
    }
    if (_rhs->type() == zpt::JSNil) { return (*this); }
    switch (this->__underlying->type()) {
        case zpt::JSObject: {
            for (auto [_, _key, _e] : _rhs) {
                if ((*this)[_key]->ok()) { (*this)[_key] += _e; }
                else { (*this) << _key << _e; }
            }
            return (*this);
        }
        case zpt::JSArray: {
            for (auto [_, __, _e] : _rhs) { (*this) << _e; }
            return (*this);
        }
        case zpt::JSString: {
            this->__underlying->string().insert(this->__underlying->string().length(),
                                                static_cast<std::string>(_rhs));
            return (*this);
        }
        case zpt::JSInteger: {
            this->__underlying->integer() += _rhs->number();
            return (*this);
        }
        case zpt::JSDouble: {
            this->__underlying->floating() += _rhs->number();
            return (*this);
        }
        case zpt::JSBoolean: {
            this->__underlying->boolean() += _rhs->number();
            return (*this);
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return (*this);
        }
        case zpt::JSDate: {
            this->__underlying->date() += _rhs->number();
            return (*this);
        }
        case zpt::JSLambda: {
            return zpt::undefined;
        }
        case zpt::JSRegex: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto zpt::json::operator-(zpt::json _rhs) const -> zpt::json {
    if (this->__underlying->type() == zpt::JSNil) { return _rhs->clone(); }
    if (_rhs->type() == zpt::JSNil) { return this->__underlying->clone(); }
    switch (this->__underlying->type()) {
        case zpt::JSObject: {
            auto _lhs = this->__underlying->clone();
            for (auto [_idx, _key, _e] : _rhs) { _lhs->object()->pop(_key); }
            return _lhs;
        }
        case zpt::JSArray: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << (this[_idx] - _rhs[_idx]); }
                return _lhs;
            }
            else {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << (_e - _rhs); }
                return _lhs;
            }
        }
        case zpt::JSString: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) - _e); }
                return _lhs;
            }
            else {
                std::string _lhs{ this->__underlying->string().data() };
                std::string _rhs_str = static_cast<std::string>(_rhs);
                std::size_t _idx = 0;
                while ((_idx = _lhs.find(_rhs_str, _idx)) != std::string::npos) {
                    _lhs.erase(_idx, _rhs_str.length());
                }
                return zpt::json{ _lhs };
            }
        }
        case zpt::JSInteger: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) - _e); }
                return _lhs;
            }
            else { return zpt::json{ this->__underlying->integer() - _rhs->number() }; }
        }
        case zpt::JSDouble: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) - _e); }
                return _lhs;
            }
            else { return zpt::json{ this->__underlying->floating() - _rhs->number() }; }
        }
        case zpt::JSBoolean: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) - _e); }
                return _lhs;
            }
            else { return zpt::json{ this->__underlying->boolean() && _rhs->number() }; }
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) - _e); }
                return _lhs;
            }
            else { return zpt::json{ this->__underlying->date() - _rhs->number() }; }
        }
        case zpt::JSLambda: {
            return zpt::undefined;
        }
        case zpt::JSRegex: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto zpt::json::operator-=(zpt::json _rhs) -> zpt::json& {
    if (this->__underlying->type() == zpt::JSNil) {
        (*this) = _rhs->clone();
        return (*this);
    }
    if (_rhs->type() == zpt::JSNil) { return (*this); }
    switch (this->__underlying->type()) {
        case zpt::JSObject: {
            for (auto [_, _key, __] : _rhs) { (**this).object()->pop(_key); }
            return (*this);
        }
        case zpt::JSArray: {
            for (auto [_idx, _, __] : _rhs) { (**this).array()->pop(_idx); }
            return (*this);
        }
        case zpt::JSString: {
            zpt::replace(this->__underlying->string(), static_cast<std::string>(_rhs), "");
            return (*this);
        }
        case zpt::JSInteger: {
            this->__underlying->integer() -= _rhs->number();
            return (*this);
        }
        case zpt::JSDouble: {
            this->__underlying->floating() = _rhs->number();
            return (*this);
        }
        case zpt::JSBoolean: {
            this->__underlying->boolean() -= _rhs->number();
            return (*this);
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return (*this);
        }
        case zpt::JSDate: {
            this->__underlying->date() -= _rhs->number();
            return (*this);
        }
        case zpt::JSLambda: {
            return zpt::undefined;
        }
        case zpt::JSRegex: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto zpt::json::operator/(zpt::json _rhs) const -> zpt::json {
    if (this->__underlying->type() == zpt::JSNil) { return _rhs->clone(); }
    if (_rhs->type() == zpt::JSNil) { return this->__underlying->clone(); }
    switch (this->__underlying->type()) {
        case zpt::JSObject: {
            expect(this->__underlying->type() == zpt::JSObject, "can't divide JSON objects");
            break;
        }
        case zpt::JSArray: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << (this[_idx] / _rhs[_idx]); }
                return _lhs;
            }
            else {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << (_e / _rhs); }
                return _lhs;
            }
        }
        case zpt::JSString: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) / _e); }
                return _lhs;
            }
            else {
                std::string _lhs{ this->__underlying->string().data() };
                std::string _rhs_str = static_cast<std::string>(_rhs);
                std::size_t _idx = 0;
                while ((_idx = _lhs.find(_rhs_str, _idx)) != std::string::npos) {
                    _lhs.erase(_idx, _rhs_str.length());
                }
                return zpt::json{ _lhs };
            }
        }
        case zpt::JSInteger: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) / _e); }
                return _lhs;
            }
            else { return zpt::json{ this->__underlying->integer() / _rhs->number() }; }
        }
        case zpt::JSDouble: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) / _e); }
                return _lhs;
            }
            else { return zpt::json{ this->__underlying->floating() / _rhs->number() }; }
        }
        case zpt::JSBoolean: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) / _e); }
                return _lhs;
            }
            else { return zpt::json{ this->__underlying->boolean() / _rhs->number() }; }
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            if (_rhs->type() == zpt::JSArray) {
                auto _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : _rhs) { _lhs << ((*this) / _e); }
                return _lhs;
            }
            else { return zpt::json{ this->__underlying->date() / _rhs->number() }; }
        }
        case zpt::JSLambda: {
            return zpt::undefined;
        }
        case zpt::JSRegex: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto zpt::json::operator|(zpt::json _rhs) const -> zpt::json {
    if (this->__underlying->type() == zpt::JSNil) { return _rhs->clone(); }
    if (_rhs->type() == zpt::JSNil) { return this->__underlying->clone(); }
    switch (this->__underlying->type()) {
        case zpt::JSObject:
        case zpt::JSArray: {
            auto _lhs = this->__underlying->clone();
            _lhs.strict_union(_rhs);
            return _lhs;
        }
        case zpt::JSString: {
            if (_rhs->type() == zpt::JSString) {
                return zpt::json{ this->__underlying->string() + _rhs->string() };
            }
            else {
                return zpt::json{ this->__underlying->string() + static_cast<std::string>(_rhs) };
            }
        }
        case zpt::JSInteger: {
            return zpt::json{ this->__underlying->integer() | static_cast<long>(_rhs) };
        }
        case zpt::JSDouble: {
            return zpt::json{ static_cast<unsigned long long>(*this) |
                              static_cast<unsigned long long>(_rhs) };
        }
        case zpt::JSBoolean: {
            return zpt::json{ this->__underlying->boolean() | static_cast<bool>(_rhs) };
        }
        case zpt::JSDate: {
            return zpt::json{ static_cast<unsigned long long>(*this) |
                              static_cast<unsigned long long>(_rhs) };
        }
        case zpt::JSUndefined:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSRegex: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto zpt::json::operator|=(zpt::json _rhs) -> zpt::json& {
    if (this->__underlying->type() == zpt::JSNil) {
        (*this) = _rhs->clone();
        return (*this);
    }
    if (_rhs->type() == zpt::JSNil) { return (*this); }
    switch (this->__underlying->type()) {
        case zpt::JSObject:
        case zpt::JSArray: {
            this->strict_union(_rhs);
            return (*this);
        }
        case zpt::JSString: {
            if (_rhs->type() == zpt::JSString) {
                (*this) = this->__underlying->string() + _rhs->string();
            }
            else { (*this) = this->__underlying->string() + static_cast<std::string>(_rhs); }
            return (*this);
        }
        case zpt::JSInteger: {
            (*this) = this->__underlying->integer() | static_cast<long>(_rhs);
            return (*this);
        }
        case zpt::JSDouble: {
            (*this) = static_cast<unsigned long long>(this->__underlying->floating()) |
                      static_cast<unsigned long long>(_rhs);
            return (*this);
        }
        case zpt::JSBoolean: {
            (*this) = this->__underlying->boolean() | static_cast<bool>(_rhs);
            return (*this);
        }
        case zpt::JSDate: {
            (*this) = static_cast<unsigned long long>(this->__underlying->date()) |
                      static_cast<unsigned long long>(_rhs);
            return (*this);
        }
        case zpt::JSUndefined:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSRegex: {
            return (*this);
        }
    }
    return (*this);
}

auto zpt::json::operator&(zpt::json _rhs) const -> zpt::json {
    if (this->__underlying->type() == zpt::JSNil) { return _rhs->clone(); }
    if (_rhs->type() == zpt::JSNil) { return this->__underlying->clone(); }
    switch (this->__underlying->type()) {
        case zpt::JSObject:
        case zpt::JSArray: {
            auto _lhs = this->__underlying->clone();
            _lhs.strict_intersection(_rhs);
            return _lhs;
        }
        case zpt::JSString: {
            std::string _lhs;
            if (_rhs->type() == zpt::JSString) {
                std::set_intersection(this->__underlying->string().begin(),
                                      this->__underlying->string().end(),
                                      _rhs.__underlying->string().begin(),
                                      _rhs.__underlying->string().end(),
                                      std::back_inserter(_lhs));
            }
            else {
                std::string _rhs_str = static_cast<std::string>(_rhs);
                std::set_intersection(this->__underlying->string().begin(),
                                      this->__underlying->string().end(),
                                      _rhs_str.begin(),
                                      _rhs_str.end(),
                                      std::back_inserter(_lhs));
            }
            return zpt::json{ _lhs };
        }
        case zpt::JSInteger: {
            return zpt::json{ this->__underlying->integer() & static_cast<long>(_rhs) };
        }
        case zpt::JSDouble: {
            return zpt::json{ static_cast<unsigned long long>(*this) &
                              static_cast<unsigned long long>(_rhs) };
        }
        case zpt::JSBoolean: {
            return zpt::json{ this->__underlying->boolean() & static_cast<bool>(_rhs) };
        }
        case zpt::JSDate: {
            return zpt::json{ static_cast<unsigned long long>(*this) &
                              static_cast<unsigned long long>(_rhs) };
        }
        case zpt::JSUndefined:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSRegex: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto zpt::json::operator&=(zpt::json _rhs) -> zpt::json& {
    if (this->__underlying->type() == zpt::JSNil) {
        (*this) = _rhs->clone();
        return (*this);
    }
    if (_rhs->type() == zpt::JSNil) { return (*this); }
    switch (this->__underlying->type()) {
        case zpt::JSObject:
        case zpt::JSArray: {
            this->strict_intersection(_rhs);
            return (*this);
        }
        case zpt::JSString: {
            std::string _lhs;
            if (_rhs->type() == zpt::JSString) {
                std::set_intersection(this->__underlying->string().begin(),
                                      this->__underlying->string().end(),
                                      _rhs.__underlying->string().begin(),
                                      _rhs.__underlying->string().end(),
                                      std::back_inserter(_lhs));
            }
            else {
                std::string _rhs_str = static_cast<std::string>(_rhs);
                std::set_intersection(this->__underlying->string().begin(),
                                      this->__underlying->string().end(),
                                      _rhs_str.begin(),
                                      _rhs_str.end(),
                                      std::back_inserter(_lhs));
            }
            (*this) = _lhs;
            return (*this);
        }
        case zpt::JSInteger: {
            (*this) = this->__underlying->integer() & static_cast<long>(_rhs);
            return (*this);
        }
        case zpt::JSDouble: {
            (*this) = static_cast<unsigned long long>(this->__underlying->floating()) &
                      static_cast<unsigned long long>(_rhs);
            return (*this);
        }
        case zpt::JSBoolean: {
            (*this) = this->__underlying->boolean() & static_cast<bool>(_rhs);
            return (*this);
        }
        case zpt::JSDate: {
            (*this) = static_cast<unsigned long long>(this->__underlying->date()) &
                      static_cast<unsigned long long>(_rhs);
            return (*this);
        }
        case zpt::JSUndefined:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSRegex: {
            return (*this);
        }
    }
    return (*this);
}

auto zpt::json::strict_union(zpt::json _rhs) -> void {
    if (this->__underlying->type() == zpt::JSNil) { return; }
    if (_rhs->type() == zpt::JSNil) { return; }
    switch (this->__underlying->type()) {
        case zpt::JSObject: {
            if (_rhs->type() == zpt::JSObject) {
                for (auto [_, _key, _e] : _rhs) {
                    if ((*this)[_key]->type() == zpt::JSObject) { (*this)[_key].strict_union(_e); }
                    else if (!(*this)[_key]->ok()) { (*this) << _key << _e; }
                }
            }
            return;
        }
        case zpt::JSArray: {
            if (_rhs->type() == zpt::JSArray) {
                for (auto [_, __, _e] : _rhs) { (*this) << _e; }
            }
            return;
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSUndefined:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda:
        case zpt::JSRegex: {
        }
    }
}

auto zpt::json::strict_intersection(zpt::json _rhs) -> void {
    if (this->__underlying->type() == zpt::JSNil) { return; }
    if (_rhs->type() == zpt::JSNil) { return; }
    switch (this->__underlying->type()) {
        case zpt::JSObject: {
            if (_rhs->type() == zpt::JSObject) {
                for (auto [_, _key, _e] : _rhs) {
                    if ((*this)(_key)->type() == zpt::JSObject ||
                        (*this)(_key)->type() == zpt::JSArray) {
                        (*this)[_key].strict_intersection(_e);
                    }
                }
                zpt::json _lhs = zpt::json::object();
                for (auto [_, _key, _e] : (*this)) {
                    if (_rhs(_key)->ok()) { _lhs << _key << _e; }
                }
                (*this) = _lhs;
            }
            return;
        }
        case zpt::JSArray: {
            if (_rhs->type() == zpt::JSArray) {
                for (auto [_idx, _, _e] : _rhs) {
                    if ((*this)(_idx)->type() == zpt::JSObject ||
                        (*this)(_idx)->type() == zpt::JSArray) {
                        (*this)[_idx].strict_intersection(_e);
                    }
                }
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _, _e] : (*this)) {
                    if (_e == _rhs(_idx)) { _lhs << _e; }
                }
                (*this) = _lhs;
            }
            return;
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSUndefined:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda:
        case zpt::JSRegex: {
        }
    }
}

auto zpt::json::load_from(std::string const& _in) -> zpt::json& {
    std::istringstream _iss;
    _iss.str(_in);
    this->load_from(_iss);
    return (*this);
}

auto zpt::json::load_from(std::istream& _in) -> zpt::json& {
    static thread_local zpt::JSONParser _thread_local_parser;
    _thread_local_parser.switchRoots(*this);
    _thread_local_parser.switchStreams(_in);
    try {
        _thread_local_parser.parse();
    }
    catch (zpt::SyntaxErrorException const& _e) {
        throw;
    }
    catch (...) {
    }
    return (*this);
}

auto zpt::json::stringify(std::ostream& _out) -> zpt::json& {
    static_cast<zpt::json const&>(*this).stringify(_out);
    return (*this);
}

auto zpt::json::stringify(std::string& _out) -> zpt::json& {
    static_cast<zpt::json const&>(*this).stringify(_out);
    return (*this);
}

auto zpt::json::stringify(std::ostream& _out) const -> zpt::json const& {
    this->__underlying->stringify(_out);
    return (*this);
}

auto zpt::json::stringify(std::string& _out) const -> zpt::json const& {
    this->__underlying->stringify(_out);
    return (*this);
}

auto zpt::json::begin() -> zpt::json::iterator { return zpt::json::iterator{ *this, 0 }; }

auto zpt::json::end() -> zpt::json::iterator {
    return zpt::json::iterator{ *this, std::numeric_limits<size_t>::max() };
}

auto zpt::json::begin() const -> zpt::json::const_iterator {
    return zpt::json::const_iterator{ *this, 0 };
}

auto zpt::json::end() const -> zpt::json::const_iterator {
    return zpt::json::const_iterator{ *this, std::numeric_limits<size_t>::max() };
}

auto zpt::json::parse_json_str(std::string const& _in) -> zpt::json {
    zpt::json _to_return;
    _to_return.load_from(_in);
    return _to_return;
}

auto zpt::json::to_unicode(std::string& _str) -> void { zpt::utf8::encode(_str, true); }

auto zpt::json::object() -> zpt::json {
    zpt::JSONObj _empty;
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_empty) };
}

auto zpt::json::array() -> zpt::json {
    zpt::JSONArr _empty;
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_empty) };
}

auto zpt::json::date(std::string const& _e) -> zpt::json {
    zpt::timestamp_t _v(zpt::timestamp(_e));
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}

auto zpt::json::date() -> zpt::json {
    zpt::timestamp_t _v((zpt::timestamp_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count());
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}

auto zpt::json::lambda(std::string const& _name, unsigned short _n_args) -> zpt::json {
    zpt::lambda _v(_name, _n_args);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}

auto zpt::json::type_of(std::string const&) -> zpt::JSONType { return zpt::JSString; }

auto zpt::json::type_of(bool) -> zpt::JSONType { return zpt::JSBoolean; }

auto zpt::json::type_of(int) -> zpt::JSONType { return zpt::JSInteger; }

auto zpt::json::type_of(long) -> zpt::JSONType { return zpt::JSInteger; }

auto zpt::json::type_of(long long) -> zpt::JSONType { return zpt::JSInteger; }

auto zpt::json::type_of(size_t) -> zpt::JSONType { return zpt::JSInteger; }

auto zpt::json::type_of(double) -> zpt::JSONType { return zpt::JSDouble; }

#ifdef __LP64__
auto zpt::json::type_of(unsigned int) -> zpt::JSONType { return zpt::JSInteger; }
#endif

auto zpt::json::type_of(zpt::JSONElementT& _value) -> zpt::JSONType { return _value.type(); }

auto zpt::json::type_of(zpt::timestamp_t) -> zpt::JSONType { return zpt::JSDate; }

auto zpt::json::type_of(zpt::pretty) -> zpt::JSONType { return zpt::JSString; }

auto zpt::json::type_of(zpt::JSONObj) -> zpt::JSONType { return zpt::JSObject; }

auto zpt::json::type_of(zpt::JSONArr) -> zpt::JSONType { return zpt::JSArray; }

auto zpt::json::type_of(zpt::JSONObj&) -> zpt::JSONType { return zpt::JSObject; }

auto zpt::json::type_of(zpt::JSONArr&) -> zpt::JSONType { return zpt::JSArray; }

auto zpt::json::type_of(zpt::lambda) -> zpt::JSONType { return zpt::JSLambda; }

auto zpt::json::type_of(zpt::regex&) -> zpt::JSONType { return zpt::JSRegex; }

auto zpt::json::type_of(zpt::json& _value) -> zpt::JSONType { return _value->type(); }

auto zpt::json::traverse(zpt::json _document, zpt::json::traverse_callback _callback) -> void {
    zpt::json::traverse(_document, _callback, "");
}

auto zpt::json::traverse(zpt::json _document,
                         zpt::json::traverse_callback _callback,
                         std::string _path) -> void {
    switch (_document->type()) {
        case zpt::JSArray:
        case zpt::JSObject: {
            for (auto [_idx, _name, _item] : _document) {
                std::string _current_path{ _document->type() == zpt::JSArray ? std::to_string(_idx)
                                                                             : _name };
                std::string _item_path{ _path + std::string{ _path.length() == 0 ? "" : "." } +
                                        _current_path };
                _callback(_name, _item, _item_path);
                if (_item->type() == zpt::JSObject || _item->type() == zpt::JSArray) {
                    zpt::json::traverse(_item, _callback, _item_path);
                }
            }
            break;
        }
        default: {
            _callback("", _document, _path);
            break;
        }
    }
}

auto zpt::json::flatten(zpt::json _document) -> zpt::json {
    if (_document->type() == zpt::JSObject || _document->type() == zpt::JSArray) {
        auto _return = zpt::json::object();
        zpt::json::traverse(
          _document, [&](std::string const&, zpt::json _item, std::string const& _path) -> void {
              if (_item->type() != zpt::JSObject && _item->type() != zpt::JSArray) {
                  _return << _path << _item;
              }
          });
        return _return;
    }
    return { ".", _document };
}

auto zpt::json::find(zpt::json::iterator _begin, zpt::json::iterator _end, zpt::json _to_find)
  -> zpt::json::iterator {
    for (zpt::json::iterator _to_return = _begin; _to_return != _end; ++_to_return) {
        if (std::get<2>(*_to_return) == _to_find) { return _to_return; }
    }
    return _end;
}

zpt::JSONIterator::JSONIterator(zpt::json const& _target, size_t _pos)
  : __target{ _target }
  , __index{ _pos } {
    switch (_target->type()) {
        case zpt::JSObject: {
            if (_pos == std::numeric_limits<size_t>::max()) {
                this->__index = (**this->__target->object()).size();
                this->__iterator = (**this->__target->object()).end();
            }
            else {
                try {
                    this->__iterator =
                      (**this->__target->object()).find(_target->object()->key_for(_pos));
                }
                catch (zpt::failed_expectation const& _e) {
                    this->__index = (**this->__target->object()).size();
                    this->__iterator = (**this->__target->object()).end();
                }
            }
            break;
        }
        case zpt::JSArray: {
            if (_pos == std::numeric_limits<size_t>::max()) {
                this->__index = (**this->__target->array()).size();
            }
            break;
        }
        default: {
            this->__index = std::numeric_limits<size_t>::max();
            break;
        }
    }
}

zpt::JSONIterator::JSONIterator(JSONIterator const& _rhs)
  : __target{ _rhs.__target }
  , __index{ _rhs.__index }
  , __iterator{ _rhs.__iterator } {}

zpt::JSONIterator::JSONIterator(JSONIterator&& _rhs)
  : __target{ std::move(_rhs.__target) }
  , __index{ _rhs.__index }
  , __iterator{ std::move(_rhs.__iterator) } {
    _rhs.__index = 0;
}

auto zpt::JSONIterator::operator=(JSONIterator const& _rhs) -> JSONIterator& {
    this->__target = _rhs.__target;
    this->__index = _rhs.__index;
    this->__iterator = _rhs.__iterator;
    return (*this);
}

auto zpt::JSONIterator::operator=(JSONIterator&& _rhs) -> JSONIterator& {
    this->__target = std::move(_rhs.__target);
    this->__index = _rhs.__index;
    this->__iterator = std::move(_rhs.__iterator);
    _rhs.__index = 0;
    return (*this);
}

auto zpt::JSONIterator::operator++() -> JSONIterator& {
    ++this->__index;
    if (this->__target->type() == zpt::JSObject) { ++this->__iterator; }
    return (*this);
}

auto zpt::JSONIterator::operator*() -> reference {
    switch (this->__target->type()) {
        case zpt::JSObject: {
            return std::make_tuple(
              this->__index, this->__iterator->first, this->__iterator->second);
        }
        case zpt::JSArray: {
            return std::make_tuple(this->__index, "", (**this->__target->array())[this->__index]);
        }
        default: {
            break;
        }
    }
    return std::make_tuple(this->__index, "", this->__target);
}

auto zpt::JSONIterator::operator++(int) -> JSONIterator {
    zpt::JSONIterator _to_return = (*this);
    ++(*this);
    return _to_return;
}

auto zpt::JSONIterator::operator->() -> pointer {
    switch (this->__target->type()) {
        case zpt::JSObject: {
            return std::make_tuple(
              this->__index, this->__iterator->first, this->__iterator->second);
        }
        case zpt::JSArray: {
            return std::make_tuple(this->__index, "", (**this->__target->array())[this->__index]);
        }
        default: {
            break;
        }
    }
    return std::make_tuple(this->__index, "", this->__target);
}

auto zpt::JSONIterator::operator==(JSONIterator _rhs) const -> bool {
    return this->__index == _rhs.__index &&
           (this->__target->type() != zpt::JSObject || this->__iterator == _rhs.__iterator);
}

auto zpt::JSONIterator::operator!=(JSONIterator _rhs) const -> bool { return !((*this) == _rhs); }

auto zpt::JSONIterator::operator--() -> JSONIterator& {
    --this->__index;
    if (this->__target->type() == zpt::JSObject) { --this->__iterator; }
    return (*this);
}

auto zpt::JSONIterator::operator--(int) -> JSONIterator {
    zpt::JSONIterator _to_return = (*this);
    --(*this);
    return _to_return;
}

auto zpt::get(std::string const& _path, zpt::json _source) -> zpt::json {
    return _source->get_path(_path);
}

auto zpt::timestamp(std::string const& _json_date) -> zpt::timestamp_t {
    if (_json_date.length() == 0) {
        return (zpt::timestamp_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
          .count();
    }
    time_t _n{ 0 };
    auto _ms{ 0 };
    std::string _s{ _json_date.data() };
    auto _idx = _s.rfind(".");
    std::string _mss;
    if (_idx != std::string::npos) {
        auto _prev_is_zero = true;
        if (_s[_idx + 1] != '0') {
            _mss.push_back(_s[_idx + 1]);
            _prev_is_zero = false;
        }
        if (!_prev_is_zero || _s[_idx + 2] != '0') { _mss.push_back(_s[_idx + 2]); }
        _mss.push_back(_s[_idx + 3]);
        _s.erase(_idx, 4);
        if (_s.length() < 20) { zpt::fromstr(_s, &_n, "%Y-%m-%dT%H:%M:%S", true); }
        else if (_s[_idx] == '+' || _s[_idx] == '-') {
            zpt::fromstr(_s, &_n, "%Y-%m-%dT%H:%M:%S%z");
        }
        else { zpt::fromstr(_s, &_n, "%Y-%m-%dT%H:%M:%S%Z"); }
        zpt::fromstr(_mss, &_ms);
    }
    return _n * 1000 + _ms;
}

auto zpt::timestamp(zpt::json _json_date) -> zpt::timestamp_t {
    return (zpt::timestamp_t)_json_date;
}

auto zpt::timestamp(zpt::timestamp_t _timestamp) -> std::string {
    return zpt::timestamp_to_str(_timestamp);
}

auto operator"" _JSON(const char* _string, size_t _length) -> zpt::json {
    zpt::json _to_return;
    _to_return.load_from(std::string{ _string, _length });
    return _to_return;
}

auto std::hash<zpt::json>::operator()(zpt::json const& _json) const noexcept -> std::size_t {
    return _json->hash();
}
