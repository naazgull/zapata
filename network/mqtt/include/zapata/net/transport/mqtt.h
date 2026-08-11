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
 * @file mqtt.h
 * @brief MQTT transport implementation.
 *
 * Provides MQTT socket communication with JSON message framing.
 * Uses length-prefixed messages for reliable delivery.
 *
 * Capabilities: SYNCHRONOUS | PERSISTENT
 *
 * @see zpt::net::transport::mqtt
 */

#pragma once
#include <mosquitto.h>
#include <set>
#include <string>
#include <utility>
#include <zapata/net/transport/mqtt_stream.h>
#include <zapata/streams.h>
#include <zapata/transport.h>

namespace zpt {
namespace net {
namespace transport {

/**
 * @brief MQTT transport with JSON framing.
 *
 * Supports persistent connections with message framing.
 * Registered for "mqtt" URI scheme.
 */
class mqtt : public zpt::basic_transport {
  public:
    /** @brief Default constructor.
     * @return void (constructors implicitly initialize the object) */
    mqtt() = default;
    /** @brief Destructor.
     * @return void (destructors implicitly clean up the object) */
    virtual ~mqtt() = default;

    /** @brief Returns whether the transport has the requested capability.
     * @param _capability Capability flag to check.
     * @return True if the transport has the requested capability. */
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
    /** @brief Processes an incoming request message from the given stream.
     * @param _stream Input stream containing the serialized request.
     * @return Parsed request message. */
    auto process_incoming_request(zpt::stream _stream) const -> zpt::message override;
    /** @brief Processes an incoming reply message from the given stream.
     * @param _stream Input stream containing the serialized reply.
     * @return Parsed reply message. */
    auto process_incoming_reply(zpt::stream _stream) const -> zpt::message override;
    /** @brief Publishes a message to the MQTT broker for distribution.
     * @param _to_publish Message to publish.
     * @return void */
    auto publish(zpt::message _to_publish) const -> void override;
};
} // namespace transport
} // namespace net

/** @brief Gets the MQTT stream singleton.

Creates or returns a shared MQTT stream using global configuration.
@param _config Optional config override; uses global config if undefined.
@return A shared pointer to the MQTT stream singleton.
*/
auto MQTT_STREAM(zpt::json _config = zpt::undefined) -> zpt::mqtt_stream::ptr;
} // namespace zpt
