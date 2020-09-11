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

#include <zapata/net/transport/tcp.h>
#include <zapata/base.h>
#include <zapata/globals/globals.h>
#include <zapata/net/socket/socket_stream.h>

auto
zpt::TCP_SERVER_SOCKET() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

auto
zpt::net::transport::tcp::send(zpt::exchange& _channel) -> void {
    if (_channel->to_send()->ok()) {
        _channel->stream() << _channel->to_send();
    }
}

auto
zpt::net::transport::tcp::receive(zpt::exchange& _channel) -> void {
    auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
    auto& _is = static_cast<std::iostream&>(*(_channel->stream()));

    std::string _content_type = _channel->to_send()["headers"]["Content-Type"];
    _channel->received() =
      _layer.translate(_is, _content_type.length() == 0 ? "*/*" : _content_type);

    _channel //
      ->version()
      .assign("1.0");
    _channel //
      ->scheme()
      .assign("tcp");

    if (!_channel->received()["performative"]->ok() || _channel->received()["status"]->ok()) {
        _channel->received() << "performative" << 7;
    }
    if (!_channel->received()["resource"]->ok()) {
        _channel //
          ->uri()
          .assign("/" + std::to_string(static_cast<int>(_channel->stream())));
    }
    else {
        _channel //
          ->uri()
          .assign(_channel->received()["resource"]);
    }

    _channel->keep_alive() = true;
}

auto
zpt::net::transport::tcp::resolve(zpt::json _uri) -> zpt::exchange {
    expect(_uri["scheme"]->ok() && _uri["domain"]->ok() && _uri["port"]->ok(),
           "URI parameter must contain 'scheme', 'domain' and 'port'",
           500,
           0);
    expect(_uri["scheme"] == "tcp", "scheme must be 'tcp'", 500, 0);
    auto _stream = zpt::stream::alloc<zpt::basic_socketstream<char>>(
      _uri["domain"]->string(), static_cast<uint16_t>(_uri["port"]->integer()));
    _stream->transport("tcp");
    zpt::exchange _to_return{ _stream.release() };
    _to_return //
      ->scheme()
      .assign("tcp");
    _to_return //
      ->uri()
      .assign(zpt::path::join(_uri["path"]));
    _to_return->keep_alive() = true;
    if (_uri["scheme_options"]->ok()) {
        _to_return->to_send() = { "headers", { "Content-Type", _uri["scheme_options"] } };
    }
    else {
        _to_return->to_send() = { "headers", { "Content-Type", "*/*" } };
    }
    return _to_return;
}
