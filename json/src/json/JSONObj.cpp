#include <cstdarg>
#include <ostream>
#include <regex>
#include <zapata/json/JSONClass.h>

zpt::JSONObjT::JSONObjT() {}

zpt::JSONObjT::~JSONObjT() {}

auto
zpt::JSONObjT::push(std::string _name) -> JSONObjT& {
    if (this->__name.length() == 0) {
        this->__name_to_index.push_back(_name);
        this->__name.assign(_name.data());
    }
    else {
        (**this).insert(std::make_pair(
          this->__name,
          std::make_tuple(
            zpt::json{ std::make_unique<zpt::JSONElementT>(std::string(_name.data())) },
            this->__name_to_index.size() - 1)));
        this->__name.clear();
    }
    return (*this);
}

auto
zpt::JSONObjT::push(JSONElementT& _value) -> JSONObjT& {
    assertz(this->__name.length() != 0, "you must pass a field name first", 500, 0);
    (**this).insert(
      std::make_pair(this->__name,
                     std::make_tuple(zpt::json{ std::make_unique<zpt::JSONElementT>(_value) },
                                     this->__name_to_index.size() - 1)));
    this->__name.clear();
    return (*this);
}

auto
zpt::JSONObjT::push(std::unique_ptr<zpt::JSONElementT> _value) -> JSONObjT& {
    assertz(this->__name.length() != 0, "you must pass a field name first", 500, 0);
    (**this).insert(std::make_pair(
      this->__name,
      std::make_tuple(zpt::json{ std::move(_value) }, this->__name_to_index.size() - 1)));
    this->__name.clear();
    return (*this);
}

auto
zpt::JSONObjT::push(zpt::json& _value) -> JSONObjT& {
    assertz(this->__name.length() != 0, "you must pass a field name first", 500, 0);
    (**this).insert(std::make_pair(std::string(this->__name.data()),
                                   std::make_tuple(_value, this->__name_to_index.size() - 1)));
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
    return this->operator[](this->__name_to_index[_idx]);
}

auto zpt::JSONObjT::operator[](size_t _idx) -> zpt::json& {
    assertz(this->__name_to_index.size() < _idx, "no such index", 500, 0);
    return this->operator[](this->__name_to_index[_idx]);
}

auto zpt::JSONObjT::operator[](const char* _idx) -> zpt::json& {
    return this->operator[](std::string(_idx));
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
