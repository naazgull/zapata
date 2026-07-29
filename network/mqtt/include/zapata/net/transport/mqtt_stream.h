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
    auto close() -> mqtt_stream& override;
    auto shutdown() -> mqtt_stream& override;
    /** @brief Reads content from the internal buffer without I/O. */
    auto read_without_io(std::any& _out) -> mqtt_stream& override;
    /** @brief Writes content to the internal buffer without I/O. */
    auto write_without_io(std::any const& _in) -> mqtt_stream& override;
    auto has_next() const -> bool override;
    auto persistent() -> bool override;
    auto connect() -> mqtt_stream&;
    auto is_connected() const -> bool;
    auto subscribe(std::string const& _topic) -> mqtt_stream&;
    auto publish(zpt::message _payload) -> mqtt_stream&;
    auto loop_misc() -> mqtt_stream&;

  private:
    struct mosquitto* __mosq{ nullptr };
    zpt::locks::spin_mutex __mosq_mutex;
    zpt::json __config;
    zpt::padded_atomic<bool> __connected{ false };
    std::set<std::string> __subscriptions;
    std::vector<zpt::message> __buffer;

    auto credentials(std::string const& _user, std::string const& _passwd) -> void;
    auto send_subscribe(std::string const& _topic) -> mqtt_stream&;
    static auto on_connect(struct mosquitto* _mosq, void* _ptr, int _rc) -> void;
    static auto on_message(struct mosquitto* _mosq,
                           void* _ptr,
                           const struct mosquitto_message* _message) -> void;
    static auto on_log(struct mosquitto* _mosq, void* _ptr, int _level, const char* _message)
      -> void;
};
} // namespace zpt
