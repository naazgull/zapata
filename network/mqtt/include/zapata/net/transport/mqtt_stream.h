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

#include <vector>
#include <zapata/base/expect.h>
#include <zapata/exceptions/ClosedException.h>
#include <zapata/json.h>
#include <zapata/log/log.h>
#include <zapata/streams.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>
#include <zapata/transport.h>

namespace zpt {
class mqtt_stream : public basic_stream {
  public:
    using ptr = std::shared_ptr<mqtt_stream>;

    mqtt_stream(zpt::json _config);
    mqtt_stream(mqtt_stream const& _rhs) = delete;
    mqtt_stream(mqtt_stream&& _rhs) = delete;
    ~mqtt_stream() override = default;

    auto operator=(mqtt_stream const& _rhs) -> mqtt_stream& = delete;
    auto operator=(mqtt_stream&& _rhs) -> mqtt_stream& = delete;

    /** @brief Sets the file descriptor. */
    auto operator=(int _rhs) -> mqtt_stream&;
    /** @brief Applies a stream manipulator (e.g., std::flush). */
    auto operator<<(ostream_manipulator _in) -> mqtt_stream&;
    /** @brief Closes the stream and resets its internal state. */
    auto close() -> mqtt_stream& override;
    /** @brief Shuts down the connection and releases mosquitto resources. */
    auto shutdown() -> mqtt_stream& override;
    /** @brief Reads content from the internal buffer without I/O. */
    auto read_without_io(std::any& _out) -> mqtt_stream& override;
    /** @brief Writes content to the internal buffer without I/O. */
    auto write_without_io(std::any const& _in) -> mqtt_stream& override;
    /** @brief Returns true if there are buffered messages waiting to be read. */
    auto has_next() const -> bool override;
    /** @brief Returns whether the stream maintains a persistent connection. */
    auto persistent() -> bool override;
    /** @brief Establishes a connection to the MQTT broker using configuration.

     Initializes mosquitto library, configures TLS if enabled, registers
     callbacks, and connects with optional credentials.
     @return Reference to this stream.
     */
    auto connect() -> mqtt_stream&;
    /** @brief Returns whether the stream is currently connected to the broker. */
    auto is_connected() const -> bool;
    /** @brief Subscribes to an MQTT topic.

     Sends a subscription request to the broker if already connected.
     @param _topic The topic to subscribe to.
     @return Reference to this stream.
     */
    auto subscribe(std::string const& _topic) -> mqtt_stream&;
    /** @brief Publishes a message to an MQTT topic derived from the message structure.

     Converts the message to JSON and publishes it to the topic path
     formed from the performative and resource.
     @param _payload The message to publish.
     @return Reference to this stream.
     @throws ClosedException if not connected.
     */
    auto publish(zpt::message _payload) -> mqtt_stream&;
    /** @brief Processes pending mosquitto I/O events without blocking.

     Calls mosquitto_loop_misc to handle internal state transitions.
     @return Reference to this stream.
     @throws ClosedException if not connected.
     */
    auto loop_misc() -> mqtt_stream&;

  private:
    /** @brief Mosquitto client handle managing the MQTT connection. */
    struct mosquitto* __mosq{ nullptr };
    /** @brief Mutex protecting access to the mosquitto client. */
    zpt::locks::spin_mutex __mosq_mutex;
    /** @brief Configuration used to establish and manage the connection. */
    zpt::json __config;
    /** @brief Atomic flag indicating whether the connection is active. */
    zpt::padded_atomic<bool> __connected{ false };
    /** @brief Set of topics currently subscribed to. */
    std::set<std::string> __subscriptions;
    /** @brief Buffer of incoming messages waiting to be read. */
    std::vector<zpt::message> __buffer;

    /** @brief Sets MQTT client access credentials. */
    auto credentials(std::string const& _user, std::string const& _passwd) -> void;
    /** @brief Sends a subscription request to the broker for a topic. */
    auto send_subscribe(std::string const& _topic) -> mqtt_stream&;
    /** @brief Static callback invoked when the MQTT connection is established.

     Records the connection state, socket FD, and resends pending subscriptions.
     */
    static auto on_connect(struct mosquitto* _mosq, void* _ptr, int _rc) -> void;
    /** @brief Static callback invoked when a message is received from the broker.

     Parses the JSON payload and adds it to the message buffer.
     */
    static auto on_message(struct mosquitto* _mosq,
                           void* _ptr,
                           const struct mosquitto_message* _message) -> void;
    /** @brief Static callback routing mosquitto log messages through zapata's logger. */
    static auto on_log(struct mosquitto* _mosq, void* _ptr, int _level, const char* _message)
      -> void;
};
} // namespace zpt
