/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
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
    inline csv(std::string _rhs)
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
split(std::string _to_split, std::string _separator, bool _trim = false) -> zpt::json;
auto
join(zpt::json _to_join, std::string _separator) -> std::string;

namespace path {
auto
split(std::string _to_split) -> zpt::json;
auto
join(zpt::json _to_join) -> std::string;
} // namespace path

namespace email {
auto
parse(std::string _email) -> zpt::json;
}

namespace uri {
auto
parse(std::string _uri) -> zpt::json;

namespace query {
auto
parse(std::string _query) -> zpt::json;
}
namespace authority {
auto
parse(std::string _authority) -> zpt::json;
}

auto
to_str(zpt::json _uri, zpt::json _opts = zpt::undefined) -> std::string;
} // namespace uri

namespace conf {
auto
getopt(int _argc, char* _argv[]) -> zpt::json;
auto
setup(zpt::json _options) -> void;
auto
dirs(std::string _dir, zpt::json _options) -> void;
auto
dirs(zpt::json _options) -> void;
auto
env(zpt::json _options) -> void;
} // namespace conf

namespace parameters {
auto
parse(int _argc, char* _argv[], zpt::json _config) -> zpt::json;
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
deserialize(std::string _cookie_header) -> zpt::json;
auto
serialize(zpt::json _info) -> std::string;
} // namespace cookies
} // namespace http
} // namespace zpt
