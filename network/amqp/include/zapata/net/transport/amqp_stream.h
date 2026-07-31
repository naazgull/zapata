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

    /** @brief Sets the file descriptor. */
    auto operator=(int _rhs) -> amqp_stream&;
    /** @brief Applies a stream manipulator (e.g., std::flush). */
    auto operator<<(ostream_manipulator _in) -> amqp_stream&;
    auto close() -> amqp_stream& override;
    auto shutdown() -> amqp_stream& override;
    /** @brief Reads content from the internal buffer without I/O. */
    auto read_without_io(std::any& _out) -> amqp_stream& override;
    /** @brief Writes content to the internal buffer without I/O. */
    auto write_without_io(std::any const& _in) -> amqp_stream& override;
    auto has_next() const -> bool override;
    auto persistent() -> bool override;
    auto connect() -> amqp_stream&;
    auto is_connected() const -> bool;
    auto subscribe(std::string const& _topic) -> amqp_stream&;
    auto publish(zpt::message _payload) -> amqp_stream&;
    auto loop_misc() -> amqp_stream&;

  private:
    std::unique_ptr<pn_connection_driver_t> __driver{ nullptr };
    zpt::locks::spin_mutex __driver_mutex;
    zpt::json __config;
    zpt::padded_atomic<bool> __connected{ false };
    pn_link_t* __sender{ nullptr };
    pn_link_t* __receiver{ nullptr };
    std::set<std::string> __subscriptions;
    std::vector<zpt::message> __buffer;
    std::uint64_t __delivery_tag{ 0 };

    auto credentials(std::string const& _user, std::string const& _passwd) -> void;
    auto send_subscribe(std::string const& _topic) -> amqp_stream&;
    auto pump_io() -> void;
    auto process_events() -> void;
    auto on_delivery(pn_event_t* _event) -> void;
    auto on_log(std::string const& _message, zpt::LogLevel _level) -> void;
};
} // namespace zpt
