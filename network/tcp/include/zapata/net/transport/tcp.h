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
 * @file tcp.h
 * @brief Raw TCP transport implementation.
 *
 * Provides raw TCP socket communication with JSON message framing.
 * Uses length-prefixed messages for reliable delivery.
 *
 * Capabilities: SYNCHRONOUS | PERSISTENT
 *
 * @see zpt::net::transport::tcp
 */

#pragma once
#include <string>
#include <utility>
#include <zapata/net/socket/socket_stream.h>
#include <zapata/streams.h>
#include <zapata/transport.h>

namespace zpt {
namespace net {
namespace transport {

/**
 * @brief Raw TCP transport with JSON framing.
 *
 * Supports persistent connections with message framing.
 * Registered for "tcp" URI scheme.
 */
class tcp : public zpt::basic_transport {
  public:
    tcp() = default;
    virtual ~tcp() = default;

    /** @brief Returns true for SYNCHRONOUS capability.
     * @param _capability Capability flag to check.
     * @return True if checking for SYNCHRONOUS. */
    auto has_capability(std::uint64_t _capability) const -> bool override;
    /** @brief Creates a new request message with JSON payload.
     * @return Shared pointer to the new request message. */
    auto make_request() const -> zpt::message override;
    /** @brief Creates a new reply message, optionally using the allocator.
     * @param _with_allocator If true, uses the memory pool allocator.
     * @return Shared pointer to the new reply message. */
    auto make_reply(bool _with_allocator = true) const -> zpt::message override;
    /** @brief Creates a reply message derived from the given request.
     * @param _request The request message to reply to.
     * @return Shared pointer to the new reply message. */
    auto make_reply(zpt::message _request) const -> zpt::message override;
    /** @brief Parses an incoming request message from the TCP stream.
     * @param _stream Input stream containing the serialized request.
     * @return Parsed request message. */
    auto process_incoming_request(zpt::stream _stream) const -> zpt::message override;
    /** @brief Parses an incoming reply message from the TCP stream.
     * @param _stream Input stream containing the serialized reply.
     * @return Parsed reply message. */
    auto process_incoming_reply(zpt::stream _stream) const -> zpt::message override;
    /** @brief Creates a copy of the given message with the transport's protocol and format.
     * @param _to_copy The message to copy.
     * @return The copied message. */
    auto copy(zpt::message const& _to_copy) const -> zpt::message override;
};
} // namespace transport
} // namespace net

/**
 * @brief Returns the global TCP server socket.
 * @param _address Address to bind (default: "0.0.0.0").
 * @param _port Port to bind (0 for configured default).
 * @return Reference to the global server socket stream.
 */
auto TCP_SERVER_SOCKET(std::string const& _address = "0.0.0.0", std::uint16_t _port = 0)
  -> zpt::serversocketstream&;
} // namespace zpt
