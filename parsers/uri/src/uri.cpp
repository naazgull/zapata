#include <fstream>
#include <iostream>
#include <signal.h>
#include <string>
#include <unistd.h>
#include <chrono>
#include <zapata/json/json.h>
#include <zapata/uri/URIParser.h>
#include <zapata/uri/uri.h>

auto
zpt::uri::parse(std::string const& _in, zpt::JSONType _type) -> zpt::json {
    std::istringstream _iss;
    _iss.str(_in);
    return zpt::uri::parse(_iss, _type);
}

auto
zpt::uri::parse(std::istream& _in, zpt::JSONType _type) -> zpt::json {
    static thread_local zpt::URIParser _thread_local_parser;
    zpt::json _root = _type == zpt::JSArray ? zpt::json::array() : zpt::json::object();
    _thread_local_parser.switchRoots(_root);
    _thread_local_parser.switchStreams(_in);
    _thread_local_parser.parse();
    if (_type == zpt::JSObject) { _root->object()->pop("__aux"); }
    return _root;
}

auto
zpt::uri::to_string(zpt::json _uri) -> std::string {
    std::ostringstream _oss;
    if (_uri->type() == zpt::JSObject) {
        if (_uri("scheme")->ok()) {
            _oss << _uri("scheme")->string();
            if (_uri("scheme_options")->ok()) {
                _oss << "+" << zpt::join(_uri("scheme_options"), "+") << std::flush;
            }
            _oss << ":" << std::flush;
        }
        if (_uri("domain")->ok()) {
            _oss << "//" << std::flush;
            if (_uri("user")("name")->ok()) {
                _oss << _uri("user")("name")->string() << "@" << std::flush;
            }
            _oss << _uri("domain")->string() << std::flush;
            if (_uri("port")->ok()) { _oss << ":" << _uri("port") << std::flush; }
        }
        _oss << (_uri("is_relative")->ok() && _uri("is_relative")->boolean() ? "" : "/");
        if (_uri("path")->ok()) { _oss << zpt::join(_uri("path"), "/") << std::flush; }
        if (_uri("params")->ok()) {
            bool _first{ true };
            _oss << "?" << std::flush;
            for (auto [_, _key, _value] : _uri("params")) {
                if (!_first) { _oss << "&"; }
                _first = false;
                _oss << _key << "=" << (_value->ok() ? static_cast<std::string>(_value) : "")
                     << std::flush;
            }
        }
        if (_uri("anchor")->ok()) { _oss << "#" << _uri("anchor")->string() << std::flush; }
    }
    return _oss.str();
}

auto
zpt::uri::to_regex(zpt::json _in) -> zpt::json {
    if (_in->type() == zpt::JSObject) { return zpt::uri::to_regex_object(_in); }
    if (_in->type() == zpt::JSArray) { return zpt::uri::to_regex_array(_in); }
    return _in;
}

auto
zpt::uri::to_regex_object(zpt::json _in) -> zpt::json {
    zpt::json _to_return = zpt::json::object();
    for (auto [_, _key, _item] : _in) {
        if (_key == "path") {
            zpt::json _parts = zpt::json::array();
            for (auto [__, ___, _part] : _item) {
                auto _casted = static_cast<std::string>(_part);
                auto _length = _casted.length();
                if (_casted[0] == '{' && _casted[1] == ':' && _casted[_length - 2] == ':' &&
                    _casted[_length - 1] == '}') {
                    _casted = zpt::r_replace(zpt::r_replace(_casted, "{:", ""), ":}", "");
                    _parts << zpt::regex{ _casted };
                }
                else { _parts << _part; }
            }
            _to_return << "path" << _parts;
        }
        else {
            auto _casted = static_cast<std::string>(_item);
            auto _length = _casted.length();
            if (_casted[0] == '{' && _casted[1] == ':' && _casted[_length - 2] == ':' &&
                _casted[_length - 1] == '}') {
                _casted = zpt::r_replace(zpt::r_replace(_casted, "{:", ""), ":}", "");
                _to_return << _key << zpt::regex{ _casted };
            }
            else { _to_return << _key << _item; }
        }
    }
    return _to_return;
}

auto
zpt::uri::to_regex_array(zpt::json _in) -> zpt::json {
    zpt::json _to_return = zpt::json::array();
    for (auto [_, __, _item] : _in) {
        auto _casted = static_cast<std::string>(_item);
        auto _length = _casted.length();
        if (_casted[0] == '{' && _casted[1] == ':' && _casted[_length - 2] == ':' &&
            _casted[_length - 1] == '}') {
            _casted = zpt::r_replace(zpt::r_replace(_casted, "{:", ""), ":}", "");
            _to_return << zpt::regex{ _casted };
        }
        else { _to_return << _item; }
    }
    return _to_return;
}

auto
zpt::uri::path::to_string(zpt::json _uri) -> std::string {
    std::ostringstream _oss;
    if (_uri->type() == zpt::JSObject) {
        if (_uri["path"]->ok()) {
            _oss << (_uri["is_relative"]->boolean() ? "" : "/") << zpt::join(_uri["path"], "/")
                 << std::flush;
        }
    }
    else {
        _oss << (_uri[0] == "." || _uri[0] == ".." ? "" : "/") << zpt::join(_uri["path"], "/")
             << std::flush;
    }
    return _oss.str();
}
