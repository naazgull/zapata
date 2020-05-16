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

auto
zpt::net::transport::file::receive_request(zpt::exchange& _channel) -> void {
    this->receive_reply(_channel);
}

auto
zpt::net::transport::file::send_reply(zpt::exchange& _channel) -> void {
    this->send_request(_channel);
}

auto
zpt::net::transport::file::send_request(zpt::exchange& _channel) -> void {
    if (_channel->to_send()->ok()) {
    }
}

auto
zpt::net::transport::file::receive_reply(zpt::exchange& _channel) -> void {
    zpt::json _content;
    try {
        _channel->stream() >> _content;
        _channel->received() = _content;
    }
    catch (...) {
    }
    _channel->keep_alive() = true;
}

auto
zpt::net::transport::file::resolve(zpt::json _uri) -> zpt::exchange {
    expect(_uri["scheme"]->ok(), "URI parameter must contain 'scheme'", 500, 0);
    expect(_uri["scheme"] == "file", "scheme must be 'file'", 500, 0);
    std::string _path{ zpt::path::join(_uri["path"]) };
    auto _stream = zpt::stream::alloc<std::basic_fstream<char>>(_path);
    zpt::exchange _to_return{ _stream.release() };
    _to_return //
      ->scheme()
      .assign("file");
    _to_return //
      ->uri()
      .assign(_path);
    _to_return->keep_alive() = true;
    if (_uri["scheme_options"]->ok()) {
        _to_return->options() << "scheme" << _uri["scheme_options"];
    }
    return _to_return;
}
