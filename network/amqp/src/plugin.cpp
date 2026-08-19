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
 * @file plugin.cpp
 * @brief AMQP transport plugin registration.
 *
 * Registers the "amqp" transport and optionally starts a background
 * connection loop if AMQP broker address and port are configured.
 */

#include <iostream>
#include <mosquitto.h>
#include <zapata/net/socket.h>
#include <zapata/net/transport/amqp.h>
#include <zapata/net/transport/self.h>
#include <zapata/runtime.h>
#include <zapata/startup.h>
#include <zapata/transport.h>

/** @brief System event that subscribes to AMQP topics at boot time.
 *
 * Reads the "amqp.subscribe" array from the global config and subscribes
 * the AMQP stream to each topic.
 */
class plugin_amqp_execute_after_boot : public zpt::system_event {
  public:
    using zpt::system_event::system_event;
    ~plugin_amqp_execute_after_boot() = default;

    /** @brief Executes subscription logic at boot.
     * @param _dispatcher Event dispatcher (unused).
     * @return events::finish */
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state override {
        auto _config = zpt::GLOBAL_CONFIG();
        auto _stream = zpt::AMQP_STREAM();
        auto const& _subscriptions = _config("amqp")("subscribe");

        zpt::SYSTEM_EVENTS_RESOLVER() //
          ->remove<plugin_amqp_execute_after_boot>(zpt::system_event_type::FINISHED_BOOT);

        if (_subscriptions->is_array()) {
            for (auto&& [_, __, _subscription] : _subscriptions) {
                _stream->subscribe(_subscription->string());
            }
        }

        return zpt::events::finish;
    }
};

/** @brief Plugin entry point: registers the AMQP transport and optionally starts a broker
 * connection loop.
 *
 * If the plugin config contains both "port" and "address", a background thread is spawned
 * to maintain a persistent connection to the AMQP broker and process miscellaneous messages.
 * The plugin_amqp_execute_after_boot event is registered to subscribe to configured topics at boot.
 *
 * @param _plugin Plugin handle providing configuration via config(). */
extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    auto& _config = _plugin.config();

    zpt::TRANSPORT_LAYER() //
      .add("amqp", zpt::make_transport<zpt::net::transport::amqp>());

    if (_config("port")->is_integer() && _config("address")->is_string()) {
        zlog("AMQP listener bound to `" << _config("address")->string() << ":"
                                        << _config("port")->integer() << "`",
             zpt::trace);

        zpt::SYSTEM_EVENTS_RESOLVER() //
          ->add<plugin_amqp_execute_after_boot>(zpt::system_event_type::FINISHED_BOOT);

        _plugin.add_thread([=]() mutable -> void {
            zpt::this_thread::name("amqp@loop-misc");
            auto _stream = zpt::AMQP_STREAM(_config);

            zlog("Started AMQP transport connected to " << _config("address")->string() << ":"
                                                        << _config("port")->integer(),
                 zpt::info);

            while (!zpt::runtime::is_in_shutdown()) {
                try {
                    if (!_stream->is_connected()) { _stream->connect(); }
                    _stream->loop_misc();
                }
                catch (zpt::failed_expectation const& _e) {
                    zlog(_e.what(), zpt::error);
                }
                catch (zpt::ClosedException const& _e) {
                }
                catch (std::exception const& _e) {
                    zlog(_e.what(), zpt::error);
                }
                std::this_thread::sleep_for(std::chrono::seconds{ 1 });
            }
            zlog("Stopped AMQP transport connected to " << _config("address")->string() << ":"
                                                        << _config("port")->integer(),
                 zpt::info);
        });
    }
    else { zlog("Loaded AMQP transport", zpt::info); }
}

/** @brief Plugin exit point: unregisters the AMQP transport.
 * @param _plugin Plugin handle (unused). */
extern "C" auto _zpt_unload_(zpt::plugin& _plugin) {
    auto& _config = _plugin.config();
    zpt::TRANSPORT_LAYER().remove("amqp");
    if (!_config("port")->is_integer() || !_config("address")->is_string()) {
        zlog("Unloading AMQP transport", zpt::info);
    }
}
