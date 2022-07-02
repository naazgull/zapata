
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

extern "C" auto
zpt_lex_http() -> int {
    return 1;
}
