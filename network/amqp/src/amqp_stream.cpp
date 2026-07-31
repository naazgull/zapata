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

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <zapata/net/transport/amqp.h>
#include <zapata/startup/startup.h>

namespace {
auto check_error(std::string const& _operation, int _errno) -> void;
}

zpt::amqp_stream::amqp_stream(zpt::json _config)
  : __config{ _config } {
    this->__transport = "amqp";
}

auto zpt::amqp_stream::operator=(int) -> zpt::amqp_stream& { return (*this); }

auto zpt::amqp_stream::operator<<(ostream_manipulator) -> zpt::amqp_stream& { return (*this); }

auto zpt::amqp_stream::close() -> zpt::amqp_stream& {
    zlog("Closing connection to " << this->__uri, zpt::trace);

    this->__underlying.reset(nullptr);
    this->__fd = -1;
    this->__transport = "";
    this->__uri = "";
    this->__state = zpt::stream_state::IDLE;

    return (*this);
}

auto zpt::amqp_stream::shutdown() -> zpt::amqp_stream& {
    std::unique_lock _guard{ this->__driver_mutex };
    if (this->__driver != nullptr) {
        pn_connection_driver_close(this->__driver.get());
        pn_connection_driver_destroy(this->__driver.get());
        this->__driver.reset();
    }

    this->__connected->store(false);
    this->__sender = nullptr;
    this->__receiver = nullptr;
    ::shutdown(this->__fd, SHUT_RDWR);
    ::close(this->__fd);
    this->__fd = -1;
    return (*this);
}

auto zpt::amqp_stream::read_without_io(std::any& _out) -> zpt::amqp_stream& {
    std::unique_lock _guard{ this->__driver_mutex };
    if (!this->__connected->load()) { throw zpt::ClosedException("socket has been shutdown"); }

    if (this->__buffer.size() == 0) { this->pump_io(); }

    if (this->__buffer.size() == 0) {
        throw zpt::InterruptedException{ "control message received on AMQP socket" };
    }

    _out = std::make_any<zpt::message>(this->__buffer.back());
    this->__buffer.pop_back();

    return (*this);
}

auto zpt::amqp_stream::write_without_io(std::any const& _in) -> zpt::amqp_stream& {
    return this->publish(std::any_cast<zpt::message>(_in));
}

auto zpt::amqp_stream::has_next() const -> bool { return this->__buffer.size() != 0; }

auto zpt::amqp_stream::persistent() -> bool { return true; }

auto zpt::amqp_stream::connect() -> amqp_stream& {
    auto& _host = this->__config("address")->string();
    auto _port = this->__config("port")->integer();
    this->__uri = std::format("amqp://{}:{}", _host, _port);

    zlog("Going to connect to " << this->__uri, zpt::trace);

    struct addrinfo _hints{};
    _hints.ai_family = AF_INET;
    _hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* _resolved{ nullptr };
    errno = 0;
    ::check_error("resolving address",
                  getaddrinfo(_host.data(), std::to_string(_port).data(), &_hints, &_resolved));

    try {
        std::unique_lock _guard{ this->__driver_mutex };
        expect(this->__driver == nullptr && this->__fd == -1,
               "Connection still active, can't reconnect");

        auto _fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        errno = 0;
        if (::connect(_fd, _resolved->ai_addr, _resolved->ai_addrlen) < 0) {
            ::freeaddrinfo(_resolved);
            ::check_error("connecting", errno);
        }
        ::freeaddrinfo(_resolved);

        this->__fd = _fd;
        this->__driver = std::make_unique<pn_connection_driver_t>();

        errno = 0;
        ::check_error("initializing connection driver",
                      pn_connection_driver_init(this->__driver.get(), nullptr, nullptr));

        if (zpt::log_lvl <= zpt::trace) {
            pn_transport_trace(this->__driver->transport, PN_TRACE_FRM);
            pn_transport_set_tracer(
              this->__driver->transport,
              [](pn_transport_t*, const char* _msg) { zlog(_msg, zpt::trace); });
        }

        auto* _sasl = pn_sasl(this->__driver->transport);
        pn_sasl_set_allow_insecure_mechs(_sasl, true);

        if (this->__config("user")->is_string() && this->__config("password")->is_string()) {
            this->credentials(this->__config("user")->string(),
                              this->__config("password")->string());
        }

        pn_connection_set_container(this->__driver->connection,
                                    zpt::GLOBAL_CONFIG()("identity")("_id")->string().data());
        pn_connection_set_hostname(this->__driver->connection, _host.data());

        auto* _desired = pn_connection_desired_capabilities(this->__driver->connection);
        pn_data_put_array(_desired, false, PN_SYMBOL);
        pn_data_enter(_desired);
        pn_data_put_symbol(_desired, pn_bytes_t{ 15, "ANONYMOUS-RELAY" });
        pn_data_exit(_desired);

        pn_connection_open(this->__driver->connection);

        auto _session = pn_session(this->__driver->connection);
        pn_session_open(_session);

        this->__sender = pn_sender(_session, "sender");
        pn_link_open(this->__sender);

        do { this->pump_io(); } while (!this->__connected->load());

        for (auto& _topic : this->__subscriptions) { this->send_subscribe(_topic); }

        zlog("Connected to " << this->__uri, zpt::trace);
    }
    catch (...) {
        if (this->__driver != nullptr) { this->shutdown(); }
        throw;
    }
    return (*this);
}

auto zpt::amqp_stream::is_connected() const -> bool { return this->__connected->load(); }

auto zpt::amqp_stream::subscribe(std::string const& _topic) -> zpt::amqp_stream& {
    std::unique_lock _guard{ this->__driver_mutex };
    if (!this->__connected->load()) { throw zpt::ClosedException("socket has been shutdown"); }
    this->send_subscribe(_topic);
    this->__subscriptions.insert(_topic);
    return (*this);
}

auto zpt::amqp_stream::publish(zpt::message _payload) -> zpt::amqp_stream& {
    auto _uri = _payload->uri();
    expect(_uri("scheme_options")->is_array(), "URI doesn't contain the publishing target");
    std::ostringstream _queue_oss;
    for (auto&& [_, __, _part] : _uri("scheme_options")) { _queue_oss << "/" << _part->string(); }
    _queue_oss << std::flush;
    auto _topic = _queue_oss.str();

    zlog("Sending message to topic " << _topic, zpt::trace);

    std::ostringstream _oss;
    _payload->to_stream(_oss);
    _oss.flush();
    auto _payload_str = _oss.str();
    std::unique_lock _guard{ this->__driver_mutex };
    if (!this->__connected->load()) { throw zpt::ClosedException("socket has been shutdown"); }

    auto* _msg = pn_message();
    pn_message_set_address(_msg, _topic.data());
    pn_message_set_content_type(_msg, "application/json");
    pn_data_put_string(pn_message_body(_msg),
                       pn_bytes_t{ _payload_str.length(), _payload_str.data() });

    std::vector<char> _encoded(1024);
    size_t _encoded_size = _encoded.size();
    while (pn_message_encode(_msg, _encoded.data(), &_encoded_size) == PN_OVERFLOW) {
        _encoded.resize(_encoded.size() * 2);
        _encoded_size = _encoded.size();
    }
    pn_message_free(_msg);

    auto _tag = std::to_string(++this->__delivery_tag);
    pn_delivery(this->__sender, pn_dtag(_tag.data(), _tag.length()));
    pn_link_send(this->__sender, _encoded.data(), _encoded_size);
    pn_link_advance(this->__sender);

    this->pump_io();

    return (*this);
}

auto zpt::amqp_stream::loop_misc() -> amqp_stream& {
    std::unique_lock _guard{ this->__driver_mutex };
    if (!this->__connected->load()) { throw zpt::ClosedException("socket has been shutdown"); }
    this->pump_io();
    return (*this);
}

auto zpt::amqp_stream::credentials(std::string const& _user, std::string const& _passwd) -> void {
    pn_connection_set_user(this->__driver->connection, _user.data());
    pn_connection_set_password(this->__driver->connection, _passwd.data());
}

auto zpt::amqp_stream::send_subscribe(std::string const& _topic) -> zpt::amqp_stream& {
    if (this->__connected->load()) {
        zlog("Subscribing to topic " << _topic, zpt::trace);
        auto _session = pn_session(this->__driver->connection);
        pn_session_open(_session);

        this->__receiver = pn_receiver(_session, _topic.data());
        pn_terminus_set_address(pn_link_source(this->__receiver), _topic.data());
        pn_link_open(this->__receiver);
        pn_link_flow(this->__receiver, 10);

        this->pump_io();
    }
    return (*this);
}

auto zpt::amqp_stream::pump_io() -> void {
    for (;;) {
        auto _rb = pn_connection_driver_read_buffer(this->__driver.get());
        if (_rb.size == 0) { break; }

        errno = 0;
        auto _n = ::recv(this->__fd, _rb.start, _rb.size, MSG_DONTWAIT);
        if (_n > 0) {
            pn_connection_driver_read_done(this->__driver.get(), _n);
            continue;
        }
        if (_n == 0) {
            pn_connection_driver_read_close(this->__driver.get());
            break;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) { break; }
        ::check_error("reading", errno);
    }

    this->process_events();

    for (;;) {
        auto _wb = pn_connection_driver_write_buffer(this->__driver.get());
        if (_wb.size == 0) { break; }

        errno = 0;
        auto _n = ::send(this->__fd, _wb.start, _wb.size, MSG_NOSIGNAL | MSG_DONTWAIT);
        if (_n > 0) {
            pn_connection_driver_write_done(this->__driver.get(), _n);
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) { break; }
        ::check_error("writing", errno);
    }
}

auto zpt::amqp_stream::process_events() -> void {
    pn_event_t* _event{ nullptr };
    while (pn_connection_driver_has_event(this->__driver.get()) &&
           (_event = pn_connection_driver_next_event(this->__driver.get())) != nullptr) {
        switch (pn_event_type(_event)) {
            case PN_CONNECTION_REMOTE_OPEN: {
                auto* _offered =
                  pn_connection_remote_offered_capabilities(this->__driver->connection);
                bool _has_anonymous_relay{ false };
                pn_data_rewind(_offered);
                while (pn_data_next(_offered)) {
                    auto _symbol = pn_data_get_symbol(_offered);
                    if (std::string_view{ _symbol.start, _symbol.size } == "ANONYMOUS-RELAY") {
                        _has_anonymous_relay = true;
                        break;
                    }
                }
                if (!_has_anonymous_relay) {
                    this->on_log("Broker did not offer ANONYMOUS-RELAY capability; "
                                 "publishing via per-message address will not be routed",
                                 zpt::warning);
                }

                zpt::STREAM_POLLING() //
                  ->listen_on(this->shared_from_this());
                this->__connected->store(true);
                break;
            }
            case PN_LINK_REMOTE_OPEN: {
                auto* _link = pn_event_link(_event);
                if (pn_link_is_receiver(_link)) { pn_link_flow(_link, 10); }
                break;
            }
            case PN_DELIVERY: {
                this->on_delivery(_event);
                break;
            }
            case PN_TRANSPORT_ERROR: {
                auto* _condition = pn_transport_condition(this->__driver->transport);
                this->on_log(pn_condition_get_description(_condition), zpt::error);
                ::shutdown(this->__fd, SHUT_RDWR);
                break;
            }
            case PN_TRANSPORT_CLOSED: {
                ::shutdown(this->__fd, SHUT_RDWR);
                break;
            }
            default: {
                break;
            }
        }
    }
}

auto zpt::amqp_stream::on_delivery(pn_event_t* _event) -> void {
    auto* _delivery = pn_event_delivery(_event);
    auto* _link = pn_delivery_link(_delivery);
    if (!pn_link_is_receiver(_link)) { return; }
    auto* _current = pn_link_current(_link);
    if (_current == nullptr) { return; }
    if (pn_delivery_partial(_current)) { return; }

    auto _pending = pn_delivery_pending(_current);
    std::vector<char> _encoded(_pending);
    auto _n = pn_link_recv(_link, _encoded.data(), _encoded.size());
    if (_n > 0) {
        auto* _msg = pn_message();
        if (pn_message_decode(_msg, _encoded.data(), _n) == 0) {
            auto* _body = pn_message_body(_msg);
            pn_data_next(_body);
            auto _payload = pn_data_get_string(_body);
            auto _message = zpt::make_message<zpt::json_message>(
              zpt::json::parse_json_str(std::string{ _payload.start, _payload.size }));
            _message->header("Content-Type", "application/json");
            this->__buffer.push_back(_message);
        }
        pn_message_free(_msg);
    }

    pn_delivery_update(_current, PN_ACCEPTED);
    pn_delivery_settle(_current);
    pn_link_flow(_link, 1);
}

auto zpt::amqp_stream::on_log(std::string const& _message, zpt::LogLevel _level) -> void {
    zlog(_message, _level);
}

namespace {
auto check_error(std::string const& _operation, int _errno) -> void {
    if (_errno == 0) { return; }
    expect(_errno == 0,
           "Error while " << _operation << ": E(" << _errno << ") " << std::strerror(_errno));
}
} // namespace
