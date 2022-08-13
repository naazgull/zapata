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

#include <zapata/net/transport/http.h>
#include <zapata/base.h>
#include <zapata/uri/uri.h>
#include <zapata/net/socket/socket_stream.h>

auto
zpt::HTTP_SERVER_SOCKET() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

auto
zpt::net::transport::http::make_request() const -> zpt::message {
    auto _to_return = zpt::make_message<zpt::http::basic_request>();
    zpt::init(message_cast<zpt::http::basic_request>(_to_return));
    return _to_return;
}

auto
zpt::net::transport::http::make_reply() const -> zpt::message {
    auto _to_return = zpt::make_message<zpt::http::basic_reply>();
    zpt::init(message_cast<zpt::http::basic_reply>(_to_return));
    return _to_return;
}

auto
zpt::net::transport::http::make_reply(zpt::message _request) const -> zpt::message {
    auto _to_return = zpt::make_message<zpt::http::basic_reply>(
      message_cast<zpt::http::basic_request>(_request), true);
    zpt::init(message_cast<zpt::http::basic_reply>(_to_return));
    return _to_return;
}

auto
zpt::net::transport::http::receive(zpt::basic_stream& _stream) const -> zpt::message {
    if (_stream.state() == zpt::stream_state::IDLE) {
        _stream.state() = zpt::stream_state::PROCESSING;
        return this->receive_request(_stream);
    }
    else if (_stream.state() == zpt::stream_state::WAITING) {
        _stream.state() = zpt::stream_state::IDLE;
        return this->receive_reply(_stream);
    }
    expect(false, "Stream not in a valida state for receiving");
}

auto
zpt::net::transport::http::send(zpt::basic_stream& _stream, zpt::message _to_send) const -> void {
    expect(_stream.transport() == "http", "Stream underlying transport isn't HTTP");
    expect(_stream.state() == zpt::stream_state::IDLE ||
             _stream.state() == zpt::stream_state::PROCESSING,
           "Stream not in a valida state for receiving");

    zlog("Sending HTTP message:\n" << _to_send, zpt::trace);
    _stream << _to_send << std::flush;
    if (_stream.state() == zpt::stream_state::IDLE) {
        _stream.state() = zpt::stream_state::WAITING;
    }
    else if (_stream.state() == zpt::stream_state::PROCESSING) {
        _stream.state() = zpt::stream_state::IDLE;
    }
}

auto
zpt::net::transport::http::receive_request(zpt::basic_stream& _stream) const -> zpt::message {
    expect(_stream.transport() == "http", "Stream underlying transport isn't HTTP");
    auto _request = zpt::make_message<zpt::http::basic_request>();
    _stream >> std::noskipws >> _request;
    _request->uri()["domain"] = _request->headers()["Host"];
    zlog("Received HTTP message:\n" << _request, zpt::trace);
    return _request;
}

auto
zpt::net::transport::http::receive_reply(zpt::basic_stream& _stream) const -> zpt::message {
    expect(_stream.transport() == "http", "Stream underlying transport isn't HTTP");
    auto _reply = zpt::make_message<zpt::http::basic_reply>();
    _stream >> std::noskipws >> _reply;
    zlog("Received HTTP message:\n" << _reply, zpt::trace);
    return _reply;
}
