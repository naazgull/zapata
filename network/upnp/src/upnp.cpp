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

#include <zapata/upnp/UPNPObj.h>
#include <zapata/net/transport/upnp.h>
#include <zapata/base.h>
#include <zapata/uri/uri.h>
#include <zapata/net/socket/socket_stream.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

auto zpt::net::transport::upnp::is_synchronous() const -> bool { return false; }

auto zpt::net::transport::upnp::make_request() const -> zpt::message {
    auto _to_return = zpt::allocate_message<zpt::upnp::basic_request>();
    zpt::init(
      static_cast<zpt::http::basic_request&>(message_cast<zpt::upnp::basic_request&>(_to_return)));
    return _to_return;
}

auto zpt::net::transport::upnp::make_reply(bool _with_allocator) const -> zpt::message {
    auto _to_return = _with_allocator ? zpt::allocate_message<zpt::upnp::basic_reply>()
                                      : zpt::make_message<zpt::upnp::basic_reply>();
    zpt::init(
      static_cast<zpt::http::basic_reply&>(message_cast<zpt::upnp::basic_reply&>(_to_return)));
    return _to_return;
}

auto zpt::net::transport::upnp::make_reply(zpt::message _request) const -> zpt::message {
    auto _to_return = zpt::allocate_message<zpt::upnp::basic_reply>(
      message_cast<zpt::upnp::basic_request>(_request), true);
    zpt::init(
      static_cast<zpt::http::basic_reply&>(message_cast<zpt::upnp::basic_reply&>(_to_return)));
    return _to_return;
}

auto zpt::net::transport::upnp::process_incoming_request(zpt::stream _stream) const
  -> zpt::message {
    expect(_stream->transport() == "upnp", "Stream underlying transport isn't 'upnp'");
    auto _request = zpt::allocate_message<zpt::upnp::basic_request>();
    (*_stream) >> std::noskipws >> _request;
    _request->uri()["domain"] = _request->headers()["Host"];
    return _request;
}

auto zpt::net::transport::upnp::process_incoming_reply(zpt::stream _stream) const -> zpt::message {
    expect(_stream->transport() == "upnp", "Stream underlying transport isn't 'upnp'");
    auto _reply = zpt::allocate_message<zpt::upnp::basic_reply>();
    (*_stream) >> std::noskipws >> _reply;
    return _reply;
}
