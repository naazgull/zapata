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

zpt::message::message(zpt::stream _stream)
  : __stream{ _stream } {}

zpt::message::message(zpt::message const& _rhs)
  : __stream{ _rhs.__stream }
  , __received{ _rhs.__received }
  , __send{ _rhs.__send } {
}

zpt::message::message(zpt::message&& _rhs)
  : __stream{ std::move(_rhs.__stream) }
  , __received{ std::move(_rhs.__received) }
  , __send{ std::move(_rhs.__send) } {
}

auto
zpt::message::operator=(zpt::message const& _rhs) -> zpt::message& {
    this->__stream = _rhs.__stream;
    this->__received = _rhs.__received;
    this->__send = _rhs.__send;
    return (*this);
}

auto
zpt::message::operator=(zpt::message&& _rhs) -> zpt::message& {
    this->__stream = std::move(_rhs.__stream);
    this->__received = std::move(_rhs.__received);
    this->__send = std::move(_rhs.__send);
    return (*this);
}

auto
zpt::message::operator<<(zpt::json _rhs) -> zpt::message& {
    if (this->__which_in == zpt::received) {
        this->__received = this->__received + _rhs;
    }
    else {
        this->__send = this->__send + _rhs;
    }
    return (*this);
}

auto
zpt::message::operator<<(zpt::action _rhs) -> zpt::message& {
    this->__which_in = _rhs;
    return (*this);
}

auto
zpt::message::get_received() -> zpt::json& {
    return this->__received;
}

auto
zpt::message::get_to_send() -> zpt::json& {
    return this->__send;
}

auto
zpt::message::stream() -> zpt::stream& {
    return this->__stream;
}
