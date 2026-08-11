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

#include <proton/connection.h>
#include <proton/connection_driver.h>
#include <proton/delivery.h>
#include <proton/link.h>
#include <proton/message.h>
#include <proton/sasl.h>
#include <proton/session.h>
#include <proton/transport.h>
#include <set>
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
class amqp_stream : public basic_stream {
  public:
    using ptr = std::shared_ptr<amqp_stream>;

    amqp_stream(zpt::json _config);
    amqp_stream(amqp_stream const& _rhs) = delete;
    amqp_stream(amqp_stream&& _rhs) = delete;
    ~amqp_stream() override = default;

    auto operator=(amqp_stream const& _rhs) -> amqp_stream& = delete;
    auto operator=(amqp_stream&& _rhs) -> amqp_stream& = delete;

    /**
     * @brief Sets the file descriptor.
     * @param _rhs The file descriptor value to assign.
     * @return Reference to this stream.
     */
    auto operator=(int _rhs) -> amqp_stream&;
    /**
     * @brief Applies a stream manipulator (e.g., std::flush).
     * @param _in The stream manipulator to apply.
     * @return Reference to this stream.
     */
    auto operator<<(ostream_manipulator _in) -> amqp_stream&;
    /** @brief Closes the AMQP connection, releasing all resources.

     Unregisters subscriptions, closes the proton connection, and resets
     all internal state.
     @return Reference to this stream.
     @throws ClosedException if the connection is not active.
     */
    auto close() -> amqp_stream& override;
    /** @brief Shuts down the AMQP connection gracefully.

     Sends close frames and releases resources without reconnecting.
     @return Reference to this stream.
     @throws ClosedException if the connection is not active.
     */
    auto shutdown() -> amqp_stream& override;
    /**
     * @brief Reads content from the internal buffer without I/O.
     * @param _out Reference to receive the deserialized content.
     * @return Reference to this stream.
     */
    auto read_without_io(std::any& _out) -> amqp_stream& override;
    /**
     * @brief Writes content to the internal buffer without I/O.
     * @param _in The content to write to the internal buffer.
     * @return Reference to this stream.
     */
    auto write_without_io(std::any const& _in) -> amqp_stream& override;
    /**
     * @brief Returns true if there are buffered messages waiting to be read.
     * @return True if there are buffered messages available.
     */
    auto has_next() const -> bool override;
    /**
     * @brief Returns whether the stream maintains a persistent connection.
     * @return True if the stream is persistent.
     */
    auto persistent() -> bool override;
    /** @brief Establishes a connection to the AMQP broker using configuration.

     Resolves the address, opens a TCP socket, initializes the proton connection
     driver, creates sender/receiver links, and processes initial events.
     @return Reference to this stream.
     @throws zpt::failed_expectation if connection fails or driver initialization errors.
     */
    auto connect() -> amqp_stream&;
    /**
     * @brief Returns whether the stream is currently connected to the broker.
     * @return True if connected to the broker, false otherwise.
     */
    auto is_connected() const -> bool;
    /** @brief Subscribes to an AMQP topic/queue.

     Creates a receiver link for the topic if already connected.
     @param _topic The topic or queue name to subscribe to.
     @return Reference to this stream.
     @throws ClosedException if not connected.
     */
    auto subscribe(std::string const& _topic) -> amqp_stream&;
    /** @brief Publishes a message to the topic specified in the message URI.

     Encodes the message as AMQP format and sends it via the sender link.
     @param _payload The message to publish.
     @return Reference to this stream.
     @throws ClosedException if not connected.
     */
    auto publish(zpt::message _payload) -> amqp_stream&;
    /** @brief Processes pending I/O events without blocking.

     Reads incoming data, dispatches proton events, and writes outgoing data.
     @return Reference to this stream.
     @throws ClosedException if not connected.
     */
    auto loop_misc() -> amqp_stream&;

  private:
    /** @brief Proton connection driver managing the AMQP connection state. */
    std::unique_ptr<pn_connection_driver_t> __driver{ nullptr };
    /** @brief Mutex protecting access to the connection driver. */
    zpt::locks::spin_mutex __driver_mutex;
    /** @brief Configuration used to establish and manage the connection. */
    zpt::json __config;
    /** @brief Atomic flag indicating whether the connection is active. */
    zpt::padded_atomic<bool> __connected{ false };
    /** @brief Proton sender link for publishing messages. */
    pn_link_t* __sender{ nullptr };
    /** @brief Proton receiver link for subscribing to topics. */
    pn_link_t* __receiver{ nullptr };
    /** @brief Set of topics currently subscribed to. */
    std::set<std::string> __subscriptions;
    /** @brief Buffer of incoming messages waiting to be read. */
    std::vector<zpt::message> __buffer;
    /** @brief Monotonically increasing tag for tracking deliveries. */
    std::uint64_t __delivery_tag{ 0 };

    /** @brief Sets SASL credentials on the connection.
     * @param _user Username for SASL authentication.
     * @param _passwd Password for SASL authentication.
     * @return void */
    auto credentials(std::string const& _user, std::string const& _passwd) -> void;
    /** @brief Sends a subscription request for a topic via proton.
     * @param _topic The topic or queue name to subscribe to.
     * @return Reference to this stream. */
    auto send_subscribe(std::string const& _topic) -> amqp_stream&;
    /** @brief Performs one cycle of socket I/O and proton event processing.
     * @return void */
    auto pump_io() -> void;
    /** @brief Dispatches proton events and handles state transitions.
     * @return void */
    auto process_events() -> void;
    /** @brief Handles an incoming delivery event, decoding and buffering the message.
     * @param _event Pointer to the proton delivery event.
     * @return void */
    auto on_delivery(pn_event_t* _event) -> void;
    /** @brief Routes proton log messages through zapata's logging system.
     * @param _message Log message text.
     * @param _level Log severity level.
     * @return void */
    auto on_log(std::string const& _message, zpt::LogLevel _level) -> void;
};
} // namespace zpt
