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
 * @file amqp.h
 * @brief AMQP transport implementation.
 *
 * Provides AMQP socket communication with JSON message framing.
 * Uses length-prefixed messages for reliable delivery.
 *
 * Capabilities: SYNCHRONOUS | PERSISTENT
 *
 * @see zpt::net::transport::amqp
 */

#pragma once
#include <mosquitto.h>
#include <set>
#include <string>
#include <utility>
#include <zapata/net/transport/amqp_stream.h>
#include <zapata/streams.h>
#include <zapata/transport.h>

namespace zpt {
namespace net {
namespace transport {

/**
 * @brief AMQP transport with JSON framing.
 *
 * Supports persistent connections with message framing.
 * Registered for "amqp" URI scheme.
 */
class amqp : public zpt::basic_transport {
  public:
    amqp() = default;
    virtual ~amqp() = default;

    auto has_capability(std::uint64_t _capability) const -> bool override;
    auto make_request() const -> zpt::message override;
    auto make_reply(bool _with_allocator = true) const -> zpt::message override;
    auto make_reply(zpt::message _request) const -> zpt::message override;
    auto process_incoming_request(zpt::stream _stream) const -> zpt::message override;
    auto process_incoming_reply(zpt::stream _stream) const -> zpt::message override;
    auto publish(zpt::message _to_publish) const -> void override;
};
} // namespace transport
} // namespace net
auto AMQP_STREAM(zpt::json _config = zpt::undefined) -> zpt::amqp_stream::ptr;
} // namespace zpt
