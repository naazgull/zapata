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

#include <zapata/net/transport/mqtt.h>

namespace {
auto check_error(std::string const& _operation, int _return, int _errno) -> void;
}

zpt::mqtt::server::server()
  : __mosq{ nullptr }
  , __polling{ zpt::STREAM_POLLING() }
  , __transport{ zpt::TRANSPORT_LAYER().get("mqtt") }
  , __connected{ false } {
    // Init mosquitto.
    // http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_lib_init
    // http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_new
    mosquitto_lib_init();
    this->__mosq = mosquitto_new(nullptr, true, this);
    int _protocol = MQTT_PROTOCOL_V5;
    mosquitto_opts_set(this->__mosq, MOSQ_OPT_PROTOCOL_VERSION, &_protocol);

    // Register the delegating callbacks.
    // http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_connect_callback_set
    // http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_message_callback_set
    mosquitto_connect_callback_set(this->__mosq, zpt::mqtt::server::on_connect);
    mosquitto_message_callback_set(this->__mosq, zpt::mqtt::server::on_message);
    // mosquitto_subscribe_callback_set(this->__mosq, zpt::mqtt::server::on_subscribe);
    mosquitto_log_callback_set(this->__mosq, zpt::mqtt::server::on_log);
}

zpt::mqtt::server::~server() {
    // Destroy and clean up.
    // http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_destroy
    // http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_lib_cleanup
    if (this->__mosq != nullptr) {
        mosquitto_destroy(this->__mosq);
        mosquitto_lib_cleanup();
        this->__mosq = nullptr;
    }
}

auto zpt::mqtt::server::is_connected() const -> bool { return this->__connected->load(); }

auto zpt::mqtt::server::credentials(std::string const& _user, std::string const& _passwd) -> void {
    this->__user = _user;
    this->__passwd = _passwd;
    // Sets MQTT server access credentials.
    // http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_username_pw_set
    errno = 0;
    int _ret = mosquitto_username_pw_set(this->__mosq, _user.data(), _passwd.data());
    ::check_error("setting credentials", _ret, errno);
}

auto zpt::mqtt::server::connect(zpt::json _options) -> bool {
    if (_options("user")->is_string() && _options("password")->is_string()) {
        this->credentials(_options("user")->string(), _options("password")->string());
    }

    auto& _host = _options("address")->string();
    auto _tls = _options("ssl")->is_bool() && _options("ssl")->boolean();
    auto _port = _options("port")->integer();
    auto _keep_alive = 1000;
    {
        this->__uri = std::format("{}://{}{}:{}",
                                  _tls ? "mqtts://" : "mqtt://",
                                  this->__user.empty() ? "" : this->__user + "@",
                                  _host,
                                  _port);

        std::unique_lock _guard{ this->__mosq_mutex };
        if (_tls) {
            errno = 0;
            auto _ret = mosquitto_tls_insecure_set(this->__mosq, false);
            ::check_error("setting TLS insecure", _ret, errno);

            errno = 0, _ret = mosquitto_tls_opts_set(this->__mosq, 1, nullptr, nullptr);
            ::check_error("setting TLS", _ret, errno);

            errno = 0, _ret = mosquitto_tls_set(
                         this->__mosq, nullptr, "/usr/lib/ssl/certs/", nullptr, nullptr, nullptr);
            ::check_error("setting TLS certificate directory", _ret, errno);
        }

        // Connects to the MQTT server.
        // http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_connect
        zlog("Going to connect to " << this->__uri, zpt::trace);
        errno = 0;
        auto _ret = mosquitto_connect(this->__mosq, _host.data(), _port, _keep_alive);
        ::check_error("connecting", _ret, errno);
    }
    zlog("Connected to " << this->__uri, zpt::trace);
    return this->__connected->load();
}

auto zpt::mqtt::server::subscribe(std::string const& _topic) -> void {
    if (this->__connected->load()) {
        zlog("Subscribing to topic " << _topic, zpt::trace);
        // Subscribes to a given topic. See also http://mosquitto.org/man/mqtt-7.html for topic
        // subscription patterns.
        // http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_subscribe
        std::unique_lock _guard{ this->__mosq_mutex };
        int _;
        errno = 0;
        auto _ret = mosquitto_subscribe(this->__mosq, &_, _topic.data(), 0);
        ::check_error("subscribing", _ret, errno);
    }
    this->__subscriptions.insert(_topic);
}

auto zpt::mqtt::server::publish(zpt::message _payload) -> void {
    if (this->__connected->load()) {
        _payload->performative(zpt::Inform);
        
        std::ostringstream _oss;
        _payload->to_stream(_oss);
        _oss.flush();
        auto _payload_str = _oss.str();
        auto _topic = _payload->resource()->string();
        // Publishes a message to a given topic. See also http://mosquitto.org/man/mqtt-7.html
        // for topic subscription patterns.
        // http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_publish
        std::unique_lock _guard{ this->__mosq_mutex };
        int _message_id{ 0 };
        errno = 0;
        auto _ret = mosquitto_publish(this->__mosq,
                                      &_message_id,
                                      _topic.data(),
                                      _payload_str.length(),
                                      _payload_str.data(),
                                      0,
                                      false);
        ::check_error("publishing", _ret, errno);
    }
}

auto zpt::mqtt::server::loop() -> void { mosquitto_loop_forever(this->__mosq, -1, 1); }

auto zpt::mqtt::server::on_connect(struct mosquitto*, void* _ptr, int _rc) -> void {
    zpt::mqtt::server::ptr _self = ((zpt::mqtt::server*)_ptr)->shared_from_this();
    _self->__connected->store(_rc == 0);
    std::unique_lock _guard{ _self->__mosq_mutex };
    for (auto& _topic : _self->__subscriptions) {
        errno = 0;
        int _;
        auto _ret = mosquitto_subscribe(_self->__mosq, &_, _topic.data(), 0);
        ::check_error("subscribing", _ret, errno);
    }
}

auto zpt::mqtt::server::on_message(struct mosquitto*,
                                   void* _ptr,
                                   const struct mosquitto_message* _received) -> void {
    zpt::mqtt::server::ptr _self = ((zpt::mqtt::server*)_ptr)->shared_from_this();
    auto _message = zpt::make_message<zpt::json_message>(zpt::json::parse_json_str(std::string{
      static_cast<char*>(_received->payload), static_cast<size_t>(_received->payloadlen) }));
    _message->header("Content-Type", "application/json");
    zlog("Received MQTT messgage " << _message, zpt::trace);

    auto _stream = zpt::allocate_shared<zpt::event_stream>();
    _stream->transport("mqtt");
    _self->__polling->listen_on(_stream);
    _self->__transport->send(_stream, _message);
}

auto zpt::mqtt::server::on_subscribe(struct mosquitto* _mosq,
                                     void* _ptr,
                                     int _mid,
                                     int _qos_count,
                                     const int* _granted_qos) -> void {}

auto zpt::mqtt::server::on_log(struct mosquitto*, void*, int _level, const char* _message) -> void {
    zlog(_message, static_cast<zpt::LogLevel>(_level));
}

namespace {
auto check_error(std::string const& _operation, int _return, int _errno) -> void {
    if (_return == MOSQ_ERR_SUCCESS) { return; }

    std::string _error_description;
    switch (_return) {
        case MOSQ_ERR_ERRNO: _error_description = mosquitto_strerror(_errno); break;
        case MOSQ_ERR_INVAL: _error_description = "input parameters were invalid"; break;
        case MOSQ_ERR_NOMEM: _error_description = "out of memory condition occurred"; break;
        case MOSQ_ERR_NO_CONN: _error_description = "the client isn't connected to a broker"; break;
        case MOSQ_ERR_CONN_LOST:
            _error_description = "the connection to the broker was lost";
            break;
        case MOSQ_ERR_PROTOCOL:
            _error_description = "there is a protocol error communicating with the broker";
            break;
        case MOSQ_ERR_CONN_REFUSED: _error_description = "MOSQ_ERR_CONN_REFUSED"; break;
        case MOSQ_ERR_NOT_FOUND: _error_description = "MOSQ_ERR_NOT_FOUND"; break;
        case MOSQ_ERR_TLS: _error_description = "MOSQ_ERR_TLS"; break;
        case MOSQ_ERR_PAYLOAD_SIZE: _error_description = "MOSQ_ERR_PAYLOAD_SIZE"; break;
        case MOSQ_ERR_NOT_SUPPORTED: _error_description = "MOSQ_ERR_NOT_SUPPORTED"; break;
        case MOSQ_ERR_AUTH: _error_description = "MOSQ_ERR_AUTH"; break;
        case MOSQ_ERR_ACL_DENIED: _error_description = "MOSQ_ERR_ACL_DENIED"; break;
        case MOSQ_ERR_UNKNOWN: _error_description = "MOSQ_ERR_UNKNOWN"; break;
        case MOSQ_ERR_EAI: _error_description = "MOSQ_ERR_EAI"; break;
        case MOSQ_ERR_PROXY: _error_description = "MOSQ_ERR_PROXY"; break;
        default: _error_description = std::string("unknown error: code=") + std::to_string(_return);
    }

    expect(_return == MOSQ_ERR_SUCCESS,
           "Error while " << _operation << ": E(" << _errno << ") " << _error_description);
};
} // namespace
