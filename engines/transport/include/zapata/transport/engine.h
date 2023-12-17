#pragma once

#include <zapata/events.h>
#include <zapata/streams.h>
#include <zapata/transport.h>
#include <zapata/mem/ref_ptr.h>
#include <list>

namespace zpt {
auto TRANSPORT_ENGINE() -> ssize_t&;

namespace events {
class process;
using initializer_t = std::function<void(zpt::events::process& _event)>;
class resolver_t {
  public:
    resolver_t() = default;
    virtual ~resolver_t() = default;

    virtual auto resolve(zpt::message _received, initializer_t _initializer) const
      -> std::list<zpt::event> = 0;
};
using resolver = std::shared_ptr<resolver_t>;
} // namespace events

namespace transports {
class engine {
  public:
    engine(zpt::json _config);
    virtual ~engine() = default;

    auto add_resolver(zpt::events::resolver _resolver) -> zpt::transports::engine&;
    auto resolve(zpt::message _received, zpt::events::initializer_t _initializer) const
      -> std::list<zpt::event>;

  private:
    zpt::json __configuration;
    std::vector<zpt::events::resolver> __resolvers;
    zpt::events::dispatcher __dispatcher;
};
} // namespace transports

namespace events {
class receive {
  public:
    receive(zpt::transports::engine& _engine, zpt::polling& _polling, zpt::stream _stream);
    receive(zpt::events::receive const& _rhs) = delete;
    receive(zpt::events::receive&& _rhs) = delete;
    virtual ~receive();

    auto operator=(zpt::events::receive const& _rhs) -> receive& = delete;
    auto operator=(zpt::events::receive&& _rhs) -> receive& = delete;

    auto blocked() const -> bool;
    auto catch_error(std::exception const& _e) -> bool;
    auto catch_error(std::bad_alloc const& _e) -> bool;
    auto catch_error(zpt::failed_expectation const& _e) -> bool;
    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state;

  protected:
    zpt::transports::engine& __engine;
    zpt::polling& __polling;
    zpt::stream __stream;
};

class send {
  public:
    send(zpt::polling& _polling, zpt::stream _stream, zpt::message _to_send);
    send(zpt::events::send const& _rhs) = delete;
    send(zpt::events::send&& _rhs) = delete;
    virtual ~send();

    auto operator=(zpt::events::send const& _rhs) -> send& = delete;
    auto operator=(zpt::events::send&& _rhs) -> send& = delete;

    auto blocked() const -> bool;
    auto catch_error(std::exception const& _e) -> bool;
    auto catch_error(std::bad_alloc const& _e) -> bool;
    auto catch_error(zpt::failed_expectation const& _e) -> bool;
    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state;

  protected:
    zpt::polling& __polling;
    zpt::stream __stream;
    zpt::message __to_send;
};

class process {
  public:
    friend class zpt::events::receive;

    process(zpt::message _received);
    process(zpt::events::process const& _rhs) = delete;
    process(zpt::events::process&& _rhs) = delete;
    virtual ~process();

    auto operator=(zpt::events::process const& _rhs) -> process& = delete;
    auto operator=(zpt::events::process&& _rhs) -> process& = delete;

    virtual auto initialize(zpt::events::dispatcher& _dispatcher,
                            zpt::polling& _polling,
                            zpt::stream _stream) -> process& final;
    virtual auto received() const -> zpt::message const final;
    virtual auto to_send() -> zpt::message final;

    virtual auto blocked() const -> bool = 0;
    virtual auto catch_error(std::exception const& _e) -> bool final;
    virtual auto catch_error(std::bad_alloc const& _e) -> bool final;
    virtual auto catch_error(zpt::failed_expectation const& _e) -> bool final;
    virtual auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state = 0;

  private:
    zpt::ref_ptr<zpt::events::dispatcher> __dispatcher;
    zpt::ref_ptr<zpt::polling> __polling;
    zpt::stream __stream;
    zpt::message __received;
    zpt::message __to_send;
};
} // namespace events
} // namespace zpt
