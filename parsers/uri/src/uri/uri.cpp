#include <fstream>
#include <iostream>
#include <signal.h>
#include <string>
#include <unistd.h>
#include <chrono>
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
        if (_uri["scheme"]->ok()) {
            _oss << _uri["scheme"]->string();
            if (_uri["scheme_options"]->ok()) {
                _oss << "+" << zpt::join(_uri["scheme_options"], "+") << std::flush;
            }
            _oss << ":" << std::flush;
        }
        if (_uri["domain"]->ok()) {
            _oss << "//" << std::flush;
            if (_uri["user"]["name"]->ok()) {
                _oss << _uri["user"]["name"]->string() << "@" << std::flush;
            }
            _oss << _uri["domain"]->string() << std::flush;
            if (_uri["port"]->ok()) { _oss << ":" << _uri["port"] << std::flush; }
        }
        if (_uri["path"]->ok()) {
            _oss << (_uri["is_relative"]->boolean() ? "" : "/") << zpt::join(_uri["path"], "/")
                 << std::flush;
        }
        if (_uri["params"]->ok()) {
            bool _first{ true };
            _oss << "?" << std::flush;
            for (auto [_, _key, _value] : _uri["params"]) {
                if (!_first) { _oss << "&"; }
                _first = false;
                _oss << _key << "=" << (_value->ok() ? static_cast<std::string>(_value) : "")
                     << std::flush;
            }
        }
        if (_uri["anchor"]->ok()) { _oss << "#" << _uri["anchor"]->string() << std::flush; }
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
                std::string _casted = static_cast<std::string>(_part);
                if (_casted[0] == '{') {
                    _casted = zpt::r_replace(zpt::r_replace(_casted, "{", ""), "}", "");
                    _parts << zpt::regex{ _casted };
                }
                else {
                    _parts << _part;
                }
            }
            _to_return << "path" << _parts;
        }
        else {
            std::string _casted = static_cast<std::string>(_item);
            if (_casted[0] == '{') {
                _casted = zpt::r_replace(zpt::r_replace(_casted, "{", ""), "}", "");
                _to_return << _key << zpt::regex{ _casted };
            }
            else {
                _to_return << _key << _item;
            }
        }
    }
    return _to_return;
}

auto
zpt::uri::to_regex_array(zpt::json _in) -> zpt::json {
    zpt::json _to_return = zpt::json::array();
    for (auto [_, __, _item] : _in) {
        std::string _casted = static_cast<std::string>(_item);
        if (_casted[0] == '{') {
            _casted = zpt::r_replace(zpt::r_replace(_casted, "{", ""), "}", "");
            _to_return << zpt::regex{ _casted };
        }
        else {
            _to_return << _item;
        }
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

// auto
// zpt::uri::parse(std::string const& _uri) -> zpt::json {
//     if (_uri.find("://") == std::string::npos) {
//         if (_uri[0] == '/') {
//             _uri = std::string("zpt://127.0.0.1") + _uri;
//         }
//         else {
//             _uri = std::string("zpt://") + _uri;
//         }
//     }
//     static const std::regex _uri_rgx("([@>]{0,1})([a-zA-Z][a-zA-Z0-9+.-]*):" // scheme:
//                                      "([/]{1,2})([^/]+)"                     // authority
//                                      "([^?#]*)"                              // path
//                                      "(?:\\?([^#]*))?"                       // ?query
//                                      "(?:#(.*))?"                            // #fragment
//     );

//     std::smatch _uri_matches;
//     std::regex_match(_uri, _uri_matches, _uri_rgx);

//     std::string _q_str = (std::string)_uri_matches[6];
//     zpt::json _query = zpt::uri::query::parse(_q_str);
//     zpt::json _authority = zpt::uri::authority::parse((std::string)_uri_matches[4]);
//     std::string _scheme = (std::string)_uri_matches[2];

//     return { "type",
//              (((std::string)_uri_matches[1]).length() == 0
//                 ? zpt::undefined
//                 : zpt::json::string((std::string)_uri_matches[1])),
//              "scheme",
//              _scheme,
//              "scheme_parts",
//              zpt::split(_scheme, "+"),
//              "authority",
//              (std::string)_uri_matches[4],
//              "domain",
//              _authority["domain"],
//              "port",
//              _authority["port"],
//              "user",
//              _authority["user"],
//              "password",
//              _authority["password"],
//              "path",
//              (std::string)_uri_matches[5],
//              "query",
//              (_query->size() != 0 ? _query : zpt::undefined),
//              "fragment",
//              zpt::url::r_decode((std::string)_uri_matches[7]) };
// }

// auto
// zpt::uri::query::parse(std::string const& _query) -> zpt::json {
//     static const std::regex _rgx("(^|&)"      // start of query or start of parameter "&"
//                                  "([^=&]*)=?" // parameter name and "=" if value is expected
//                                  "([^=&]*)"   // parameter value
//                                  "(?=(&|$))"  // forward reference, next should be end of query
//                                  or
//                                               // start of next parameter
//     );

//     zpt::json _return = zpt::json::object();
//     auto _begin = std::sregex_iterator(_query.begin(), _query.end(), _rgx);
//     auto _end = std::sregex_iterator();
//     for (auto _i = _begin; _i != _end; ++_i) {
//         std::smatch _match = *_i;
//         _return << (std::string)_match[2] << zpt::url::r_decode((std::string)_match[3]);
//     }

//     return _return;
// }

// auto
// zpt::uri::authority::parse(std::string const& _authority) -> zpt::json {
//     static const std::regex _auth_rgx("(([^:]+):([^@]+)@)?" // username and password
//                                       "([^:]+):?"           // domain
//                                       "(.*)"                // port
//     );

//     std::smatch _match;
//     std::regex_match(_authority, _match, _auth_rgx);
//     std::string _port = ((std::string)_match[5]);
//     std::string _user = ((std::string)_match[2]);
//     std::string _password = ((std::string)_match[3]);

//     zpt::json _return{
//         "domain",   ((std::string)_match[4]),
//         "port",     (_port.length() != 0 ? zpt::json::string(_port) : zpt::undefined),
//         "user",     (_user.length() != 0 ? zpt::json::string(_user) : zpt::undefined),
//         "password", (_password.length() != 0 ? zpt::json::string(_password) : zpt::undefined)
//     };
//     return _return;
// }

// auto
// zpt::uri::to_str(zpt::json _uri, zpt::json _opts) -> std::string {
//     std::string _authority =
//       ((_uri["authority"]->ok()
//           ? std::string(_uri["authority"])
//           : (((!_opts["user"]->ok() || bool(_opts["user"])) && _uri["user"]->ok()
//                 ? std::string(_uri["user"]) +
//                     ((!_opts["password"]->ok() || bool(_opts["password"])) &&
//                     _uri["password"]->ok()
//                        ? std::string(":") + std::string(_uri["password"])
//                        : std::string("")) +
//                     std::string("@")
//                 : std::string("")) +
//              std::string(_uri["address"]) +
//              ((!_opts["port"]->ok() || bool(_opts["port"])) && _uri["port"]->ok()
//                 ? std::string(":") + std::string(_uri["port"])
//                 : std::string("")))));
//     std::string _query;
//     if ((!_opts["query"]->ok() || bool(_opts["query"])) && _uri["query"]->is_object() &&
//         _uri["query"]->size() != 0) {
//         _query += std::string("?");
//         for (auto [_idx, _key, _p] : _uri["query"]) {
//             if (_query.length() != 1) {
//                 _query += std::string("&");
//             }
//             _query += _key + std::string("=") + zpt::url::r_encode(_p);
//         }
//     }
//     std::string _fragment;
//     if ((!_opts["fragment"]->ok() || bool(_opts["fragment"])) && _uri["fragment"]->is_string()) {
//         _fragment += std::string("#") + std::string(_uri["fragment"]);
//     }
//     return std::string(_uri["scheme"]) + std::string("://") + _authority +
//            std::string(_uri["path"]) + _query + _fragment;
// }
