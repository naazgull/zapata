#include <zapata/globals.h>
#include <zapata/transport.h>
#include <zapata/transport/engine.h>

namespace {
template<typename T>
auto get_error_body(T const& _e) -> zpt::json {
    zpt::json _to_return{ "error", 500, "exception", zpt::demangle(typeid(T).name()) };
    if constexpr (std::is_same_v<T, std::bad_alloc>) {
        _to_return["what"] = "Unable to allocate memory outside configure maximum value.";
    }
    else { _to_return["what"] = _e.what(); }
    return _to_return;
}

template<typename T>
auto report_error(T const& _e,
                  zpt::stream _stream,
                  zpt::polling::ptr _polling,
                  zpt::events::dispatcher::ptr _dispatcher) -> void {
    if (_polling->is_in_shutdown() || _dispatcher->is_in_shutdown()) { return; }

    _stream->state() = zpt::stream_state::ERRORING_OUT;
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get(_stream->transport());
    auto _reply = _transport->make_reply(false);
    _reply->status(500);
    _reply->headers()["Content-Type"] = "application/json";
    _reply->body() = ::get_error_body(_e);
    _dispatcher->trigger<zpt::events::send>(_polling, _stream, _reply);
}
} // namespace

zpt::events::receive::receive(zpt::transports::engine& _engine,
                              zpt::polling::ptr _polling,
                              zpt::stream _stream)
  : __engine{ _engine }
  , __polling{ _polling }
  , __stream{ _stream } {}

zpt::events::receive::~receive() {}

auto zpt::events::receive::initialize(zpt::event_initialization&) -> void {}

auto zpt::events::receive::blocked() const -> bool { return false; }

auto zpt::events::receive::catch_error(std::exception const& _e,
                                       zpt::events::dispatcher::ptr _dispatcher) -> bool {
    ::report_error(_e, this->__stream, this->__polling, _dispatcher);
    return true;
}

auto zpt::events::receive::catch_error(std::bad_alloc const& _e,
                                       zpt::events::dispatcher::ptr _dispatcher) -> bool {
    ::report_error(_e, this->__stream, this->__polling, _dispatcher);
    return true;
}

auto zpt::events::receive::catch_error(zpt::failed_expectation const& _e,
                                       zpt::events::dispatcher::ptr _dispatcher) -> bool {
    ::report_error(_e, this->__stream, this->__polling, _dispatcher);
    return true;
}

auto zpt::events::receive::operator()(zpt::events::dispatcher::ptr _dispatcher)
  -> zpt::events::state {
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get(this->__stream->transport());
    try {
        auto _received = _transport->receive(this->__stream);
        if (this->__polling->is_in_shutdown() || _dispatcher->is_in_shutdown()) {
            return zpt::events::abort;
        }
        zlog(_received, zpt::debug);

        auto _events = this->__engine.resolve(_received, [this, _dispatcher](zpt::event _event) {
            zpt::events::transport_event_init _init;
            _init.__dispatcher = _dispatcher;
            _init.__polling = this->__polling;
            _init.__stream = this->__stream;
            _event->initialize(_init);
        });
        if (_events.size() == 0) {
            if (_transport->is_synchronous()) {
                auto _to_send = _transport->make_reply(_received);
                _to_send->status(404);
                _dispatcher->trigger<zpt::events::send>(this->__polling, this->__stream, _to_send);
            }
        }
        else {
            for (auto _event : _events) { _dispatcher->trigger(_event); }
        }
        return zpt::events::finish;
    }
    catch (std::bad_alloc const& _e) {
        this->catch_error(_e, _dispatcher);
    }
    catch (zpt::ClosedException const& _e) {
        throw;
    }
    catch (std::exception const& _e) {
        this->catch_error(_e, _dispatcher);
    }
    return zpt::events::abort;
}

zpt::events::send::send(zpt::polling::ptr _polling, zpt::stream _stream, zpt::message _to_send)
  : __polling{ _polling }
  , __stream{ _stream }
  , __to_send{ _to_send } {}

zpt::events::send::~send() { this->__polling->unmute(this->__stream); }

auto zpt::events::send::initialize(zpt::event_initialization&) -> void {}

auto zpt::events::send::blocked() const -> bool { return false; }

auto zpt::events::send::catch_error(std::exception const&, zpt::events::dispatcher::ptr) -> bool {
    return false;
}

auto zpt::events::send::catch_error(std::bad_alloc const&, zpt::events::dispatcher::ptr) -> bool {
    return false;
}

auto zpt::events::send::catch_error(zpt::failed_expectation const&, zpt::events::dispatcher::ptr)
  -> bool {
    return false;
}

auto zpt::events::send::operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get(this->__stream->transport());
    this->__to_send->headers()["Content-Type"] = "application/json";
    _transport->send(this->__stream, this->__to_send);
    return zpt::events::finish;
}

zpt::events::process::process(zpt::message _received)
  : __received{ _received } {}

zpt::events::process::~process() {
    try {
        auto _transport = zpt::TRANSPORT_LAYER() //
                            .get(this->__stream->transport());
        if (!_transport->is_synchronous() &&
            (this->__to_send == nullptr || this->__to_send->status() == 0)) {
            return;
        }
        if (this->__to_send == nullptr) {
            this->__to_send = _transport->make_reply(this->__received);
        }
        if (this->__to_send->status() == 0) { this->__to_send->status(204); }

        this->__dispatcher->trigger<zpt::events::send>(
          this->__polling, this->__stream, this->__to_send);
        return;
    }
    catch (std::bad_alloc const& _e) {
        this->catch_error(_e, this->__dispatcher);
    }
    catch (std::exception const& _e) {
        this->catch_error(_e, this->__dispatcher);
    }
}

auto zpt::events::process::received() const -> zpt::message const { return this->__received; }

auto zpt::events::process::to_send() -> zpt::message { return this->__to_send; }

auto zpt::events::process::initialize(zpt::event_initialization& _init) -> void {
    auto _transport_init = reinterpret_cast<zpt::events::transport_event_init&>(_init);
    this->__dispatcher = _transport_init.__dispatcher;
    this->__polling = _transport_init.__polling;
    this->__stream = _transport_init.__stream;
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get(this->__stream->transport());
    this->__to_send = _transport->make_reply(this->__received);
    this->__to_send->status(0);
}

auto zpt::events::process::catch_error(std::exception const& _e,
                                       zpt::events::dispatcher::ptr _dispatcher) -> bool {
    ::report_error(_e, this->__stream, this->__polling, _dispatcher);
    return true;
}

auto zpt::events::process::catch_error(std::bad_alloc const& _e,
                                       zpt::events::dispatcher::ptr _dispatcher) -> bool {
    ::report_error(_e, this->__stream, this->__polling, _dispatcher);
    return true;
}

auto zpt::events::process::catch_error(zpt::failed_expectation const& _e,
                                       zpt::events::dispatcher::ptr _dispatcher) -> bool {
    ::report_error(_e, this->__stream, this->__polling, _dispatcher);
    return true;
}

zpt::transports::engine::engine(zpt::json _config)
  : __configuration{ _config }
  , __dispatcher{ std::make_shared<zpt::events::dispatcher>(
      "transport",
      _config("limits")("max_consumer_threads")->ok()
        ? _config("limits")("max_consumer_threads")->integer()
        : 1) } {
    zpt::STREAM_POLLING() //
      ->register_delegate([this](zpt::polling::ptr _poll, zpt::stream _stream) -> bool {
          try {
              this->__dispatcher->trigger<zpt::events::receive>(*this, _poll, _stream);
              return true;
          }
          catch (std::bad_alloc const& _e) {
              ::report_error(_e, _stream, _poll, this->__dispatcher);
          }
          catch (zpt::ClosedException const& _e) {
              throw;
          }
          catch (std::exception const& _e) {
              ::report_error(_e, _stream, _poll, this->__dispatcher);
          }
          return true;
      });
    auto _event_init = std::make_shared<zpt::events::transport_event_init>();
    _event_init->__polling = zpt::STREAM_POLLING();
    _event_init->__dispatcher = this->__dispatcher;
    this
      ->__dispatcher //
      ->set_event_initialization(_event_init)
      .start_consumers();
}

auto zpt::transports::engine::add_resolver(zpt::events::resolver _resolver)
  -> zpt::transports::engine& {
    this->__resolvers.push_back(_resolver);
    return (*this);
}

auto zpt::transports::engine::resolve(zpt::message _received,
                                      zpt::events::initializer_t _initializer) const
  -> std::list<zpt::event> {
    std::list<zpt::event> _return;
    for (auto& _resolver : this->__resolvers) {
        try {
            auto _events = _resolver->resolve(_received, _initializer);
            _return.insert(_return.end(), _events.begin(), _events.end());
        }
        catch (...) {
        }
    }
    return _return;
}

auto zpt::transports::engine::shutdown() -> zpt::transports::engine& {
    this->__dispatcher->stop_consumers();
    return (*this);
}

auto zpt::TRANSPORT_ENGINE(zpt::json _config) -> zpt::transports::engine& {
    static zpt::transports::engine _global{ _config };
    return _global;
}
