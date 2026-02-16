#pragma once

#include <list>
#include <zapata/events.h>
#include <zapata/net/socket.h>
#include <zapata/startup.h>
#include <zapata/streams.h>
#include <zapata/transport.h>

namespace zpt {
namespace transports {
class engine {
  public:
    engine(zpt::json _config);
    virtual ~engine() = default;

    auto add_resolver(zpt::events::resolver _resolver) -> engine&;
    auto resolve(zpt::message _received, zpt::events::initializer_t _initializer) const
      -> std::list<zpt::event>;
    auto dispatcher() -> zpt::events::dispatcher::ptr;
    auto shutdown() -> engine&;

  private:
    zpt::json __configuration;
    std::vector<zpt::events::resolver> __resolvers;
    zpt::events::dispatcher::ptr __dispatcher;
};
} // namespace transports

namespace events {
class transport_event_init : public zpt::event_initialization {
  public:
    zpt::events::dispatcher::ptr __dispatcher{ nullptr };
    zpt::polling::ptr __polling{ nullptr };
    zpt::stream __stream{ nullptr };
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
    auto catch_error(zpt::failed_expectation const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool;
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
    auto catch_error(zpt::failed_expectation const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool;
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
    virtual auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool final;
    virtual auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool final;
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
    bool __error_sent{ false };
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

constexpr int CALL_STATE_UNPROCESSED = 0;
constexpr int CALL_STATE_SENT = 1;
constexpr int CALL_STATE_SUCCESS_REPLY = 2;
constexpr int CALL_STATE_FAILURE_REPLY = 3;

class call_context {
  public:
    using ptr = std::shared_ptr<call_context>;

    call_context() = default;
    ~call_context() = default;

    auto state() const -> int;
    auto reply() const -> zpt::message;
    auto reply(zpt::message _to_update) -> call_context&;
    auto is_replied() const -> bool;
    auto has_error() const -> bool;

  private:
    zpt::padded_atomic<int> __state{ zpt::events::CALL_STATE_SENT };
    zpt::message __reply{ nullptr };
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
    auto catch_error(zpt::failed_expectation const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;

  private:
    zpt::events::dispatcher::ptr __dispatcher;
    zpt::events::resolver __resolver;
    zpt::polling::ptr __polling;
    zpt::message __to_send;

    auto call_internally() -> call&;
    auto send_externally() -> call&;
};
} // namespace events

auto TRANSPORT_ENGINE(zpt::json _config = nullptr) -> zpt::transports::engine&;

template<ProcessOperation T>
auto make_call(zpt::events::resolver _resolver, zpt::message _to_send)
  -> zpt::events::call_context::ptr;
} // namespace zpt

template<ProcessOperation T>
zpt::events::call<T>::call(zpt::events::resolver _resolver, zpt::message _send)
  : __resolver{ _resolver }
  , __to_send{ _send } {
    if (!this->__to_send->headers()("X-Conversation-ID")->ok()) {
        this->__to_send->headers()["X-Conversation-ID"] = zpt::generate::r_uuid();
    }
    this->__resolver->add(_send, zpt::events::make_callback<T>);
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
auto zpt::events::call<T>::catch_error(std::exception const&, zpt::events::dispatcher::ptr)
  -> bool {
    return false;
}

template<ProcessOperation T>
auto zpt::events::call<T>::catch_error(std::bad_alloc const&, zpt::events::dispatcher::ptr)
  -> bool {
    return false;
}

template<ProcessOperation T>
auto zpt::events::call<T>::catch_error(zpt::failed_expectation const&, zpt::events::dispatcher::ptr)
  -> bool {
    return false;
}

template<ProcessOperation T>
auto zpt::events::call<T>::operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
    auto& _uri = this->__to_send->uri();
    expect(_uri("path")->ok(), "Can't send a message without a resource path");

    bool _is_self{ false };
    if (!_uri("scheme")->ok() || !_uri("domain")->ok() || !_uri("port")->ok()) {
        auto _found = this->__resolver->search(
          std::format("/{}{}",
                      zpt::ontology::to_str(this->__to_send->performative()),
                      _uri("raw_path")->string()));
        expect(_found->ok() && _found->size() != 0,
               "Couldn't find a provider of '" << _uri("path")->string());

        for (auto [_, __, _service] : _found) {
            if ((_is_self = (_service("provider_id") == zpt::IDENTITY()("_id")))) { break; }
        }

        if (!_is_self) {
            auto _provider = this->__resolver->get_provider(_found(0)("provider_id")->string());
            expect(_provider->ok() && _provider->size() != 0,
                   "Couldn't find a provider of '" << _uri("path")->string());
            _uri["scheme"] = _provider(0)("protocols")("default");
            _uri["domain"] =
              _provider(0)("protocols")("registered")(_uri("scheme")->string())("bind");
            _uri["port"] =
              _provider(0)("protocols")("registered")(_uri("scheme")->string())("port");
        }
    }

    if (_is_self) { this->call_internally(); }
    else { this->send_externally(); }

    return zpt::events::finish;
}

template<ProcessOperation T>
auto zpt::events::call<T>::call_internally() -> call& {
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get("self");
    expect(_transport->has_capability(zpt::transport_capability::SYNCHRONOUS),
           "`call` only makes sense for synchronous protocols");

    auto _stream = std::make_shared<zpt::event_stream>();
    _stream->transport("self");

    this->__to_send->headers()["Content-Type"] = "application/json";
    _transport->send(_stream, this->__to_send);

    this->__polling->listen_on(_stream);

    return (*this);
}

template<ProcessOperation T>
auto zpt::events::call<T>::send_externally() -> call& {
    auto& _uri = this->__to_send->uri();
    auto _scheme = _uri("scheme")->string();
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get(_scheme);
    expect(_transport->has_capability(zpt::transport_capability::SYNCHRONOUS),
           "`call` only makes sense for synchronous protocols");

    auto _stream = zpt::make_stream<zpt::socketstream>(
      _uri("domain")->string(), _uri("port")->integer(), zpt::NO_SSL, IPPROTO_TCP);
    _stream->transport(_scheme);

    this->__to_send->headers()["Content-Type"] = "application/json";
    _transport->send(_stream, this->__to_send);

    this->__polling->listen_on(_stream);

    return (*this);
}

template<ProcessOperation T>
auto zpt::make_call(zpt::events::resolver _resolver, zpt::message _to_send)
  -> zpt::events::call_context::ptr {
    zpt::TRANSPORT_ENGINE() //
      .dispatcher()
      ->trigger<zpt::events::call<T>>(_resolver, _to_send);
    return std::make_shared<zpt::events::call_context>();
}
