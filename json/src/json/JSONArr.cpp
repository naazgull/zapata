#include <cstdarg>
#include <ostream>
#include <regex>
#include <zapata/json/JSONClass.h>

/*JSON ARRAY*/
zpt::JSONArrT::JSONArrT() {}

zpt::JSONArrT::~JSONArrT() {}

auto
zpt::JSONArrT::push(zpt::JSONElementT& _value) -> JSONArrT& {
    this->__underlying.push_back(zpt::json{ _value });
    return (*this);
}

auto
zpt::JSONArrT::push(std::unique_ptr<zpt::JSONElementT> _value) -> JSONArrT& {
    this->__underlying.push_back(zpt::json{ _value.release() });
    return (*this);
}

auto
zpt::JSONArrT::push(zpt::json& _value) -> JSONArrT& {
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
zpt::JSONArr::operator<<(zpt::JSONElementT& _in) {
    (*this)->push(_in);
    return *this;
}
