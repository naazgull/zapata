#include <regex>
#include <zapata/exceptions/SyntaxErrorException.h>
#include <zapata/file/manip.h>
#include <zapata/json/json.h>
#include <zapata/log/log.h>

auto zpt::to_string(zpt::JSONType _type) -> std::string {
    switch (_type) {
        case JSObject: {
            return "object";
        }
        case JSArray: {
            return "array";
        }
        case JSString: {
            return "string";
        }
        case JSInteger: {
            return "int";
        }
        case JSDouble: {
            return "double";
        }
        case JSBoolean: {
            return "bool";
        }
        case JSNil: {
            return "null";
        }
        case JSDate: {
            return "date-time";
        }
        case JSLambda: {
            return "lambda";
        }
        case JSRegex: {
            return "regexp";
        }
        case JSUndefined: {
            return "undefined";
        }
    }
    return "undefined";
}

auto zpt::to_string(zpt::json _in) -> std::string { return static_cast<std::string>(_in); }

auto zpt::split(std::string const& _to_split, std::string const& _separator, bool _trim)
  -> zpt::json {
    zpt::json _ret = zpt::json::array();
    if (_to_split.length() == 0 || _separator.length() == 0) { return _ret; }
    std::istringstream _iss(_to_split);
    std::string _part;
    while (_iss.good()) {
        std::getline(_iss, _part, _separator[0]);
        if (_part.length() != 0) {
            if (_trim) { zpt::trim(_part); }
            _ret << _part;
        }
    }
    return _ret;
}

auto zpt::join(zpt::json _to_join, std::string const& _separator) -> std::string {
    expect(_to_join->type() == zpt::JSArray || _to_join->type() == zpt::JSObject,
           "JSON to join must be an array");
    std::string _return{ "" };
    for (auto const& [_idx, _key, _value] : _to_join) {
        if (_return.length() != 0) { _return += _separator; }
        if (_to_join->type() == zpt::JSArray) { _return += static_cast<std::string>(_value); }
        else if (_to_join->type() == zpt::JSObject) {
            _return += _key + _separator + static_cast<std::string>(_value);
        }
    }
    return _return;
}

auto zpt::path::split(std::string const& _to_split) -> zpt::json {
    return zpt::split(_to_split, "/", true);
}

auto zpt::path::join(zpt::json _to_join) -> std::string {
    return std::string("/") + zpt::join(_to_join, "/");
}

auto zpt::conf::getopt(int _argc, char* _argv[]) -> zpt::json {
    zpt::json _non_positional = zpt::json::array();
    zpt::json _return = { "--", _non_positional };
    std::string _last("");
    for (int _i = 1; _i != _argc; _i++) {
        std::string _arg(_argv[_i]);
        if (_arg.find("--enable") == 0 || _arg.find("--disable") == 0 ||
            _arg.find("--force") == 0) {
            if (_last.length() != 0) {
                if (!_return(_last)->ok()) { _return << _last << zpt::json::array(); }
                _return[_last] << true;
            }
            _arg.erase(0, 2);
            if (!_return(_arg)->ok()) { _return << _arg << zpt::json::array(); }
            _return[_arg] << true;
            _last.assign("");
        }
        else if (_arg.find("-enable") == 0 || _arg.find("-disable") == 0 ||
                 _arg.find("-force") == 0) {
            if (_last.length() != 0) {
                if (!_return(_last)->ok()) { _return << _last << zpt::json::array(); }
                _return[_last] << true;
            }
            _arg.erase(0, 1);
            if (!_return(_arg)->ok()) { _return << _arg << zpt::json::array(); }
            _return[_arg] << true;
            _last.assign("");
        }
        else if (_arg.find("--") == 0) {
            if (_last.length() != 0) {
                if (!_return(_last)->ok()) { _return << _last << zpt::json::array(); }
                _return[_last] << true;
            }
            _arg.erase(0, 2);
            _last.assign(_arg);
        }
        else if (_arg.find("-") == 0) {
            if (_last.length() != 0) {
                if (!_return(_last)->ok()) { _return << _last << zpt::json::array(); }
                _return[_last] << true;
            }
            _arg.erase(0, 1);
            _last.assign(_arg);
        }
        else {
            if (_last.length() != 0) {
                if (!_return(_last)->ok()) { _return << _last << zpt::json::array(); }
                _return[_last] << _arg;
                _last.assign("");
            }
            else { _non_positional << _arg; }
        }
    }
    if (_last.length() != 0) {
        if (!_return(_last)->ok()) { _return << _last << zpt::json::array(); }
        _return[_last] << true;
    }
    return _return;
}

auto zpt::conf::setup(zpt::json _options) -> void {
    zpt::conf::dirs(_options);
    zpt::conf::env(_options);
}

auto zpt::conf::evaluate_ref(zpt::json _options,
                             zpt::json _parent,
                             std::variant<std::string, size_t> const& _parent_key,
                             std::filesystem::path const& _context,
                             zpt::json _root) -> void {
    for (auto [_idx, _key, _value] : _options) {
        if (_options->is_object()) {
            if (_key == "$ref") {
                auto& _ref = _value->string();
                zpt::json _other;

                if (_ref[0] == '#') {
                    _ref = _ref.substr(2);
                    _other = _root->get_path(_ref, "/");
                }
                else if (_ref.find("file:") == 0) {
                    _ref = _ref.substr(5);
                    std::filesystem::path _path{ _ref };
                    if (!_path.is_absolute()) {
                        _path = std::filesystem::canonical(_context / _path);
                    }
                    else { _path = std::filesystem::canonical(_path); }
                    zpt::conf::file(_path, _other, _root);
                }

                if (_parent_key.index() == 1) { _parent[std::get<size_t>(_parent_key)] = _other; }
                else {
                    if (std::get<std::string>(_parent_key).length() == 0) { _parent = _other; }
                    else { _parent[std::get<std::string>(_parent_key)] = _other; }
                }
            }
            else { zpt::conf::evaluate_ref(_value, _options, _key, _context, _root); }
        }
        else if (_options->is_array()) {
            zpt::conf::evaluate_ref(_value, _options, _idx, _context, _root);
        }
    }
}

auto zpt::conf::file(std::filesystem::path const& _file, zpt::json& _options, zpt::json _root)
  -> void {
    zpt::json _conf;
    std::ifstream _ifs;
    _ifs.open(_file.string());
    expect(_ifs.is_open(), "no such file '" << _file << "'");

    std::filesystem::path _context = std::filesystem::absolute(_file);
    _context.remove_filename();
    try {
        _ifs >> _conf;
        zpt::conf::evaluate_ref(_conf, _conf, "", _context, _root);
        _options |= _conf;
    }
    catch (zpt::SyntaxErrorException const& _e) {
        _conf = zpt::undefined;
        expect(_conf->ok(), "syntax error parsing file: " << _file << ": " << _e.what());
    }

    _ifs.close();
}

auto zpt::conf::dirs(std::string const& _dir, zpt::json& _options) -> void {
    std::vector<std::string> _non_positional;
    if (std::filesystem::is_directory(_dir)) { zpt::glob(_dir, _non_positional, "(.*)\\.conf"); }
    else { _non_positional.push_back(_dir); }
    std::sort(_non_positional.begin(), _non_positional.end());
    for (auto _file : _non_positional) {
        expect(std::filesystem::exists(_file), "'" << _file << "' can't be found.");
        if (std::filesystem::is_directory(_file)) { zpt::conf::dirs(_file, _options); }
        else { zpt::conf::file(static_cast<std::string>(_file), _options, _options); }
    }
}

auto zpt::conf::dirs(zpt::json& _options) -> void {
    bool* _redo = new bool(false);
    do {
        *_redo = false;
        zpt::json _traversable = _options->clone();
        zpt::json::traverse(
          _traversable,
          [&](std::string const& _key, zpt::json _item, std::string const& _path) -> void {
              if (_key == "$include") {
                  zpt::json _object = (_path.rfind(".") != std::string::npos
                                         ? _options->get_path(_path.substr(0, _path.rfind(".")))
                                         : _options);
                  if (_item->is_array()) {
                      for (auto const& [_idx, _key, _file] : _item) {
                          zpt::conf::dirs((std::string)_file, _object);
                      }
                  }
                  else { zpt::conf::dirs((std::string)_item, _object); }
                  _object->object()->pop("$include");
                  *_redo = true;
              }
          });
    } while (*_redo);
    delete _redo;
}

auto zpt::conf::env(zpt::json& _options) -> void {
    zpt::json _traversable = _options->clone();
    zpt::json::traverse(
      _traversable,
      [&](std::string const&, zpt::json _item, std::string const& _object_path) -> void {
          if (_item->type() != zpt::JSString) { return; }
          std::string _value{ static_cast<std::string>(_item) };
          bool _changed{ false };
          for (size_t _idx = _value.find("$"); _idx != std::string::npos;
               _idx = _value.find("$", _idx + 1)) {
              std::string _var = _value.substr(_idx + 2, _value.find("}", _idx) - _idx - 2);
              const char* _env_value = std::getenv(_var.data());
              if (_env_value != nullptr) {
                  zpt::replace(
                    _value, std::string("${") + _var + std::string("}"), zpt::r_trim(_env_value));
                  _changed = true;
              }
          }
          if (_changed) { _options->set_path(_object_path, zpt::json::string(_value)); }
      });
}

auto zpt::email::parse(std::string const& _email) -> zpt::json {
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

auto zpt::parameters::parse(int _argc, char* _argv[], zpt::json _config) -> zpt::json {
    zpt::json _return = zpt::json::object();
    zpt::json _values = zpt::json::array();

    std::string _key{ "" };
    std::string _value{ "" };
    for (int _i = 1; _i != _argc; _i++) {
        std::string _arg(_argv[_i]);

        if (_arg == "--") {
            _key.assign("");
            _value.assign("");
        }
        else if (_arg.find("--") == 0 || _arg.find("-") == 0) {
            if (_key.length() != 0) { _return << _key << true; }
            _key.assign(_arg);
            _value.assign("");
        }
        else if (_key.length() != 0) {
            _value.assign(_arg);
            zpt::json _js_value;
            if (_config(_key)("type") == "string") { _js_value = zpt::json::string(_value); }
            else if (_config(_key)("type") == "bool") {
                _js_value = zpt::json::boolean(_value == "true" || _value == "1");
            }
            else if (_config(_key)("type") == "int") {
                int _int_value;
                std::istringstream _iss;
                _iss.str(_value);
                _iss >> _int_value;
                expect(!_iss.fail(), "value provided for '" << _key << "' is not an integer");
                _js_value = zpt::json::integer(_int_value);
            }
            else if (_config(_key)("type") == "double") {
                double _dbl_value;
                std::istringstream _iss;
                _iss.str(_value);
                _iss >> _dbl_value;
                expect(!_iss.fail(), "value provided for '" << _key << "' is not a double");
                _js_value = zpt::json::floating(_dbl_value);
            }
            else { expect(false, "option type must be one of [ string, bool, int, double ]"); }

            _values = _return[_key];
            if (_values == zpt::undefined) { _return << _key << _js_value; }
            else if (_values->type() == zpt::JSArray) { _values << _js_value; }
            else {
                zpt::json _old = _values;
                _values = zpt::json::array();
                _values << _old << _js_value;
                _return << _key << _values;
            }
            _key.assign("");
        }
        else {
            _value.assign(_arg);
            _values = _return["--"];
            if (_values == zpt::undefined) {
                _values = zpt::json::array();
                _return << "--" << _values;
            }
            _values << _value;
        }
    }
    if (_key.length() != 0) { _return << _key << true; }

    for (auto const& [_, _key, _option] : _return) {
        if (_key == "--") { continue; }
        for (auto const& [_, __, _cfg_value] : _config(_key)("options")) {
            if (_cfg_value == "multiple") {
                if (_option->type() != zpt::JSArray) {
                    _return << _key << zpt::json{ zpt::array, _option };
                }
            }
        }
    }

    return _return;
}

auto zpt::parameters::verify(zpt::json _to_check, zpt::json _rules) -> void {
    for (auto const& [_, _key, _parameter] : _to_check) {
        if (_key == "--") { continue; }

        expect(_rules(_key)->type() == zpt::JSObject, "'" << _key << "' is not a valid parameter");

        for (auto const& [_, _cfg_name, _cfg_value] : _rules(_key)) {
            if (_cfg_name == "type") {
                expect((_parameter->type() != zpt::JSArray &&
                        zpt::to_string(_parameter->type()) == _cfg_value->string()) ||
                         (_parameter->type() == zpt::JSArray &&
                          zpt::to_string(_parameter(0)->type()) == _cfg_value->string()),
                       "'" << _key << "' parameter has the wrong type");
            }
            if (_cfg_value == "pattern") {
                expect(
                  _cfg_value->type() == zpt::JSRegex &&
                    std::regex_match(static_cast<std::string>(_parameter), *_cfg_value->regex()),
                  "'" << _key << "' parameter pattern doesn't match");
            }
        }
    }

    for (auto const& [_, _key, _config] : _rules) {
        for (auto const& [_, _cfg_name, _cfg_value] : _config("options")) {
            if (_cfg_name == "mandatory") {
                expect(_to_check(_key)->ok(), "'" << _key << "' is a required parameter");
            }
            if (_cfg_value == "single") {
                expect(_to_check(_key)->type() != zpt::JSArray,
                       std::string{ "'" } + _key +
                         std::string{ "' option can't have multiple values" });
            }
        }
    }
}

auto zpt::parameters::usage(zpt::json _config) -> std::string {
    std::ostringstream _oss;
    _oss << "Usage: zpt [OPTIONS]" << std::endl << "Options:" << std::endl << std::flush;
    size_t _max_len{ 0 };
    for (auto const& [_, _key, _parameter] : _config) {
        size_t _len = _key.length();
        if (_parameter("type")->ok() && _parameter("type") != "void") {
            _len += 2 + _parameter("type")->string().length();
        }
        _max_len = std::max(_max_len, _len);
    }
    for (auto const& [_, _key, _parameter] : _config) {
        size_t _len = _key.length();
        _oss << "\t" << _key << " ";
        if (_parameter("type")->ok() && _parameter("type") != "void") {
            _oss << "<" << _parameter("type")->string() << ">" << std::flush;
            _len += 2 + _parameter("type")->string().length();
        }
        _oss << std::string(_max_len + 3 - _len, ' ') << _parameter("description")->string() << " "
             << _parameter("options") << std::endl
             << std::flush;
    }
    return zpt::r_replace(_oss.str(), "\"", "");
}

auto zpt::test::location(zpt::json _location) -> bool {
    return (_location->is_object() && _location("longitude")->is_number() &&
            _location("latitude")->is_number()) ||
           (_location->is_array() && _location->size() == 2 && _location(0)->is_number() &&
            _location(1)->is_number());
}

auto zpt::test::timestamp(zpt::json _timestamp) -> bool {
    if (_timestamp->type() == zpt::JSDate) { return true; }
    if (_timestamp->type() != zpt::JSString) { return false; }
    static const std::regex _timestamp_rgx("^([0-9]{4})-"
                                           "([0-9]{2})-"
                                           "([0-9]{2})T"
                                           "([0-9]{2}):"
                                           "([0-9]{2}):"
                                           "([0-9]{2})."
                                           "([0-9]{3})([+-])"
                                           "([0-9]{4})$");
    return std::regex_match(_timestamp->string(), _timestamp_rgx);
}

auto zpt::http::cookies::deserialize(std::string const& _cookie_header) -> zpt::json {
    zpt::json _splitted = zpt::split(_cookie_header, ";");
    zpt::json _return = zpt::json::object();
    bool _first = true;
    for (auto const& [_idx, _key, _part] : _splitted) {
        std::string _value = std::string(_part);
        zpt::trim(_value);
        if (_first) {
            _return << "value" << zpt::json::string(_value);
            _first = false;
        }
        else {
            zpt::json _pair = zpt::split(_value, "=");
            if (_pair->size() == 2) { _return << _pair(0)->string() << _pair[1]; }
        }
    }
    return _return;
}

auto zpt::http::cookies::serialize(zpt::json _info) -> std::string {
    std::string _return((std::string)_info("value"));
    for (auto const& [_idx, _key, _field] : _info) {
        if (_key == "value") { continue; }
        _return += std::string("; ") + _key + std::string("=") + std::string(_field);
    }
    return _return;
}

extern "C" auto zpt_lex_json() -> int { return 1; }
