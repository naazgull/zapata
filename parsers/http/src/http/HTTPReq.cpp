#include <zapata/http/HTTPObj.h>

#include <iostream>
#include <zapata/log/log.h>
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/NoHeaderNameException.h>
#include <zapata/http/HTTPParser.h>
#include <zapata/json/json.h>
#include <zapata/uri.h>

zpt::HTTPReqT::HTTPReqT()
  : __method(zpt::Get) {}

zpt::HTTPReqT::~HTTPReqT() {}

auto
zpt::HTTPReqT::method() const -> zpt::performative {
    return this->__method;
}

auto
zpt::HTTPReqT::method(zpt::performative _method) -> void {
    this->__method = _method;
}

auto
zpt::HTTPReqT::url(std::string const& _url) -> void {
    this->__url_parts = zpt::uri::parse(_url);
    this->__url.assign(_url.data());
    this->__path.assign("/" + zpt::join(this->__url_parts["path"], "/"));
}

auto
zpt::HTTPReqT::url() const -> std::string const& {
    return this->__url;
}

auto
zpt::HTTPReqT::scheme() const -> std::string const {
    if (this->__url_parts["scheme"]->ok()) { return this->__url_parts["scheme"]; }
    return "http";
}

auto
zpt::HTTPReqT::domain() const -> std::string const {
    if (this->__url_parts["domain"]->ok()) { return this->__url_parts["domain"]; }
    return "localhost";
}

auto
zpt::HTTPReqT::path() const -> std::string const& {
    return this->__path;
}

auto
zpt::HTTPReqT::params() const -> zpt::json {
    return this->__url_parts["params"];
}

auto
zpt::HTTPReqT::anchor() const -> std::string {
    return this->__url_parts["anchor"];
}

auto
zpt::HTTPReqT::stringify(std::ostream& _out) const -> void {
    _out << zpt::http::method_names[this->__method] << " " << this->__url;
    _out << " HTTP/" << this->version() << CRLF;

    for (auto h : this->__headers) { _out << h.first << ": " << h.second << CRLF; }
    _out << CRLF << this->__body;
}

auto
zpt::HTTPReqT::stringify(std::string& _out) const -> void {
    std::ostringstream _oss;
    this->stringify(_oss);
    _oss << std::flush;
    _out.assign(_oss.str());
}

zpt::HTTPReq::HTTPReq()
  : __underlying{ std::make_shared<HTTPReqT>() } {}

zpt::HTTPReq::~HTTPReq() {}

auto
zpt::HTTPReq::operator*() -> zpt::HTTPReqT& {
    return *this->__underlying.get();
}

auto
zpt::HTTPReq::operator->() -> zpt::HTTPReqT* {
    return this->__underlying.get();
}

zpt::HTTPReq::operator std::string() { return (*this)->to_string(); }

auto
zpt::HTTPReq::parse(std::istream& _in) -> void {
    static thread_local zpt::HTTPParser _p;
    _p.switchRoots(*this);
    _p.switchStreams(_in);
    _p.parse();
}

auto operator"" _HTTP_REQUEST(const char* _string, size_t _length) -> zpt::http::req {
    std::istringstream _oss;
    zpt::http::req _to_return;
    _oss.str(std::string{ _string, _length });
    _oss >> _to_return;
    return _to_return;
}
