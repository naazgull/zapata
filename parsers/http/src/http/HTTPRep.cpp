#include <zapata/http/HTTPObj.h>

#include <iostream>
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/NoHeaderNameException.h>
#include <zapata/http/HTTPParser.h>

zpt::HTTPRepT::HTTPRepT()
  : __status{ zpt::http::status::HTTP100 } {
    this->__headers["Content-Length"] = "0";
}

zpt::HTTPRepT::~HTTPRepT() {}

auto
zpt::HTTPRepT::status() -> zpt::http::status {
    return this->__status;
}

auto
zpt::HTTPRepT::status(zpt::http::status _in) -> void {
    this->__status = _in;
}

auto
zpt::HTTPRepT::stringify(std::ostream& _out) -> void {
    zpt::performative _status = this->__status > 99 ? this->__status : 100;
    _out << "HTTP/" << this->version() << " " << std::to_string(_status);
    if (this->version()[0] != '2') {
        _out << " " << zpt::http::status_names[_status];
    }
    _out << CRLF;
    for (auto i : this->__headers) {
        _out << i.first << ": " << i.second << CRLF;
    }
    _out << CRLF << this->__body;
}

auto
zpt::HTTPRepT::stringify(std::string& _out) -> void {
    std::ostringstream _oss;
    this->stringify(_oss);
    _oss << std::flush;
    _out.assign(_oss.str());
}

zpt::HTTPRep::HTTPRep()
  : __underlying{ std::make_shared<HTTPRepT>() } {}

zpt::HTTPRep::~HTTPRep() {}

auto zpt::HTTPRep::operator*() -> zpt::HTTPRepT& {
    return *this->__underlying.get();
}

auto zpt::HTTPRep::operator-> () -> zpt::HTTPRepT* {
    return this->__underlying.get();
}

zpt::HTTPRep::operator std::string() {
    return (*this)->to_string();
}

auto
zpt::HTTPRep::parse(std::istream& _in) -> void {
    static thread_local zpt::HTTPParser _p;
    _p.switchRoots(*this);
    _p.switchStreams(_in);
    _p.parse();
}

auto operator"" _HTTP_REPLY(const char* _string, size_t _length) -> zpt::http::rep {
    std::istringstream _oss;
    zpt::http::rep _to_return;
    _oss.str(std::string{ _string, _length });
    _oss >> _to_return;
    return _to_return;
}
