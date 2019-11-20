#include <zapata/http/HTTPObj.h>

#include <iostream>
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/NoHeaderNameException.h>
#include <zapata/http/HTTPParser.h>

zpt::HTTPRepT::HTTPRepT()
  : __status{ zpt::http::status::HTTP100 } {}

zpt::HTTPRepT::~HTTPRepT() {}

zpt::http::status
zpt::HTTPRepT::status() {
    return this->__status;
}

auto
zpt::HTTPRepT::status(zpt::http::status _in) -> void {
    this->__status = _in;
}

auto
zpt::HTTPRepT::stringify(std::ostream& _out) -> void {
    std::string _ret;
    this->stringify(_ret);
    _out << _ret << std::flush;
}

auto
zpt::HTTPRepT::stringify(std::string& _out) -> void {
    _out.insert(_out.length(), "HTTP/1.1 "),
      _out.insert(_out.length(),
                  zpt::http::status_names[this->__status > 99 ? this->__status : 100]);
    _out.insert(_out.length(), CRLF);
    for (auto i : this->__headers) {
        _out.insert(_out.length(), i.first);
        _out.insert(_out.length(), ": ");
        _out.insert(_out.length(), i.second);
        _out.insert(_out.length(), CRLF);
    }
    _out.insert(_out.length(), CRLF);
    _out.insert(_out.length(), this->__body);
}

zpt::HTTPRep::HTTPRep()
  : __underlying{ std::make_shared<HTTPRepT>() } {}

zpt::HTTPRep::~HTTPRep() {}

auto zpt::HTTPRep::operator*() -> std::shared_ptr<zpt::HTTPRepT>& {
    return this->__underlying;
}

auto zpt::HTTPRep::operator-> () -> std::shared_ptr<zpt::HTTPRepT>& {
    return this->__underlying;
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
