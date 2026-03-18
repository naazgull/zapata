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
namespace net {
namespace ws {
/** @brief Performs WebSocket handshake on a stream. */
auto handshake(zpt::stream& _stream) -> void;
/** @brief Reads a WebSocket frame, returns (payload, opcode). */
auto read(zpt::stream& _stream) -> std::tuple<std::string, int>;
/** @brief Writes data as a WebSocket text frame. */
auto write(zpt::stream& _stream, std::string const& _in) -> void;
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

    auto has_capability(std::uint64_t _capability) const -> bool override;
    auto make_request() const -> zpt::message override;
    auto make_reply(bool _with_allocator = true) const -> zpt::message override;
    auto make_reply(zpt::message _request) const -> zpt::message override;
    auto process_incoming_request(zpt::stream _stream) const -> zpt::message override;
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
