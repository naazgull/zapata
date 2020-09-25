#include <zapata/http/HTTPObj.h>

#include <iostream>
#include <zapata/log/log.h>
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/NoHeaderNameException.h>
#include <zapata/http/HTTPParser.h>

zpt::HTTPReqT::HTTPReqT()
  : __method(zpt::http::Get) {}

zpt::HTTPReqT::~HTTPReqT() {}

auto
zpt::HTTPReqT::method() -> zpt::performative {
    return this->__method;
}

auto
zpt::HTTPReqT::method(zpt::performative _method) -> void {
    this->__method = _method;
}

auto
zpt::HTTPReqT::url() -> std::string& {
    return this->__url;
}

auto
zpt::HTTPReqT::url(std::string const& _url) -> void {
    this->__url.assign(_url.data());
}

auto
zpt::HTTPReqT::query() -> std::string& {
    return this->__query;
}

auto
zpt::HTTPReqT::query(std::string const& _query) -> void {
    this->__query.assign(_query.data());
}

auto
zpt::HTTPReqT::params() -> zpt::http::parameter_map& {
    return this->__params;
}

auto
zpt::HTTPReqT::param(std::string const& _idx) -> std::string const& {
    static std::string const _empty{ "" };
    auto _found = this->__params.find(_idx);
    if (_found != this->__params.end()) {
        return _found->second;
    }
    return _empty;
}

auto
zpt::HTTPReqT::param(std::string const& _name, std::string const& _value) -> void {
    if (this->__query.length() != 0) {
        this->__query.insert(this->__query.length(), "&");
    }
    this->__query.insert(this->__query.length(), _name);
    this->__query.insert(this->__query.length(), "=");
    this->__query.insert(this->__query.length(), _value);
    this->__params[_name] = _value;
}

auto
zpt::HTTPReqT::stringify(std::ostream& _out) -> void {
    _out << zpt::http::method_names[this->__method] << " " << this->__url;
    if (this->__query.length() != 0) {
        _out << "?" << this->__query;
    }
    else if (this->__params.size() != 0) {
        _out << "?";
        bool _first = true;
        for (auto i : this->__params) {
            if (!_first) {
                _out << "&";
            }
            _first = false;
            std::string _n(i.first);
            zpt::url::encode(_n);
            std::string _v(i.second);
            zpt::url::encode(_v);
            _out << _n << "=" << _v;
        }
    }
    _out << " HTTP/" << this->version() << CRLF;

    for (auto h : this->__headers) {
        _out << h.first << ": " << h.second << CRLF;
    }
    _out << CRLF << this->__body;
}

auto
zpt::HTTPReqT::stringify(std::string& _out) -> void {
    std::ostringstream _oss;
    this->stringify(_oss);
    _oss << std::flush;
    _out.assign(_oss.str());
}

zpt::HTTPReq::HTTPReq()
  : __underlying{ std::make_shared<HTTPReqT>() } {}

zpt::HTTPReq::~HTTPReq() {}

auto zpt::HTTPReq::operator*() -> zpt::HTTPReqT& {
    return *this->__underlying.get();
}

auto zpt::HTTPReq::operator-> () -> zpt::HTTPReqT* {
    return this->__underlying.get();
}

zpt::HTTPReq::operator std::string() {
    return (*this)->to_string();
}

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
