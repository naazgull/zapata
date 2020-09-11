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

#pragma once
#include <string>
#include <utility>
#include <zapata/streams.h>
#include <zapata/transport.h>
#include <zapata/http.h>

#ifndef CRLF
#define CRLF "\r\n"
#endif

namespace zpt {
auto
HTTP_SERVER_SOCKET() -> ssize_t&;
namespace net {
namespace transport {
class http : public zpt::transport::transport_t {
  public:
    http() = default;
    virtual ~http() = default;

    auto set_headers(zpt::exchange& _channel, zpt::HTTPObj& _http_channel)
      -> zpt::net::transport::http&;
    auto set_body(zpt::exchange& _channel, zpt::HTTPObj& _http_channel)
      -> zpt::net::transport::http&;
    auto set_params(zpt::exchange& _channel, zpt::http::req& _request)
      -> zpt::net::transport::http&;
    auto set_method(zpt::exchange& _channel, zpt::http::req& _request)
      -> zpt::net::transport::http&;
    auto set_method(zpt::exchange& _channel, zpt::http::rep& _response)
      -> zpt::net::transport::http&;
    auto set_status(zpt::exchange& _channel, zpt::http::rep& _response)
      -> zpt::net::transport::http&;
    auto set_headers(zpt::http::req& _request, zpt::exchange& _channel)
      -> zpt::net::transport::http&;
    auto set_headers(zpt::http::rep& _response, zpt::exchange& _channel)
      -> zpt::net::transport::http&;
    auto set_body(zpt::http::req& _request, zpt::exchange& _channel) -> zpt::net::transport::http&;
    auto set_body(zpt::http::rep& _response, zpt::exchange& _channel) -> zpt::net::transport::http&;
    auto set_method(zpt::http::req& _request, zpt::exchange& _channel)
      -> zpt::net::transport::http&;
    auto set_status(zpt::http::rep& _response, zpt::exchange& _channel)
      -> zpt::net::transport::http&;

    auto receive(zpt::exchange& _channel) -> void override;
    auto send(zpt::exchange& _channel) -> void override;
    auto resolve(zpt::json _uri) -> zpt::exchange override;
    auto receive_request(zpt::exchange& _channel) -> void;
    auto receive_reply(zpt::exchange& _channel) -> void;
    auto send_request(zpt::exchange& _channel) -> void;
    auto send_reply(zpt::exchange& _channel) -> void;
};
} // namespace transport
} // namespace net
} // namespace zpt
