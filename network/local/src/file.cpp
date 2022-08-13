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
#include <zapata/uri.h>
#include <zapata/globals/globals.h>

auto
zpt::net::transport::file::send(zpt::exchange& _channel) const -> void {}

auto
zpt::net::transport::file::receive(zpt::exchange& _channel) const -> void {
    auto& _layer = zpt::globals::get<zpt::network::layer>(zpt::TRANSPORT_LAYER());
    auto& _is = static_cast<std::iostream&>(*(_channel->stream()));

    zpt::json _content = _layer.translate(_is, _channel->content_type()[0]);
    _channel->received() = { "headers", _channel->to_send()["headers"], "body", _content };
    _channel->keep_alive() = true;
}

auto
zpt::net::transport::file::resolve(zpt::json _uri) const -> zpt::exchange {
    expect(_uri["scheme"]->ok(), "URI parameter must contain 'scheme'");
    expect(_uri["scheme"] == "file", "scheme must be 'file'");
    std::string _path{ zpt::uri::path::to_string(_uri) };
    auto _stream = zpt::make_stream<std::basic_fstream<char>>(_path);
    _stream->transport("file");
    zpt::exchange _to_return{ _stream.release() };
    _to_return //
      ->scheme()
      .assign("file");
    _to_return //
      ->uri()
      .assign(_path);
    _to_return->keep_alive() = true;
    if (_uri["scheme_options"]->ok()) {
        _to_return->to_send() = { "headers", { "Content-Type", _uri["scheme_options"] } };
    }
    else { _to_return->to_send() = { "headers", { "Content-Type", "*/*" } }; }
    return _to_return;
}
