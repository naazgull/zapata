#include <zapata/http/config.h>
#include <zapata/http/http.h>

auto
zpt::http::from_str(std::string& _in, zpt::HTTPReq& _out) -> zpt::HTTPReq& {
    std::istringstream _ss;
    _ss.str(_in);
    zpt::HTTPParser _p;
    _p.switchRoots(_out);
    _p.switchStreams(_ss);
    _p.parse();
    return _out;
}

auto
zpt::http::from_str(std::string& _in, zpt::HTTPRep& _out) -> zpt::HTTPRep& {
    std::istringstream _ss;
    _ss.str(_in);
    zpt::HTTPParser _p;
    _p.switchRoots(_out);
    _p.switchStreams(_ss);
    _p.parse();
    return _out;
}

auto
zpt::http::from_file(std::ifstream& _in, zpt::HTTPReq& _out) -> zpt::HTTPReq& {
    if (_in.is_open()) {
        zpt::HTTPParser _p;
        _p.switchRoots(_out);
        _p.switchStreams(_in);
        _p.parse();
    }
    return _out;
}

auto
zpt::http::from_file(std::ifstream& _in, zpt::HTTPRep& _out) -> zpt::HTTPRep& {
    if (_in.is_open()) {
        zpt::HTTPParser _p;
        _p.switchRoots(_out);
        _p.switchStreams(_in);
        _p.parse();
    }
    return _out;
}

auto
zpt::http::from_stream(std::istream& _in, zpt::HTTPReq& _out) -> zpt::HTTPReq& {
    zpt::HTTPParser _p;
    _p.switchRoots(_out);
    _p.switchStreams(_in);
    _p.parse();
    return _out;
}

auto
zpt::http::from_stream(std::istream& _in, zpt::HTTPRep& _out) -> zpt::HTTPRep& {
    zpt::HTTPParser _p;
    _p.switchRoots(_out);
    _p.switchStreams(_in);
    _p.parse();
    return _out;
}

void
zpt::http::to_str(std::string& _out, HTTPRep& _in) {
    _in->stringify(_out);
}

void
zpt::http::to_str(std::string& _out, HTTPReq& _in) {
    _in->stringify(_out);
}

void
zpt::http::to_str(std::ostream& _out, HTTPRep& _in) {
    _in->stringify(_out);
    _out << std::flush;
}

void
zpt::http::to_str(std::ostream& _out, HTTPReq& _in) {
    _in->stringify(_out);
    _out << std::flush;
}

auto
zpt::http::to_str(zpt::performative _performative) -> std::string {
    switch (_performative) {
        case zpt::http::Get: {
            return "GET";
        }
        case zpt::http::Put: {
            return "PUT";
        }
        case zpt::http::Post: {
            return "POST";
        }
        case zpt::http::Delete: {
            return "DELETE";
        }
        case zpt::http::Head: {
            return "HEAD";
        }
        case zpt::http::Options: {
            return "OPTIONS";
        }
        case zpt::http::Patch: {
            return "PATCH";
        }
        case zpt::http::Reply: {
            return "REPLY";
        }
        case zpt::http::Msearch: {
            return "M-SEARCH";
        }
        case zpt::http::Notify: {
            return "NOTIFY";
        }
        case zpt::http::Trace: {
            return "TRACE";
        }
        case zpt::http::Connect: {
            return "CONNECT";
        }
    }
    return "HEAD";
}

auto
zpt::http::from_str(std::string const& _performative) -> zpt::performative {
    if (_performative == "GET" || _performative == "get") {
        return zpt::http::Get;
    }
    if (_performative == "PUT" || _performative == "put") {
        return zpt::http::Put;
    }
    if (_performative == "POST" || _performative == "post") {
        return zpt::http::Post;
    }
    if (_performative == "DELETE" || _performative == "delete") {
        return zpt::http::Delete;
    }
    if (_performative == "HEAD" || _performative == "head") {
        return zpt::http::Head;
    }
    if (_performative == "OPTIONS" || _performative == "options") {
        return zpt::http::Options;
    }
    if (_performative == "PATCH" || _performative == "patch") {
        return zpt::http::Patch;
    }
    if (_performative == "REPLY" || _performative == "reply") {
        return zpt::http::Reply;
    }
    if (_performative == "M-SEARCH" || _performative == "m-search") {
        return zpt::http::Msearch;
    }
    if (_performative == "NOTIFY" || _performative == "notify") {
        return zpt::http::Notify;
    }
    if (_performative == "TRACE" || _performative == "trace") {
        return zpt::http::Msearch;
    }
    if (_performative == "CONNECT" || _performative == "connect") {
        return zpt::http::Connect;
    }
    return 0;
}

extern "C" auto
zpt_lex_http() -> int {
    return 1;
}
