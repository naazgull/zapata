#pragma once

#include <string>
#include <unistd.h>
#include <zapata/json/JSONClass.h>
#include <zapata/json/JSONParser.h>

namespace zpt {

auto
to_string(zpt::json _in) -> std::string;

class csv : public std::string {
  public:
    inline csv()
      : std::string(){};
    inline csv(std::string const& _rhs)
      : std::string(_rhs){};
    inline csv(const char* _rhs)
      : std::string(_rhs){};
    friend auto operator<<(std::ostream& _out, zpt::csv& _in) -> std::ostream& {
        _out << std::string(_in.data());
        return _out;
    };
    friend auto operator>>(std::istream& _in, zpt::csv& _out) -> std::istream& {
        _out.clear();
        std::getline(_in, _out, '\n');
        zpt::trim(_out);
        return _in;
    };
    inline operator zpt::json() {
        zpt::json _result = zpt::json::array();
        std::istringstream _iss;
        _iss.str(*this);

        char _c = '\0';
        bool _commas = false;
        bool _escaped = false;
        std::string _value;
        do {
            _iss >> _c;
            switch (_c) {
                case '\0': {
                    break;
                }
                case '"': {
                    if (_escaped) {
                        _escaped = false;
                        _value.push_back(_c);
                    }
                    else {
                        _commas = !_commas;
                    }
                    break;
                }
                case '\\': {
                    _escaped = true;
                    break;
                }
                case ',': {
                    if (!_commas) {
                        _result << _value;
                        _value.clear();
                    }
                    else {
                        _value.push_back(_c);
                    }
                    break;
                }
                default: {
                    _value.push_back(_c);
                    break;
                }
            }
            _c = '\0';
        } while (_iss.good());
        _result << _value;

        return _result;
    };
};

auto
split(std::string const& _to_split, std::string const& _separator, bool _trim = false) -> zpt::json;
auto
join(zpt::json _to_join, std::string const& _separator) -> std::string;

namespace path {
auto
split(std::string const& _to_split) -> zpt::json;
auto
join(zpt::json _to_join) -> std::string;
} // namespace path

namespace email {
auto
parse(std::string const& _email) -> zpt::json;
}

auto
to_str(zpt::json _uri, zpt::json _opts = zpt::undefined) -> std::string;

namespace conf {
auto
getopt(int _argc, char* _argv[]) -> zpt::json;
auto
setup(zpt::json _options) -> void;
auto
evaluate_ref(zpt::json _options, std::string const& _parent_key, zpt::json _parent) -> void;
auto
file(std::string const& _file, zpt::json& _options) -> void;
auto
dirs(std::string const& _dir, zpt::json& _options) -> void;
auto
dirs(zpt::json& _options) -> void;
auto
env(zpt::json& _options) -> void;
} // namespace conf

namespace parameters {
auto
parse(int _argc, char* _argv[], zpt::json _config) -> zpt::json;
auto
verify(zpt::json _to_check, zpt::json _rules, bool _inclusive = false) -> void;
auto
usage(zpt::json _config) -> std::string;
} // namespace parameters

namespace test {
auto
location(zpt::json _location) -> bool;
auto
timestamp(zpt::json _timestamp) -> bool;
} // namespace test

namespace http {
namespace cookies {
auto
deserialize(std::string const& _cookie_header) -> zpt::json;
auto
serialize(zpt::json _info) -> std::string;
} // namespace cookies
} // namespace http
} // namespace zpt
