#include <zapata/http/HTTPObj.h>

#include <iostream>
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
zpt::HTTPReqT::url(std::string _url) -> void {
    this->__url.assign(_url.data());
}

auto
zpt::HTTPReqT::query() -> std::string& {
    return this->__query;
}

auto
zpt::HTTPReqT::query(std::string _query) -> void {
    this->__query.assign(_query.data());
}

auto
zpt::HTTPReqT::params() -> zpt::http::parameter_map& {
    return this->__params;
}

auto
zpt::HTTPReqT::param(const char* _idx) -> std::string {
    auto _found = this->__params.find(_idx);
    if (_found != this->__params.end()) {
        return _found->second;
    }
    return "";
}

auto
zpt::HTTPReqT::param(const char* _name, const char* _value) -> void {
    this->__params.insert(std::make_pair(_name, _value));
}

auto
zpt::HTTPReqT::param(const char* _name, std::string _value) -> void {
    this->__params.insert(std::make_pair(_name, _value));
}

auto
zpt::HTTPReqT::param(std::string _name, std::string _value) -> void {
    this->__params.insert(std::make_pair(_name, _value));
}

auto
zpt::HTTPReqT::stringify(std::ostream& _out) -> void {
    std::string _ret;
    this->stringify(_ret);
    _out << _ret << std::flush;
}

auto
zpt::HTTPReqT::stringify(std::string& _out) -> void {
    _out.insert(_out.length(), zpt::http::method_names[this->__method]);
    _out.insert(_out.length(), " ");
    _out.insert(_out.length(), this->__url);
    if (this->__params.size() != 0) {
        _out.insert(_out.length(), "?");
        bool _first = true;
        for (auto i : this->__params) {
            if (!_first) {
                _out.insert(_out.length(), "&");
            }
            _first = false;
            std::string _n(i.first);
            zpt::url::encode(_n);
            std::string _v(i.second);
            zpt::url::encode(_v);
            _out.insert(_out.length(), _n);
            _out.insert(_out.length(), "=");
            _out.insert(_out.length(), _v);
        }
    }
    _out.insert(_out.length(), " HTTP/");
    _out.insert(_out.length(), this->version());
    _out.insert(_out.length(), CRLF);

    for (auto h : this->__headers) {
        _out.insert(_out.length(), h.first);
        _out.insert(_out.length(), ": ");
        _out.insert(_out.length(), h.second);
        _out.insert(_out.length(), CRLF);
    }
    _out.insert(_out.length(), CRLF);
    _out.insert(_out.length(), this->__body);
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
