/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <zapata/transport/transport.h>
#include <zapata/exceptions/NoMoreElementsException.h>

zpt::transport::transport(zpt::transport const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::transport::transport(zpt::transport&& _rhs)
  : __underlying{ std::move(_rhs.__underlying) } {}

auto
zpt::transport::operator=(zpt::transport const& _rhs) -> zpt::transport& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::transport::operator=(zpt::transport&& _rhs) -> zpt::transport& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::transport::operator-> () -> zpt::transport::transport_t* {
    return this->__underlying.get();
}

auto zpt::transport::operator*() -> zpt::transport::transport_t& {
    return *this->__underlying.get();
}

zpt::transport::transport(std::unique_ptr<zpt::transport::transport_t> _underlying)
  : __underlying{ _underlying.release() } {}

auto
zpt::transport::layer::add(std::string _scheme, zpt::transport _transport)
  -> zpt::transport::layer& {
    this->__underlying.insert(std::make_pair(_scheme, _transport));
    return (*this);
}

auto
zpt::transport::layer::get(std::string _scheme) -> zpt::transport& {
    auto _found = this->__underlying.find(_scheme);
    if (_found == this->__underlying.end()) {
        throw zpt::NoMoreElementsException("there is no such transport");
    }
    return _found->second;
}

auto
zpt::transport::layer::begin() -> std::map<std::string, zpt::transport>::iterator {
    return this->__underlying.begin();
}

auto
zpt::transport::layer::end() -> std::map<std::string, zpt::transport>::iterator {
    return this->__underlying.end();
}

zpt::message::message(zpt::stream* _stream)
  : __underlying{ std::make_shared<zpt::message::message_t>(_stream) } {}

zpt::message::message(zpt::message const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::message::message(zpt::message&& _rhs)
  : __underlying{ std::move(_rhs.__underlying) } {}

auto
zpt::message::operator=(zpt::message const& _rhs) -> zpt::message& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::message::operator=(zpt::message&& _rhs) -> zpt::message& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::message::operator-> () -> zpt::message::message_t* {
    return this->__underlying.get();
}

auto zpt::message::operator*() -> zpt::message::message_t& {
    return *this->__underlying.get();
}

zpt::message::message_t::message_t(zpt::stream* _stream)
  : __stream{ _stream }
  , __scheme{ _stream->transport() } {}

auto
zpt::message::message_t::stream() -> zpt::stream& {
    return *this->__stream;
}

auto
zpt::message::message_t::uri() -> std::string& {
    return this->__uri;
}

auto
zpt::message::message_t::version() -> std::string& {
    return this->__version;
}

auto
zpt::message::message_t::scheme() -> std::string& {
    return this->__scheme;
}

auto
zpt::message::message_t::method() -> zpt::performative& {
    return this->__method;
}

auto
zpt::message::message_t::headers() -> zpt::json& {
    return this->__headers;
}

auto
zpt::message::message_t::received() -> zpt::json& {
    return this->__received;
}

auto
zpt::message::message_t::to_send() -> zpt::json& {
    return this->__send;
}

auto
zpt::message::message_t::status() -> zpt::status& {
    return this->__status;
}

auto
zpt::message::message_t::keep_alive() -> bool& {
    return this->__keep_alive;
}
