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

zpt::JSONObjT::JSONObjT() {}

zpt::JSONObjT::~JSONObjT() {}

auto
zpt::JSONObjT::push(std::string const& _name) -> JSONObjT& {
    if (this->__name.length() == 0) { this->__name.assign(_name.data()); }
    else {
        auto [_it, _inserted] = this->__underlying.insert(std::make_pair(this->__name, zpt::json(_name)));
        if (!_inserted) { _it->second = _name; }
        this->__name.clear();
    }
    return (*this);
}

auto
zpt::JSONObjT::push(std::unique_ptr<zpt::JSONElementT> _value) -> JSONObjT& {
    expect(this->__name.length() != 0, "you must pass a field name first");
    zpt::json _ref{ std::move(_value) };
    auto [_it, _inserted] = this->__underlying.insert(std::make_pair(this->__name, _ref));
    if (!_inserted) { _it->second = _ref; }
    this->__name.clear();
    return (*this);
}

auto
zpt::JSONObjT::push(zpt::json const& _value) -> JSONObjT& {
    expect(this->__name.length() != 0, "you must pass a field name first");
    auto [_it, _inserted] = this->__underlying.insert(std::make_pair(this->__name, _value));
    if (!_inserted) { _it->second = _value; }
    this->__name.clear();
    return (*this);
}

auto
zpt::JSONObjT::pop(int _idx) -> JSONObjT& {
    return this->pop(static_cast<size_t>(_idx));
}

auto
zpt::JSONObjT::pop(size_t _idx) -> JSONObjT& {
    expect(this->__underlying.size() < _idx, "no such index");
    try {
        this->pop(this->key_for(_idx));
    }
    catch (zpt::failed_expectation const& e) {
    }
    return (*this);
}

auto
zpt::JSONObjT::pop(const char* _name) -> JSONObjT& {
    return this->pop(std::string(_name));
}

auto
zpt::JSONObjT::pop(std::string const& _name) -> JSONObjT& {
    auto _found = this->__underlying.find(_name);
    if (_found != this->__underlying.end()) { this->__underlying.erase(_found); }
    return (*this);
}

auto
zpt::JSONObjT::key_for(size_t _idx) const -> std::string {
    expect(this->__underlying.size() > _idx, "no such index");
    std::string _name{ "" };
    size_t _pos{ 0 };
    for (auto _element : this->__underlying) {
        if (_pos == _idx) {
            _name.assign(_element.first);
            break;
        }
        ++_pos;
    }
    return _name;
}

auto
zpt::JSONObjT::get_path(std::string const& _path, std::string const& _separator) -> zpt::json {
    std::istringstream _iss;
    std::string _part;
    std::string _remainder;

    _iss.str(_path);
    std::getline(_iss, _part, _separator[0]);
    std::getline(_iss, _remainder);
    zpt::trim(_remainder);
    auto _current = (*this)[_part];
    if (!_current->ok()) {
        if (_part == "*" && _remainder.length() != 0) {
            for (auto _a : this->__underlying) {
                _current = _a.second->get_path(_remainder, _separator);
                if (_current->ok()) { return _current; }
            }
        }
        return zpt::undefined;
    }

    if (_remainder.length() == 0) { return _current; }
    return _current->get_path(_remainder, _separator);
}

auto
zpt::JSONObjT::set_path(std::string const& _path, zpt::json _value, std::string const& _separator) -> JSONObjT& {
    std::istringstream _iss(_path);
    std::string _part;

    getline(_iss, _part, _separator[0]);
    auto _current = (*this)[_part];
    if (!_current->ok()) {
        if (_iss.good()) {
            _current = zpt::json::object();
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
        if (_iss.good()) { _current->set_path(_path.substr(_part.length() + 1), _value, _separator); }
        else {
            this->push(_part);
            this->push(_value);
        }
    }
    return (*this);
}

auto
zpt::JSONObjT::del_path(std::string const& _path, std::string const& _separator) -> JSONObjT& {
    std::istringstream _iss(_path);
    std::string _part;

    getline(_iss, _part, _separator[0]);
    auto _current = (*this)[_part];
    if (!_current->ok()) { return (*this); }

    while (_iss.good()) {
        getline(_iss, _part, _separator[0]);
        if (_current[_part]->ok()) {
            if (_iss.good()) { _current = _current[_part]; }
            else { _current->object()->pop(_part); }
        }
        else { return (*this); }
    }
    return (*this);
}

auto
zpt::JSONObjT::clone() const -> zpt::json {
    zpt::json _return = zpt::json::object();
    for (auto _f : this->__underlying) { _return << _f.first << _f.second->clone(); }
    return _return;
}

auto
zpt::JSONObjT::operator->() -> zpt::json::map* {
    return &this->__underlying;
}

auto
zpt::JSONObjT::operator*() -> zpt::json::map& {
    return this->__underlying;
}

auto
zpt::JSONObjT::operator->() const -> zpt::json::map const* {
    return &this->__underlying;
}

auto
zpt::JSONObjT::operator*() const -> zpt::json::map const& {
    return this->__underlying;
}

auto
zpt::JSONObjT::operator==(zpt::JSONObjT const& _rhs) const -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) { return false; }
        if (_found->second == _f.second) { continue; }
        return false;
    }
    return true;
}

auto
zpt::JSONObjT::operator==(zpt::JSONObj const& _rhs) const -> bool {
    return *this == *_rhs;
}

auto
zpt::JSONObjT::operator!=(JSONObjT const& _rhs) const -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) { return true; }
        if (_found->second != _f.second) { return true; }
    }
    return false;
}

auto
zpt::JSONObjT::operator!=(zpt::JSONObj const& _rhs) const -> bool {
    return *this != *_rhs;
}

auto
zpt::JSONObjT::operator<(zpt::JSONObjT const& _rhs) const -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) { return false; }
        if (_found->second < _f.second) { continue; }
        return false;
    }
    return true;
}

auto
zpt::JSONObjT::operator<(zpt::JSONObj const& _rhs) const -> bool {
    return *this < *_rhs;
}

auto
zpt::JSONObjT::operator>(zpt::JSONObjT const& _rhs) const -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) { return false; }
        if (_found->second > _f.second) { continue; }
        return false;
    }
    return true;
}

auto
zpt::JSONObjT::operator>(zpt::JSONObj const& _rhs) const -> bool {
    return *this > *_rhs;
}

auto
zpt::JSONObjT::operator<=(zpt::JSONObjT const& _rhs) const -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) { return false; }
        if (_found->second <= _f.second) { continue; }
        return false;
    }
    return true;
}

auto
zpt::JSONObjT::operator<=(zpt::JSONObj const& _rhs) const -> bool {
    return *this <= *_rhs;
}

auto
zpt::JSONObjT::operator>=(zpt::JSONObjT const& _rhs) const -> bool {
    for (auto _f : this->__underlying) {
        auto _found = _rhs.__underlying.find(_f.first);
        if (_found == _rhs.__underlying.end()) { return false; }
        if (_found->second >= _f.second) { continue; }
        return false;
    }
    return true;
}

auto
zpt::JSONObjT::operator>=(zpt::JSONObj const& _rhs) const -> bool {
    return *this >= *_rhs;
}

auto
zpt::JSONObjT::operator[](int _idx) -> zpt::json& {
    expect(this->__underlying.size() < static_cast<size_t>(_idx), "no such index");
    return this->operator[](this->key_for(_idx));
}

auto
zpt::JSONObjT::operator[](size_t _idx) -> zpt::json& {
    expect(this->__underlying.size() < _idx, "no such index");
    return this->operator[](this->key_for(_idx));
}

auto
zpt::JSONObjT::operator[](const char* _idx) -> zpt::json& {
    return this->operator[](std::string(_idx));
}

auto
zpt::JSONObjT::operator[](std::string const& _idx) -> zpt::json& {
    return this->__underlying[_idx];
}

auto
zpt::JSONObjT::operator[](int _idx) const -> zpt::json const {
    expect(this->__underlying.size() < static_cast<size_t>(_idx), "no such index");
    return this->operator[](this->key_for(_idx));
}

auto
zpt::JSONObjT::operator[](size_t _idx) const -> zpt::json const {
    expect(this->__underlying.size() < _idx, "no such index");
    return this->operator[](this->key_for(_idx));
}

auto
zpt::JSONObjT::operator[](const char* _idx) const -> zpt::json const {
    return this->operator[](std::string(_idx));
}

auto
zpt::JSONObjT::operator[](std::string const& _idx) const -> zpt::json const {
    auto _found = this->__underlying.find(_idx);
    if (_found != this->__underlying.end()) { return _found->second; }
    return zpt::undefined;
}

auto
zpt::JSONObjT::stringify(std::string& _out) -> zpt::JSONObjT& {
    static_cast<zpt::JSONObjT const&>(*this).stringify(_out);
    return (*this);
}

auto
zpt::JSONObjT::stringify(std::ostream& _out) -> zpt::JSONObjT& {
    static_cast<zpt::JSONObjT const&>(*this).stringify(_out);
    return (*this);
}

auto
zpt::JSONObjT::stringify(std::string& _out) const -> zpt::JSONObjT const& {
    _out.insert(_out.length(), "{");
    auto _first = true;
    for (auto _element : this->__underlying) {
        if (!_first) { _out.insert(_out.length(), ","); }
        _first = false;
        _out.insert(_out.length(), "\"");
        _out.insert(_out.length(), _element.first);
        _out.insert(_out.length(), "\":");
        _element.second->stringify(_out);
    }
    _out.insert(_out.length(), "}");
    return (*this);
}

auto
zpt::JSONObjT::stringify(std::ostream& _out) const -> zpt::JSONObjT const& {
    _out << "{" << std::flush;
    auto _first = true;
    for (auto _element : this->__underlying) {
        if (!_first) { _out << ","; }
        _first = false;
        _out << "\"" << _element.first << "\":" << std::flush;
        _element.second->stringify(_out);
    }
    _out << "}" << std::flush;
    return (*this);
}
auto
zpt::JSONObjT::prettify(std::string& _out, uint _n_tabs) -> zpt::JSONObjT& {
    static_cast<zpt::JSONObjT const&>(*this).prettify(_out, _n_tabs);
    return (*this);
}

auto
zpt::JSONObjT::prettify(std::ostream& _out, uint _n_tabs) -> zpt::JSONObjT& {
    static_cast<zpt::JSONObjT const&>(*this).prettify(_out, _n_tabs);
    return (*this);
}

auto
zpt::JSONObjT::prettify(std::string& _out, uint _n_tabs) const -> zpt::JSONObjT const& {
    _out.insert(_out.length(), "{");
    auto _first = true;
    for (auto _element : this->__underlying) {
        if (!_first) { _out.insert(_out.length(), ","); }
        _out.insert(_out.length(), "\n");
        _first = false;
        _out.insert(_out.length(), std::string(_n_tabs + 1, '\t'));
        _out.insert(_out.length(), "\"");
        _out.insert(_out.length(), _element.first);
        _out.insert(_out.length(), "\" : ");
        _element.second->prettify(_out, _n_tabs + 1);
    }
    if (!_first) {
        _out.insert(_out.length(), "\n");
        _out.insert(_out.length(), std::string(_n_tabs, '\t'));
    }
    _out.insert(_out.length(), "}");
    return (*this);
}

auto
zpt::JSONObjT::prettify(std::ostream& _out, uint _n_tabs) const -> zpt::JSONObjT const& {
    _out << "{" << std::flush;
    auto _first = true;
    for (auto _element : this->__underlying) {
        if (!_first) { _out << ","; }
        _out << "\n ";
        _first = false;
        _out << std::string(_n_tabs + 1, '\t') << "\"" << _element.first << "\" : " << std::flush;
        _element.second->prettify(_out, _n_tabs + 1);
    }
    if (!_first) { _out << "\n" << std::string(_n_tabs, '\t') << std::flush; }
    _out << "}" << std::flush;
    return (*this);
}

/*JSON POINTER TO OBJECT*/
zpt::JSONObj::JSONObj()
  : __underlying{ std::make_shared<JSONObjT>() } {}

zpt::JSONObj::JSONObj(const JSONObj& _rhs) { (*this) = _rhs; }

zpt::JSONObj::JSONObj(JSONObj&& _rhs) { (*this) = _rhs; }

zpt::JSONObj::JSONObj(JSONObjT* _target)
  : __underlying{ _target } {}

zpt::JSONObj::~JSONObj() {}

auto
zpt::JSONObj::hash() const -> size_t {
    return reinterpret_cast<size_t>(this->__underlying.get());
}

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

auto
zpt::JSONObj::operator->() -> zpt::JSONObjT* {
    return this->__underlying.get();
}

auto
zpt::JSONObj::operator*() -> zpt::JSONObjT& {
    return *this->__underlying.get();
}

auto
zpt::JSONObj::operator->() const -> zpt::JSONObjT const* {
    return this->__underlying.get();
}

auto
zpt::JSONObj::operator*() const -> zpt::JSONObjT const& {
    return *this->__underlying.get();
}

zpt::JSONObj::operator std::string() {
    if (this->__underlying.get() == nullptr) { return ""; }
    std::string _out;
    (*this)->stringify(_out);
    return _out;
}

zpt::JSONObj::operator zpt::pretty() {
    if (this->__underlying.get() == nullptr) { return ""; }
    std::string _out;
    (*this)->prettify(_out);
    return _out;
}

auto
zpt::JSONObj::operator<<(std::string const& _in) -> zpt::JSONObj& {
    (*this)->push(_in);
    return *this;
}

auto
zpt::JSONObj::operator<<(const char* _in) -> zpt::JSONObj& {
    (*this)->push(std::string{ _in });
    return *this;
}

auto
zpt::JSONObj::operator<<(std::initializer_list<zpt::json> _list) -> zpt::JSONObj& {
    (*this)->push(zpt::json{ _list });
    return *this;
}
