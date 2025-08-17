#pragma once

#include <zapata/events.h>
#include <zapata/streams.h>
#include <zapata/transport.h>
#include <zapata/net/socket.h>
#include <list>

namespace zpt {
namespace events {
using initializer_t = std::function<void(zpt::event _event)>;
using resolver_callback = std::function<zpt::event(zpt::message, zpt::events::initializer_t)>;
class resolver_t {
  public:
    resolver_t() = default;
    virtual ~resolver_t() = default;

    virtual auto add(zpt::message _sent,
                     zpt::events::resolver_callback callback) -> resolver_t& = 0;
    virtual auto add(zpt::performative _performtive,
                     std::string const& _path,
                     zpt::json const& _metadata,
                     zpt::events::resolver_callback _callback) -> resolver_t& = 0;
    virtual auto add(zpt::json const& _service_description) -> resolver_t& = 0;
    virtual auto remove(zpt::message _sent) -> resolver_t& = 0;
    virtual auto remove(zpt::performative _performtive,
                        std::string const& _path) -> resolver_t& = 0;
    virtual auto resolve(zpt::message _received,
                         initializer_t _initializer) const -> std::list<zpt::event> = 0;
    virtual auto search(std::string const& _path,
                        std::string const& _provider_id = "") const -> zpt::json = 0;
    virtual auto list(std::string const& _provider_id = "") const -> zpt::json = 0;
    virtual auto register_provider(zpt::json const& _service_description) -> resolver_t& = 0;
    virtual auto unregister_provider(std::string const& _id) -> resolver_t& = 0;
    virtual auto get_provider(std::string const& _id) const -> zpt::json = 0;
};
using resolver = std::shared_ptr<resolver_t>;
} // namespace events

namespace transports {
class engine {
  public:
    engine(zpt::json _config);
    virtual ~engine() = default;

    auto add_resolver(zpt::events::resolver _resolver) -> engine&;
    auto resolve(zpt::message _received,
                 zpt::events::initializer_t _initializer) const -> std::list<zpt::event>;
    auto shutdown() -> engine&;

  private:
    zpt::json __configuration;
    std::vector<zpt::events::resolver> __resolvers;
    zpt::events::dispatcher::ptr __dispatcher;
};

template<typename T>
auto make_callback(zpt::message _received, zpt::events::initializer_t _initializer) -> zpt::event;
} // namespace transports

namespace events {
class transport_event_init : public zpt::event_initialization {
  public:
    zpt::events::dispatcher::ptr __dispatcher;
    zpt::polling::ptr __polling;
    zpt::stream __stream;
};

class receive {
  public:
    receive(zpt::transports::engine& _engine, zpt::polling::ptr _polling, zpt::stream _stream);
    receive(zpt::events::receive const& _rhs) = delete;
    receive(zpt::events::receive&& _rhs) = delete;
    virtual ~receive();

    auto operator=(zpt::events::receive const& _rhs) -> receive& = delete;
    auto operator=(zpt::events::receive&& _rhs) -> receive& = delete;

    auto initialize(zpt::event_initialization& init) -> void;
    auto blocked() const -> bool;
    auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(zpt::failed_expectation const& _e,
                     zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;

  protected:
    zpt::transports::engine& __engine;
    zpt::polling::ptr __polling;
    zpt::stream __stream;
};

class send {
  public:
    send(zpt::polling::ptr _polling, zpt::stream _stream, zpt::message _to_send);
    send(zpt::events::send const& _rhs) = delete;
    send(zpt::events::send&& _rhs) = delete;
    virtual ~send();

    auto operator=(zpt::events::send const& _rhs) -> send& = delete;
    auto operator=(zpt::events::send&& _rhs) -> send& = delete;

    auto initialize(zpt::event_initialization& init) -> void;
    auto blocked() const -> bool;
    auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(zpt::failed_expectation const& _e,
                     zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;

  protected:
    zpt::polling::ptr __polling;
    zpt::stream __stream;
    zpt::message __to_send;
};

class process {
  public:
    using ptr = std::shared_ptr<process>;
    friend class zpt::events::receive;

    process(zpt::message _received);
    process(zpt::events::process const& _rhs) = delete;
    process(zpt::events::process&& _rhs) = delete;
    virtual ~process();

    auto operator=(zpt::events::process const& _rhs) -> process& = delete;
    auto operator=(zpt::events::process&& _rhs) -> process& = delete;

    virtual auto received() const -> zpt::message const final;
    virtual auto to_send() -> zpt::message final;

    virtual auto initialize(zpt::event_initialization& init) -> void final;
    virtual auto catch_error(std::exception const& _e,
                             zpt::events::dispatcher::ptr _dispatcher) -> bool final;
    virtual auto catch_error(std::bad_alloc const& _e,
                             zpt::events::dispatcher::ptr _dispatcher) -> bool final;
    virtual auto catch_error(zpt::failed_expectation const& _e,
                             zpt::events::dispatcher::ptr _dispatcher) -> bool final;

    virtual auto blocked() const -> bool = 0;
    virtual auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state = 0;

  private:
    zpt::events::dispatcher::ptr __dispatcher;
    zpt::polling::ptr __polling;
    zpt::stream __stream;
    zpt::message __received{ nullptr };
    zpt::message __to_send{ nullptr };
};
} // namespace events
} // namespace zpt

template<typename T>
concept ProcessOperation = std::is_base_of<zpt::events::process, T>::value;

namespace zpt {
namespace events {
class discard : public zpt::events::process {
  public:
    discard(zpt::message _received);
    ~discard() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

template<ProcessOperation T = zpt::events::discard>
class call {
  public:
    using ptr = std::shared_ptr<process>;
    friend class zpt::events::receive;

    call(zpt::events::resolver _resolver, zpt::message _send);
    call(zpt::events::call<T> const& _rhs) = delete;
    call(zpt::events::call<T>&& _rhs) = delete;
    virtual ~call();

    auto operator=(zpt::events::call<T> const& _rhs) -> call& = delete;
    auto operator=(zpt::events::call<T>&& _rhs) -> call& = delete;

    auto initialize(zpt::event_initialization& init) -> void;
    auto blocked() const -> bool;
    auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(zpt::failed_expectation const& _e,
                     zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;

  private:
    zpt::events::dispatcher::ptr __dispatcher;
    zpt::events::resolver __resolver;
    zpt::polling::ptr __polling;
    zpt::message __to_send;
};
} // namespace events

auto TRANSPORT_ENGINE(zpt::json _config = nullptr) -> zpt::transports::engine&;
} // namespace zpt

template<typename T>
auto zpt::transports::make_callback(zpt::message _received,
                                    zpt::events::initializer_t _initializer) -> zpt::event {
    auto _event = zpt::make_event<T>(_received);
    _initializer(_event);
    return _event;
}

template<ProcessOperation T>
zpt::events::call<T>::call(zpt::events::resolver _resolver, zpt::message _send)
  : __resolver{ _resolver }
  , __to_send{ _send } {
    if (!this->__to_send->headers()("X-Conversation-ID")->ok()) {
        this->__to_send->headers()["X-Conversation-ID"] = zpt::generate::r_uuid();
    }
    this->__resolver->add(_send, zpt::transports::make_callback<T>);
}

template<ProcessOperation T>
zpt::events::call<T>::~call() {}

template<ProcessOperation T>
auto zpt::events::call<T>::initialize(zpt::event_initialization& _init) -> void {
    auto _transport_init = reinterpret_cast<zpt::events::transport_event_init&>(_init);
    this->__dispatcher = _transport_init.__dispatcher;
    this->__polling = _transport_init.__polling;
}

template<ProcessOperation T>
auto zpt::events::call<T>::blocked() const -> bool {
    return false;
}

template<ProcessOperation T>
auto zpt::events::call<T>::catch_error(std::exception const&,
                                       zpt::events::dispatcher::ptr) -> bool {
    return false;
}

template<ProcessOperation T>
auto zpt::events::call<T>::catch_error(std::bad_alloc const&,
                                       zpt::events::dispatcher::ptr) -> bool {
    return false;
}

template<ProcessOperation T>
auto zpt::events::call<T>::catch_error(zpt::failed_expectation const&,
                                       zpt::events::dispatcher::ptr) -> bool {
    return false;
}

template<ProcessOperation T>
auto zpt::events::call<T>::operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
    auto _uri = this->__to_send->uri();
    expect(_uri("path")->ok(), "Can't send a message without a resource path");

    std::string _scheme;
    std::string _address;
    unsigned int _port;
    if (_uri("scheme")->ok() && _uri("domain")->ok() && _uri("port")->ok()) {
        _scheme = _uri("scheme")->string();
        _address = _uri("domain")->string();
        _port = _uri("port")->integer();
    }
    else {
        auto _found = this->__resolver->search(_uri("path")->string());
        zlog(_found, zpt::debug);
        expect(_found->ok() && _found->size() != 0,
               "Couldn't find a provider of '" << _uri("path")->string());

        auto _provider = this->__resolver->get_provider(_found(0)("provider_id")->string());
        _scheme = _provider("protocols")("default")->string();
        _address = _provider("protocols")("registered")(_scheme)("bind")->string();
        _port = _provider("protocols")("registered")(_scheme)("port")->integer();
    }

    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get(_scheme);
    expect(_transport->is_synchronous(), "`call` only makes sense for synchronous protocols");

    auto _stream = zpt::make_stream<zpt::socketstream>(_address, _port, zpt::NO_SSL, IPPROTO_TCP);
    _stream->transport(_scheme);

    this->__to_send->headers()["Content-Type"] = "application/json";
    _transport->send(_stream, this->__to_send);

    this->__polling->listen_on(_stream);
    return zpt::events::finish;
}
