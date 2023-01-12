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
auto HTTP_SERVER_SOCKET() -> ssize_t&;
namespace net {
namespace transport {
class http : public zpt::basic_transport {
  public:
    http() = default;
    virtual ~http() = default;

    auto make_request() const -> zpt::message override;
    auto make_reply() const -> zpt::message override;
    auto make_reply(zpt::message _request) const -> zpt::message override;
    auto process_incoming_request(zpt::basic_stream& _stream) const -> zpt::message override;
    auto process_incoming_reply(zpt::basic_stream& _stream) const -> zpt::message override;
};
} // namespace transport
} // namespace net
} // namespace zpt
