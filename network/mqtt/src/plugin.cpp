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

#include <iostream>
#include <mosquitto.h>
#include <zapata/net/socket.h>
#include <zapata/net/transport/mqtt.h>
#include <zapata/startup.h>
#include <zapata/transport.h>

// auto _polling = zpt::STREAM_POLLING();
// auto _transport = zpt::TRANSPORT_LAYER().get("mqtt");

// mosquitto_lib_init();
// auto _mosq = mosquitto_new(nullptr, true, this);
// int _protocol = MQTT_PROTOCOL_V311;
// mosquitto_opts_set(this->__mosq, MOSQ_OPT_PROTOCOL_VERSION, &_protocol);

extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    auto& _config = _plugin.config();

    zpt::TRANSPORT_LAYER() //
      .add("mqtt", zpt::make_transport<zpt::net::transport::mqtt>());

    if (_config("port")->is_integer() && _config("address")->is_string()) {
        zlog("MQTT listener bound to `" << _config("address")->string() << ":"
                                        << _config("port")->integer() << "`",
             zpt::trace);

        _plugin.add_thread([=]() mutable -> void {
            zpt::this_thread::name("mqtt@listener");
            // auto _server = zpt::MQTT_SERVER();
            auto _polling = zpt::STREAM_POLLING();

            // _server->connect(_config("address")->string(), _config("port")->integer());
            zlog("Started MQTT transport connected to " << _config("address")->string() << ":"
                                                        << _config("port")->integer(),
                 zpt::info);

            try {
                do {
                    // _server->receive();
                } while (!_polling->is_in_shutdown());
            }
            catch (zpt::failed_expectation const& _e) {
                zlog(_e.what(), zpt::error);
            }
            catch (zpt::ClosedException const& _e) {
            }
            catch (std::exception const& _e) {
                zlog(_e.what(), zpt::error);
            }
            zlog("Stopped MQTT transport connected to " << _config("address")->string() << ":"
                                                        << _config("port")->integer(),
                 zpt::info);
        });
    }
    else { zlog("Loaded MQTT transport", zpt::info); }
}

extern "C" auto _zpt_unload_(zpt::plugin& _plugin) {
    auto& _config = _plugin.config();
    zpt::TRANSPORT_LAYER().remove("mqtt");
    if (!_config("port")->is_integer() || !_config("address")->is_string()) {
        zlog("Unloading MQTT transport", zpt::info);
    }
}
