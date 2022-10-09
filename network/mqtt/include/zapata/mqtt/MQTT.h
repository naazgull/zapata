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

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mosquitto.h>
#include <mutex>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>
#include <zapata/events.h>
#include <zapata/json.h>

#define MQTT_RAW -6

namespace zpt {
class MQTT;

/**
 * Data structure that will hold the data for callbacks.
 * Attributes will be instantiated according to the callback type being
 * registered.
 * For more info, see libmosquitto documentation on setting callbacks
 * (http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_connect_callback_set)
 */
typedef struct MQTTDataStruct {
    int __rc = 0;
    int __mid = 0;
    zpt::json __topic;
    zpt::json __message;
    int __qos_count = 0;
    const int* __granted_qos = nullptr;
} MQTTData;

namespace mqtt {

typedef std::shared_ptr<zpt::MQTT> broker;
/**
 * Smart pointer to the callback data.
 */
typedef std::shared_ptr<zpt::MQTTData> data;
/**
 * Lambda style callback definition.
 */
typedef std::function<void(zpt::mqtt::data _data, zpt::mqtt::broker _mqtt)> handler;
/**
 * Map for holding the differente callbacks registered for each different event
 * type.
 */
typedef std::map<std::string, std::vector<zpt::mqtt::handler>> handlers;
} // namespace mqtt

// class MQTTPtr : public std::shared_ptr<zpt::MQTT> {
// public:
// 	MQTTPtr();
// 	MQTTPtr(zpt::MQTT*);
// 	virtual ~MQTTPtr();
// };

// typedef std::shared_ptr<zpt::MQTT> MQTTPtr;

class MQTT : public zpt::Channel {
  public:
    /**
     * Init mosquitto.
     * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_lib_init
     * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_new
     */
    MQTT();
    /**
     * Destroy and clean up.
     * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_destroy
     * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_lib_cleanup
     */
    virtual ~MQTT();

    auto unbind() -> void;

    /**
     * Sets MQTT server access credentials.
     * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_username_pw_set
     */
    virtual auto credentials(std::string const& _user, std::string const& _passwd) -> void;

    virtual auto user() -> std::string;
    virtual auto passwd() -> std::string;
    virtual auto self() const -> zpt::mqtt::broker;
    virtual auto connected() -> bool;

    /**
     * Connects to the MQTT server.
     * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_connect
     */
    virtual auto connect(zpt::json _options) -> bool;
    virtual auto connect(std::string const& _host, bool _tls = false, int _port = 1883, int _keep_alive = 25) -> bool;
    virtual auto reconnect() -> bool;

    /**
     * Subscribes to a given topic. See also http://mosquitto.org/man/mqtt-7.html
     * for topic
     * subscription patterns.
     * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_subscribe
     */
    virtual auto subscribe(std::string const& _topic) -> void;

    /**
     * Publishes a message to a given topic. See also
     * http://mosquitto.org/man/mqtt-7.html for topic
     * subscription
     * patterns.
     * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_publish
     */
    virtual auto publish(std::string const& _topic, zpt::json _payload) -> void;

    /**
     * Add to the callback list, the callback *_callback*, attached to the event
     * type *_event*.
     */
    virtual auto on(std::string const& _event, zpt::mqtt::handler _callback) -> void;

    /**
     * Remove from the callback list, the callback *_callback*, attached to the
     * event type *_event*.
     */
    virtual auto off(std::string const& _event) -> void;

    /**
     * Searches for and executes registered callbacks under the event type
     * *_event*.
     */
    virtual auto trigger(std::string const& _event, zpt::mqtt::data _data) -> void;

    /**
     * Checks if some data is available from MQTT server.
     * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_loop
     */
    // virtual auto start() -> void;
    // virtual auto loop() -> void;

    virtual auto id() -> std::string;
    virtual auto uri(size_t _idx) -> zpt::json;
    virtual auto uri(std::string const& _uris) -> void;
    virtual auto detach() -> void;
    virtual auto close() -> void;
    virtual auto available() -> bool;
    virtual auto buffer(zpt::json _envelope) -> void;
    virtual auto recv() -> zpt::json;
    virtual auto send(zpt::performative _performative, std::string const& _resource, zpt::json _payload) -> zpt::json;
    virtual auto send(zpt::json _envelope) -> zpt::json;
    virtual auto loop_iteration() -> void;
    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;
    virtual auto is_reusable() -> bool;

  private:
    static auto on_connect(struct mosquitto* _mosq, void* _ptr, int _rc) -> void;
    static auto on_disconnect(struct mosquitto* _mosq, void* _ptr, int _reason) -> void;
    static auto on_publish(struct mosquitto* _mosq, void* _ptr, int _mid) -> void;
    static auto on_message(struct mosquitto* _mosq, void* _ptr, const struct mosquitto_message* _message) -> void;
    static auto on_subscribe(struct mosquitto* _mosq, void* _ptr, int _mid, int _qos_count, const int* _granted_qos)
      -> void;
    static auto on_unsubscribe(struct mosquitto* _mosq, void* _ptr, int _mid) -> void;
    static auto on_error(struct mosquitto* _mosq, void* _ptr) -> void;
    static auto on_log(struct mosquitto* _mosq, void* _ptr, int _level, const char* _message) -> void;

    struct mosquitto* __mosq;
    zpt::mqtt::handlers __callbacks;
    std::string __user;
    std::string __passwd;
    std::mutex __mtx_conn;
    std::mutex __mtx_callbacks;
    zpt::mqtt::broker __self;
    bool __connected;
    zpt::json __postponed;
    zpt::json __buffer;
};
} // namespace zpt
