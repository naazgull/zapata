#include <regex>
#include <zapata/exceptions/SyntaxErrorException.h>
#include <zapata/file/manip.h>
#include <zapata/json/json.h>
#include <zapata/log/log.h>

auto
zpt::to_string(zpt::json _in) -> std::string {
    return static_cast<std::string>(_in);
}

auto
zpt::split(std::string _to_split, std::string _separator, bool _trim) -> zpt::json {
    zpt::json _ret = zpt::json::array();
    if (_to_split.length() == 0 || _separator.length() == 0) {
        return _ret;
    }
    std::istringstream _iss(_to_split);
    std::string _part;
    while (_iss.good()) {
        std::getline(_iss, _part, _separator[0]);
        if (_part.length() != 0) {
            if (_trim) {
                zpt::trim(_part);
            }
            _ret << _part;
        }
    }
    return _ret;
}

auto
zpt::join(zpt::json _to_join, std::string _separator) -> std::string {
    expect(_to_join->type() == zpt::JSArray || _to_join->type() == zpt::JSObject,
            "JSON to join must be an array",
            412,
            0);
    std::string _return{ "" };
    for (auto [_idx, _key, _value] : _to_join) {
        if (_return.length() != 0) {
            _return += _separator;
        }
        if (_to_join->type() == zpt::JSArray) {
            _return += static_cast<std::string>(_value);
        }
        else if (_to_join->type() == zpt::JSObject) {
            _return += _key + _separator + static_cast<std::string>(_value);
        }
    }
    return _return;
}

auto
zpt::path::split(std::string _to_split) -> zpt::json {
    return zpt::split(_to_split, "/", true);
}

auto
zpt::path::join(zpt::json _to_join) -> std::string {
    return std::string("/") + zpt::join(_to_join, "/");
}

auto
zpt::conf::getopt(int _argc, char* _argv[]) -> zpt::json {
    zpt::json _non_positional = zpt::json::array();
    zpt::json _return = { "--", _non_positional };
    std::string _last("");
    for (int _i = 1; _i != _argc; _i++) {
        std::string _arg(_argv[_i]);
        if (_arg.find("--enable") == 0 || _arg.find("--disable") == 0 ||
            _arg.find("--force") == 0) {
            if (_last.length() != 0) {
                if (!_return[_last]->ok()) {
                    _return << _last << zpt::json::array();
                }
                _return[_last] << true;
            }
            _arg.erase(0, 2);
            if (!_return[_arg]->ok()) {
                _return << _arg << zpt::json::array();
            }
            _return[_arg] << true;
            _last.assign("");
        }
        else if (_arg.find("-enable") == 0 || _arg.find("-disable") == 0 ||
                 _arg.find("-force") == 0) {
            if (_last.length() != 0) {
                if (!_return[_last]->ok()) {
                    _return << _last << zpt::json::array();
                }
                _return[_last] << true;
            }
            _arg.erase(0, 1);
            if (!_return[_arg]->ok()) {
                _return << _arg << zpt::json::array();
            }
            _return[_arg] << true;
            _last.assign("");
        }
        else if (_arg.find("--") == 0) {
            if (_last.length() != 0) {
                if (!_return[_last]->ok()) {
                    _return << _last << zpt::json::array();
                }
                _return[_last] << true;
            }
            _arg.erase(0, 2);
            _last.assign(_arg);
        }
        else if (_arg.find("-") == 0) {
            if (_last.length() != 0) {
                if (!_return[_last]->ok()) {
                    _return << _last << zpt::json::array();
                }
                _return[_last] << true;
            }
            _arg.erase(0, 1);
            _last.assign(_arg);
        }
        else {
            if (_last.length() != 0) {
                if (!_return[_last]->ok()) {
                    _return << _last << zpt::json::array();
                }
                _return[_last] << _arg;
                _last.assign("");
            }
            else {
                _non_positional << _arg;
            }
        }
    }
    if (_last.length() != 0) {
        if (!_return[_last]->ok()) {
            _return << _last << zpt::json::array();
        }
        _return[_last] << true;
    }
    return _return;
}

auto
zpt::conf::setup(zpt::json _options) -> void {
    zpt::conf::dirs(_options);
    zpt::conf::env(_options);
}

auto
zpt::conf::dirs(std::string _dir, zpt::json _options) -> void {
    std::vector<std::string> _non_positional;
    if (zpt::is_dir(_dir)) {
        zpt::glob(_dir, _non_positional, "(.*)\\.conf");
    }
    else {
        _non_positional.push_back(_dir);
    }
    std::sort(_non_positional.begin(), _non_positional.end());
    for (auto _file : _non_positional) {
        zpt::json _conf;
        std::ifstream _ifs;
        _ifs.open(_file.data());
        try {
            _ifs >> _conf;
        }
        catch (zpt::SyntaxErrorException& _e) {
            _conf = zpt::undefined;
            expect(_conf->ok(),
                    std::string("syntax error parsing file: ") + _file + std::string(": ") +
                      _e.what(),
                    500,
                    0);
        }
        _ifs.close();

        for (auto [_idx, _key, _new_field] : _conf) {
            _options << _key << (_options[_key] + _new_field);
        }
    }
}

auto
zpt::conf::dirs(zpt::json _options) -> void {
    bool* _redo = new bool(false);
    do {
        *_redo = false;
        zpt::json _traversable = _options->clone();
        _traversable->inspect(
          { "$any", "type" },
          [&](std::string _object_path, std::string _key, zpt::JSONElementT& _parent) -> void {
              if (_key == "$include") {
                  zpt::json _object =
                    (_object_path.rfind(".") != std::string::npos
                       ? _options->get_path(_object_path.substr(0, _object_path.rfind(".")))
                       : _options);
                  zpt::json _to_include = _options->get_path(_object_path);
                  if (_to_include->is_array()) {
                      for (auto [_idx, _key, _file] : _to_include) {
                          zpt::conf::dirs((std::string)_file, _object);
                      }
                  }
                  else {
                      zpt::conf::dirs((std::string)_to_include, _object);
                  }
                  _object >> "$include";
                  *_redo = true;
              }
          });
    } while (*_redo);
    delete _redo;
}

auto
zpt::conf::env(zpt::json _options) -> void {
    zpt::json _traversable = _options->clone();
    _traversable->inspect(
      { "$regexp", "([\"])(.*)([$])([{])([^}]+)([}])(.*)([\"])" },
      [&](std::string _object_path, std::string _key, zpt::JSONElementT& _parent) -> void {
          std::string _value = std::string(_options->get_path(_object_path));
          std::string _found = std::string(_value.data());

          for (size_t _idx = _found.find("$"); _idx != std::string::npos;
               _idx = _found.find("$", _idx + 1)) {
              std::string _var = _found.substr(_idx + 2, _found.find("}", _idx) - _idx - 2);

              const char* _var_val = std::getenv(_var.data());
              if (_var_val != nullptr) {
                  zpt::replace(
                    _value, std::string("${") + _var + std::string("}"), zpt::r_trim(_var_val));
              }
          }
          _options->set_path(_object_path, zpt::json::string(_value));
      });
}

auto
zpt::email::parse(std::string _email) -> zpt::json {
    static const std::regex _email_rgx("(?:\"([^\"]*)\")?"
                                       "(?:[ ])?"
                                       "(?:<)?"
                                       "([a-zA-Z0-9])([a-zA-Z0-9+._-]*)@"
                                       "([a-zA-Z0-9])([a-zA-Z0-9+._-]*)"
                                       "(?:>)?");
    std::smatch _email_matches;
    std::regex_match(_email, _email_matches, _email_rgx);

    return {
        "full",
        ((std::string)_email_matches[0]),
        "name",
        (((std::string)_email_matches[1]).length() == 0
           ? zpt::undefined
           : zpt::json::string(((std::string)_email_matches[1]))),
        "user",
        (((std::string)_email_matches[2]) + ((std::string)_email_matches[3])),
        "domain",
        (((std::string)_email_matches[4]) + ((std::string)_email_matches[5])),
        "address",
        (((std::string)_email_matches[2]) + ((std::string)_email_matches[3]) + std::string("@") +
         ((std::string)_email_matches[4]) + ((std::string)_email_matches[5])),
    };
}

auto
zpt::parameters::parse(int _argc, char* _argv[], zpt::json _config) -> zpt::json {
    zpt::json _return = zpt::json::object();
    zpt::json _values = zpt::json::array();
    for (int _i = 1; _i != _argc; _i++) {
        std::string _arg(_argv[_i]);
        if (_arg.find("--") == 0 || _arg.find("-") == 0) {
            std::string _value;
            if (_arg.find("=") != std::string::npos) {
                zpt::json _split = zpt::split(_arg, "=");
                _arg.assign(static_cast<std::string>(_split[0]));
                _value.assign(static_cast<std::string>(_split[1]));
            }
            _values = _return[_arg];
            if (_values == zpt::undefined) {
                _return << _arg << (_value.length() != 0 ? zpt::json{ _value } : zpt::json{ true });
            }
            else if (_values->type() == zpt::JSArray) {
                _values << (_value.length() != 0 ? zpt::json{ _value } : zpt::json{ true });
            }
            else {
                zpt::json _old = _values;
                _values = zpt::json::array();
                _values << _old << (_value.length() != 0 ? zpt::json{ _value } : zpt::json{ true });
                _return << _arg << _values;
            }
        }
        else {
            _values = _return["--"];
            if (_values == zpt::undefined) {
                _values = zpt::json::array();
                _return << "--" << _values;
            }
            _values << _arg;
        }
    }

    for (auto [_, _key, _option] : _return) {
        if (_key == "--") {
            continue;
        }
        expect(_config[_key]->type() == zpt::JSArray,
                std::string{ "'" } + _key + std::string{ "' is not a valid option" },
                500,
                0);
        for (auto [_, __, _cfg_value] : _config[_key]) {
            if (_cfg_value == "single") {
                expect(_option->type() == zpt::JSString || _option->type() == zpt::JSBoolean,
                        std::string{ "'" } + _key +
                          std::string{ "' option can't have multiple values" },
                        500,
                        0);
            }
        }
    }

    for (auto [_, _key, _option] : _config) {
        for (auto [_, __, _cfg_value] : _option) {
            if (_cfg_value == "mandatory") {
                expect(_return[_key] != zpt::undefined,
                        std::string{ "'" } + _key + std::string{ "' option is mandatory" },
                        500,
                        0);
            }
        }
    }
    return _return;
}

auto
zpt::test::location(zpt::json _location) -> bool {
    return (_location->is_object() && _location["longitude"]->is_number() &&
            _location["latitude"]->is_number()) ||
           (_location->is_array() && _location->size() == 2 && _location[0]->is_number() &&
            _location[1]->is_number());
}

auto
zpt::test::timestamp(zpt::json _timestamp) -> bool {
    if (_timestamp->type() == zpt::JSDate) {
        return true;
    }
    if (_timestamp->type() != zpt::JSString) {
        return false;
    }
    static const std::regex _timestamp_rgx("^([0-9]{4})-"
                                           "([0-9]{2})-"
                                           "([0-9]{2})T"
                                           "([0-9]{2}):"
                                           "([0-9]{2}):"
                                           "([0-9]{2})."
                                           "([0-9]{3})([+-])"
                                           "([0-9]{4})$");
    return std::regex_match(_timestamp->str(), _timestamp_rgx);
}

auto
zpt::http::cookies::deserialize(std::string _cookie_header) -> zpt::json {
    zpt::json _splitted = zpt::split(_cookie_header, ";");
    zpt::json _return = zpt::json::object();
    bool _first = true;
    for (auto [_idx, _key, _part] : _splitted) {
        std::string _value = std::string(_part);
        zpt::trim(_value);
        if (_first) {
            _return << "value" << zpt::json::string(_value);
            _first = false;
        }
        else {
            zpt::json _pair = zpt::split(_value, "=");
            if (_pair->size() == 2) {
                _return << _pair[0]->str() << _pair[1];
            }
        }
    }
    return _return;
}

auto
zpt::http::cookies::serialize(zpt::json _info) -> std::string {
    std::string _return((std::string)_info["value"]);
    for (auto [_idx, _key, _field] : _info) {
        if (_key == "value") {
            continue;
        }
        _return += std::string("; ") + _key + std::string("=") + std::string(_field);
    }
    return _return;
}

extern "C" auto
zpt_lex_json() -> int {
    return 1;
}