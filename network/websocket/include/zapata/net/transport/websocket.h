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
 * @file websocket.h
 * @brief WebSocket transport implementation.
 *
 * Provides WebSocket (RFC 6455) protocol support for real-time
 * bidirectional communication over persistent connections.
 *
 * Capabilities: SYNCHRONOUS | PERSISTENT
 *
 * @see zpt::net::transport::websocket
 */

#pragma once
#include <string>
#include <utility>
#include <zapata/net/socket/socket_stream.h>
#include <zapata/streams.h>
#include <zapata/transport.h>

namespace zpt {
/**
 * @brief WebSocket message implementation.
 *
 * Stores message data as JSON internally and sends the content as JSON, framed for the WebSocket
 * protocol.
 */
class ws_message : public json_message {
  public:
    using zpt::json_message::json_message;
    /** @brief Destructor. */
    virtual ~ws_message() = default;

    /** @brief Serializes message to output stream as JSON framed for WebSocket protocol. */
    auto to_stream(std::ostream& _out) const -> zpt::basic_message const& override;
    /** @brief Deserializes message from input stream. */
    auto from_stream(std::istream& _in) -> zpt::basic_message& override;
};

namespace net {
namespace ws {
/** @brief Reads a WebSocket frame, returns (payload, opcode). */
auto read(std::istream& _stream) -> std::tuple<std::string, int>;
/** @brief Writes data as a WebSocket text frame. */
auto write(std::ostream& _stream, std::string const& _in, bool _mask = false) -> void;
} // namespace ws
namespace transport {
/**
 * @brief WebSocket transport implementation.
 *
 * Supports bidirectional messaging over persistent connections.
 * Registered for "ws" and "wss" URI schemes.
 */
class websocket : public zpt::basic_transport {
  public:
    websocket() = default;
    virtual ~websocket() = default;

    /** @brief Returns true for SYNCHRONOUS capability. */
    auto has_capability(std::uint64_t _capability) const -> bool override;
    /** @brief Creates a new request message with JSON payload. */
    auto make_request() const -> zpt::message override;
    /** @brief Creates a new reply message, optionally using the allocator. */
    auto make_reply(bool _with_allocator = true) const -> zpt::message override;
    /** @brief Creates a reply message derived from the given request. */
    auto make_reply(zpt::message _request) const -> zpt::message override;
    /** @brief Parses an incoming request message from the WebSocket stream. */
    auto process_incoming_request(zpt::stream _stream) const -> zpt::message override;
    /** @brief Parses an incoming reply message from the WebSocket stream. */
    auto process_incoming_reply(zpt::stream _stream) const -> zpt::message override;
};
} // namespace transport
} // namespace net

/**
 * @brief Returns the global WebSocket server socket.
 * @param _port Port to bind (0 for configured default).
 */
auto WEBSOCKET_SERVER_SOCKET(std::string const& _address = "0.0.0.0", std::uint16_t _port = 0)
  -> zpt::serversocketstream&;
} // namespace zpt
