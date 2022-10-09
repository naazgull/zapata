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

/*JSON ARRAY*/
zpt::JSONArrT::JSONArrT() {}

zpt::JSONArrT::~JSONArrT() {}

auto
zpt::JSONArrT::push(std::unique_ptr<zpt::JSONElementT> _value) -> JSONArrT& {
    this->__underlying.push_back(zpt::json{ std::move(_value) });
    return (*this);
}

auto
zpt::JSONArrT::push(zpt::json const& _value) -> JSONArrT& {
    this->__underlying.push_back(_value);
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
zpt::JSONArrT::pop(std::string const& _idx) -> JSONArrT& {
    long _i = -1;
    zpt::fromstr(_idx, &_i);
    if (_i > 0) {
        expect((size_t)_i < this->__underlying.size(),
               "the index of the element you want to "
               "remove must be lower than the array "
               "size");
        this->__underlying.erase(this->__underlying.begin() + _i);
    }
    return (*this);
}

auto
zpt::JSONArrT::pop(size_t _idx) -> JSONArrT& {
    expect(_idx >= 0, "the index of the element you want to remove must be higher then 0");
    expect(_idx < this->__underlying.size(),
           "the index of the element you want to remove "
           "must be lower than the array size");
    this->__underlying.erase(this->__underlying.begin() + _idx);
    return (*this);
}

auto
zpt::JSONArrT::sort() -> JSONArrT& {
    std::sort(this->__underlying.begin(), this->__underlying.end(), [](zpt::json _lhs, zpt::json _rhs) -> bool {
        return _lhs < _rhs;
    });
    return (*this);
}

auto
zpt::JSONArrT::sort(std::function<bool(zpt::json, zpt::json)> _comparator) -> JSONArrT& {
    std::sort(this->__underlying.begin(), this->__underlying.end(), _comparator);
    return (*this);
}

auto
zpt::JSONArrT::get_path(std::string const& _path, std::string const& _separator) -> zpt::json {
    std::istringstream _iss(_path);
    std::string _part;
    std::string _remainder;

    getline(_iss, _part, _separator[0]);
    getline(_iss, _remainder);
    zpt::trim(_remainder);
    auto _current = (*this)[_part];
    if (!_current->ok()) {
        if (_part == "*" && _remainder.length() != 0) {
            auto _return = zpt::json::array();
            for (auto _a : this->__underlying) {
                _current = _a->get_path(_remainder, _separator);
                if (_current->ok()) { _return << _current; }
            }
            return _return;
        }
        return zpt::undefined;
    }

    if (_remainder.length() == 0) { return _current; }
    return _current->get_path(_remainder, _separator);
}

auto
zpt::JSONArrT::set_path(std::string const& _path, zpt::json _value, std::string const& _separator) -> JSONArrT& {
    std::istringstream _iss(_path);
    std::string _part;

    getline(_iss, _part, _separator[0]);
    auto _current = (*this)[_part];
    if (!_current->ok()) {
        if (_iss.good()) {
            _current = zpt::json::object();
            this->__underlying.push_back(_current);
            _current->set_path(_path.substr(_part.length() + 1), _value, _separator);
        }
        else { this->__underlying.push_back(_value); }
    }
    else {
        if (_iss.good()) { _current->set_path(_path.substr(_part.length() + 1), _value, _separator); }
        else {
            this->pop(_part);
            (*this)[std::stoi(_part)] = _value;
        }
    }
    return (*this);
}

auto
zpt::JSONArrT::del_path(std::string const& _path, std::string const& _separator) -> JSONArrT& {
    std::istringstream _iss(_path);
    std::string _part;

    getline(_iss, _part, _separator[0]);
    auto _current = (*this)[_part];
    if (!_current->ok()) { return (*this); }

    while (_iss.good()) {
        getline(_iss, _part, _separator[0]);
        if (_current[_part]->ok()) {
            if (_iss.good()) { _current = _current[_part]; }
            else { _current->array()->pop(_part); }
        }
        else { return (*this); }
    }
    return (*this);
}

auto
zpt::JSONArrT::clone() const -> zpt::json {
    auto _return = zpt::json::array();
    for (auto _f : this->__underlying) { _return << _f->clone(); }
    return _return;
}

auto
zpt::JSONArrT::operator->() -> std::vector<zpt::json>* {
    return &this->__underlying;
}

auto
zpt::JSONArrT::operator*() -> std::vector<zpt::json>& {
    return this->__underlying;
}

auto
zpt::JSONArrT::operator->() const -> std::vector<zpt::json> const* {
    return &this->__underlying;
}

auto
zpt::JSONArrT::operator*() const -> std::vector<zpt::json> const& {
    return this->__underlying;
}

auto
zpt::JSONArrT::operator==(zpt::JSONArrT const& _rhs) const -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] == _rhs[_f]) { continue; }
        return false;
    }
    return true;
}

auto
zpt::JSONArrT::operator==(zpt::JSONArr const& _rhs) const -> bool {
    return *this == *_rhs;
}

auto
zpt::JSONArrT::operator!=(JSONArrT const& _rhs) const -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] != _rhs[_f]) { return true; }
    }
    return false;
}

auto
zpt::JSONArrT::operator!=(zpt::JSONArr const& _rhs) const -> bool {
    return *this != *_rhs;
}

auto
zpt::JSONArrT::operator<(zpt::JSONArrT const& _rhs) const -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] < _rhs[_f]) { continue; }
        return false;
    }
    return true;
}

auto
zpt::JSONArrT::operator<(zpt::JSONArr const& _rhs) const -> bool {
    return *this < *_rhs;
}

auto
zpt::JSONArrT::operator>(zpt::JSONArrT const& _rhs) const -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] > _rhs[_f]) { continue; }
        return false;
    }
    return true;
}

auto
zpt::JSONArrT::operator>(zpt::JSONArr const& _rhs) const -> bool {
    return *this > *_rhs;
}

auto
zpt::JSONArrT::operator<=(zpt::JSONArrT const& _rhs) const -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] <= _rhs[_f]) { continue; }
        return false;
    }
    return true;
}

auto
zpt::JSONArrT::operator<=(zpt::JSONArr const& _rhs) const -> bool {
    return *this <= *_rhs;
}

auto
zpt::JSONArrT::operator>=(zpt::JSONArrT const& _rhs) const -> bool {
    for (size_t _f = 0; _f != this->__underlying.size(); _f++) {
        if ((*this)[_f] >= _rhs[_f]) { continue; }
        return false;
    }
    return true;
}

auto
zpt::JSONArrT::operator>=(zpt::JSONArr const& _rhs) const -> bool {
    return *this >= *_rhs;
}

auto
zpt::JSONArrT::operator[](int _idx) -> zpt::json& {
    return (*this)[(size_t)_idx];
}

auto
zpt::JSONArrT::operator[](size_t _idx) -> zpt::json& {
    if (_idx >= this->__underlying.size()) { this->__underlying.resize(_idx + 1); }
    return this->__underlying.at(_idx);
}

auto
zpt::JSONArrT::operator[](const char* _idx) -> zpt::json& {
    return (*this)[std::string(_idx)];
}

auto
zpt::JSONArrT::operator[](std::string const& _idx) -> zpt::json& {
    long _i = -1;
    zpt::fromstr(_idx, &_i);
    expect(_i != -1, "illegal index value");
    return (*this)[(size_t)_i];
}

auto
zpt::JSONArrT::operator[](int _idx) const -> zpt::json const {
    return (*this)[(size_t)_idx];
}

auto
zpt::JSONArrT::operator[](size_t _idx) const -> zpt::json const {
    if (_idx >= this->__underlying.size()) { return zpt::undefined; }
    return this->__underlying.at(_idx);
}

auto
zpt::JSONArrT::operator[](const char* _idx) const -> zpt::json const {
    return (*this)[std::string(_idx)];
}

auto
zpt::JSONArrT::operator[](std::string const& _idx) const -> zpt::json const {
    long _i = -1;
    zpt::fromstr(_idx, &_i);
    expect(_i != -1, "illegal index value");
    return (*this)[(size_t)_i];
}

auto
zpt::JSONArrT::stringify(std::string& _out) -> zpt::JSONArrT& {
    static_cast<zpt::JSONArrT const&>(*this).stringify(_out);
    return (*this);
}

auto
zpt::JSONArrT::stringify(std::ostream& _out) -> zpt::JSONArrT& {
    static_cast<zpt::JSONArrT const&>(*this).stringify(_out);
    return (*this);
}

auto
zpt::JSONArrT::stringify(std::string& _out) const -> zpt::JSONArrT const& {
    _out.insert(_out.length(), "[");
    auto _first = true;
    for (auto _i : this->__underlying) {
        if (!_first) { _out.insert(_out.length(), ","); }
        _first = false;
        _i->stringify(_out);
    }
    _out.insert(_out.length(), "]");
    return (*this);
}

auto
zpt::JSONArrT::stringify(std::ostream& _out) const -> zpt::JSONArrT const& {
    _out << "[" << std::flush;
    auto _first = true;
    for (auto _i : this->__underlying) {
        if (!_first) { _out << ","; }
        _first = false;
        _i->stringify(_out);
    }
    _out << "]" << std::flush;
    return (*this);
}

auto
zpt::JSONArrT::prettify(std::string& _out, uint _n_tabs) -> JSONArrT& {
    static_cast<zpt::JSONArrT const&>(*this).prettify(_out, _n_tabs);
    return (*this);
}

auto
zpt::JSONArrT::prettify(std::ostream& _out, uint _n_tabs) -> JSONArrT& {
    static_cast<zpt::JSONArrT const&>(*this).prettify(_out, _n_tabs);
    return (*this);
}

auto
zpt::JSONArrT::prettify(std::string& _out, uint _n_tabs) const -> JSONArrT const& {
    _out.insert(_out.length(), "[");
    auto _first = true;
    for (auto _i : this->__underlying) {
        if (!_first) { _out.insert(_out.length(), ","); }
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
zpt::JSONArrT::prettify(std::ostream& _out, uint _n_tabs) const -> JSONArrT const& {
    _out << "[" << std::flush;
    auto _first = true;
    for (auto _i : this->__underlying) {
        if (!_first) { _out << ","; }
        _out << "\n ";
        _first = false;
        _out << std::string(_n_tabs + 1, '\t') << std::flush;
        _i->prettify(_out, _n_tabs + 1);
    }
    if (!_first) { _out << "\n" << std::string(_n_tabs, '\t'); }
    _out << "]" << std::flush;
    return (*this);
}

/*JSON POINTER TO ARRAY*/
zpt::JSONArr::JSONArr()
  : __underlying{ std::make_shared<zpt::JSONArrT>() } {}

zpt::JSONArr::JSONArr(const JSONArr& _rhs) { (*this) = _rhs; }

zpt::JSONArr::JSONArr(JSONArr&& _rhs) { (*this) = _rhs; }

zpt::JSONArr::JSONArr(zpt::JSONArrT* _target)
  : __underlying{ _target } {}

zpt::JSONArr::~JSONArr() {}

auto
zpt::JSONArr::hash() const -> size_t {
    return reinterpret_cast<size_t>(this->__underlying.get());
}

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

auto
zpt::JSONArr::operator->() -> zpt::JSONArrT* {
    return this->__underlying.get();
}

auto
zpt::JSONArr::operator*() -> zpt::JSONArrT& {
    return *this->__underlying.get();
}

auto
zpt::JSONArr::operator->() const -> zpt::JSONArrT const* {
    return this->__underlying.get();
}

auto
zpt::JSONArr::operator*() const -> zpt::JSONArrT const& {
    return *this->__underlying.get();
}

zpt::JSONArr::operator std::string() {
    if (this->__underlying.get() == nullptr) { return ""; }
    std::string _out;
    (*this)->stringify(_out);
    return _out;
}

zpt::JSONArr::operator zpt::pretty() {
    if (this->__underlying.get() == nullptr) { return ""; }
    std::string _out;
    (*this)->prettify(_out);
    return _out;
}

auto
zpt::JSONArr::operator<<(std::initializer_list<zpt::json> _list) -> zpt::JSONArr& {
    (*this)->push(zpt::json{ _list });
    return *this;
}
