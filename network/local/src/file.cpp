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

auto zpt::net::transport::file::make_request() const -> zpt::message {
    auto _to_return = zpt::allocate_message<zpt::json_message>();
    return _to_return;
}

auto zpt::net::transport::file::make_reply(bool _with_allocator) const -> zpt::message {
    auto _to_return = _with_allocator ? zpt::allocate_message<zpt::json_message>()
                                      : zpt::make_message<zpt::json_message>();
    return _to_return;
}

auto zpt::net::transport::file::make_reply(zpt::message _request) const -> zpt::message {
    auto _to_return =
      zpt::make_message<zpt::json_message>(message_cast<zpt::json_message>(_request), true);
    return _to_return;
}

auto zpt::net::transport::file::process_incoming_request(zpt::stream _stream) const
  -> zpt::message {
    expect(_stream->transport() == "file", "Stream underlying transport isn't 'file'");
    auto _message = zpt::allocate_message<zpt::json_message>();
    (*_stream) >> std::noskipws >> _message;
    return _message;
}

auto zpt::net::transport::file::process_incoming_reply(zpt::stream _stream) const -> zpt::message {
    expect(_stream->transport() == "file", "Stream underlying transport isn't 'file'");
    auto _message = zpt::allocate_message<zpt::json_message>();
    (*_stream) >> std::noskipws >> _message;
    return _message;
}
