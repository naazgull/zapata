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

/**
 * @file http.h
 * @brief HTTP transport implementation.
 *
 * Provides HTTP/1.1 protocol support for the transport layer.
 * Handles HTTP request/response parsing and serialization.
 *
 * Capabilities: SYNCHRONOUS
 *
 * @see zpt::net::transport::http
 */

#pragma once
#include <string>
#include <utility>
#include <zapata/http.h>
#include <zapata/net/socket/socket_stream.h>
#include <zapata/streams.h>
#include <zapata/transport.h>

#ifndef CRLF
#define CRLF "\r\n"
#endif

namespace zpt {
namespace net {
namespace transport {

/**
 * @brief HTTP/1.1 transport implementation.
 *
 * Supports standard HTTP request-response communication.
 * Registered for "http" and "https" URI schemes.
 */
class http : public zpt::basic_transport {
  public:
    /** @brief Default constructor. */
    http() = default;
    /** @brief Destructor. */
    virtual ~http() = default;

    /** @brief Returns true for SYNCHRONOUS capability. */
    auto has_capability(std::uint64_t _capability) const -> bool override;
    /** @brief Creates a new HTTP request message. */
    auto make_request() const -> zpt::message override;
    /** @brief Creates a new HTTP reply message. */
    auto make_reply(bool _with_allocator = true) const -> zpt::message override;
    /** @brief Creates an HTTP reply for a given request. */
    auto make_reply(zpt::message _request) const -> zpt::message override;
    /** @brief Parses an incoming HTTP request from a stream. */
    auto process_incoming_request(zpt::stream _stream) const -> zpt::message override;
    /** @brief Parses an incoming HTTP reply from a stream. */
    auto process_incoming_reply(zpt::stream _stream) const -> zpt::message override;
};
} // namespace transport
} // namespace net

/**
 * @brief Returns the global HTTP server socket.
 * @param _port Port to bind (0 for configured default).
 */
auto HTTP_SERVER_SOCKET(std::string const& _address = "0.0.0.0", std::uint16_t _port = 0)
  -> zpt::serversocketstream&;
} // namespace zpt
