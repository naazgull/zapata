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

#include <zapata/net/transport/local.h>
#include <zapata/base.h>
#include <zapata/globals/globals.h>
#include <zapata/net/socket/socket_stream.h>

auto
zpt::UNIX_SERVER_SOCKET() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

auto
zpt::net::transport::unix::receive_request(zpt::exchange& _channel) -> void {
    this->receive_reply(_channel);
}

auto
zpt::net::transport::unix::send_reply(zpt::exchange& _channel) -> void {
    this->send_request(_channel);
}

auto
zpt::net::transport::unix::send_request(zpt::exchange& _channel) -> void {
    if (_channel->to_send()->ok()) {
        _channel->stream() << _channel->to_send();
    }
}

auto
zpt::net::transport::unix::receive_reply(zpt::exchange& _channel) -> void {
    auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
    auto& _is = reinterpret_cast<std::istream&>(*(_channel->stream()));

    std::string _content_type = _channel->to_send()["headers"]["Content-Type"];
    zpt::json _content = _layer.translate(_is, _content_type);
    _channel->received() = { "headers", _channel->to_send()["headers"], "body", _content };
    _channel->keep_alive() = true;
}

auto
zpt::net::transport::unix::resolve(zpt::json _uri) -> zpt::exchange {
    expect(_uri["scheme"]->ok(), "URI parameter must contain 'scheme'", 500, 0);
    expect(_uri["scheme"] == "unix", "scheme must be 'unix'", 500, 0);
    std::string _path{ zpt::path::join(_uri["path"]) };
    auto _stream = zpt::stream::alloc<std::basic_fstream<char>>(_path);
    zpt::exchange _to_return{ _stream.release() };
    _to_return //
      ->scheme()
      .assign("unix");
    _to_return //
      ->uri()
      .assign(_path);
    _to_return->keep_alive() = true;
    if (_uri["scheme_options"]->ok()) {
        _to_return->to_send() = { "headers", { "Content-Type", _uri["scheme_options"] } };
    }
    else {
        _to_return->to_send() = { "headers", { "Content-Type", "*/*" } };
    }
    return _to_return;
}
