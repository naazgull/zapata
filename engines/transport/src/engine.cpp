#include <zapata/globals.h>
#include <zapata/transport.h>
#include <zapata/transport/engine.h>

namespace {
template<typename T>
auto get_error_body(T const& _e) -> zpt::json {
    int _error{ 500 };
    if constexpr (std::is_same_v<T, zpt::failed_expectation>) { _error = _e.code(); }

    zpt::json _to_return{ "error", _error, "exception", zpt::demangle(typeid(T).name()) };
    if constexpr (std::is_same_v<T, std::bad_alloc>) {
        _to_return["what"] = "Unable to allocate memory outside configure maximum value.";
    }
    else { _to_return["what"] = _e.what(); }
    return _to_return;
}

template<typename T>
auto report_error(T const& _e, zpt::stream _stream, zpt::polling::ptr _polling) -> zpt::message {
#ifdef PROPAGATE_EXCEPTION
    throw _e;
#endif
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get(_stream->transport());

    if (!_transport->has_capability(zpt::transport_capability::SYNCHRONOUS) ||
        _polling->is_in_shutdown()) {
        _polling->unmute(_stream);
        zlog(_e.what(), zpt::error);
        return nullptr;
    }

    _stream->state() = zpt::stream_state::ERRORING_OUT;
    auto _reply = _transport->make_reply(false);
    auto _body = ::get_error_body(_e);
    _reply //
      ->status(_body("error")->integer())
      .body() = _body;
    return _reply;
}
} // namespace

zpt::events::receive::receive(zpt::transports::engine::ptr _engine,
                              zpt::polling::ptr _polling,
                              zpt::stream _stream)
  : __engine{ _engine }
  , __polling{ _polling }
  , __stream{ _stream } {}

zpt::events::receive::~receive() {}

auto zpt::events::receive::initialize(zpt::event_initialization&) -> void {}

auto zpt::events::receive::blocked() const -> bool { return false; }

auto zpt::events::receive::authorized() const -> bool { return true; }

auto zpt::events::receive::catch_error(std::exception const& _e,
                                       zpt::events::dispatcher::ptr _dispatcher) -> bool {
    auto _reply = ::report_error(_e, this->__stream, this->__polling);
    if (_reply != nullptr) {
        _dispatcher->trigger<zpt::events::send>(this->__polling, this->__stream, _reply);
    }
    return true;
}

auto zpt::events::receive::catch_error(std::bad_alloc const& _e,
                                       zpt::events::dispatcher::ptr _dispatcher) -> bool {
    auto _reply = ::report_error(_e, this->__stream, this->__polling);
    if (_reply != nullptr) {
        _dispatcher->trigger<zpt::events::send>(this->__polling, this->__stream, _reply);
    }
    return true;
}

auto zpt::events::receive::catch_error(zpt::failed_expectation const& _e,
                                       zpt::events::dispatcher::ptr _dispatcher) -> bool {
    auto _reply = ::report_error(_e, this->__stream, this->__polling);
    if (_reply != nullptr) {
        _dispatcher->trigger<zpt::events::send>(this->__polling, this->__stream, _reply);
    }
    return true;
}

auto zpt::events::receive::operator()(zpt::events::dispatcher::ptr _dispatcher)
  -> zpt::events::state {
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get(this->__stream->transport());
#ifndef PROPAGATE_EXCEPTION
    try {
#endif
        auto _received = _transport->receive(this->__stream);
        if (!_received->empty() && !this->__polling->is_in_shutdown() &&
            !_dispatcher->is_in_shutdown()) {

            auto _events =
              this->__engine->resolve(_received, [this, _dispatcher](zpt::event& _event) {
                  zpt::events::transport_event_init _init;
                  _init.__dispatcher = _dispatcher;
                  _init.__polling = this->__polling;
                  _init.__stream = this->__stream;
                  _event->initialize(_init);
              });

            if (_events.size() == 0) {
                if (_transport->has_capability(zpt::transport_capability::SYNCHRONOUS) &&
                    _received->performative() != zpt::Reply) {
                    auto _to_send = _transport->make_reply(_received);
                    _to_send->status(404);
                    _dispatcher->trigger<zpt::events::send>(
                      this->__polling, this->__stream, _to_send);
                }
                else {
                    zlog("Couldn't find a callback for '" << _received->uri() << "'", zpt::error);
                    this->__polling->unmute(this->__stream);
                }
            }
            else {
                for (auto& _event : _events) { _dispatcher->trigger(std::move(_event)); }
            }
            return zpt::events::finish;
        }
        this->__polling->unmute(this->__stream);
#ifndef PROPAGATE_EXCEPTION
    }
    catch (std::bad_alloc const& _e) {
        this->catch_error(_e, _dispatcher);
    }
    catch (std::exception const& _e) {
        this->catch_error(_e, _dispatcher);
    }
#endif
    return zpt::events::abort;
}

zpt::events::send::send(zpt::polling::ptr _polling, zpt::stream _stream, zpt::message _to_send)
  : __polling{ _polling }
  , __stream{ _stream }
  , __to_send{ _to_send } {}

zpt::events::send::~send() { this->__polling->unmute(this->__stream); }

auto zpt::events::send::initialize(zpt::event_initialization&) -> void {}

auto zpt::events::send::blocked() const -> bool { return false; }

auto zpt::events::send::authorized() const -> bool { return true; }

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
  : __received{ _received }
  , __context{ nullptr } {}

zpt::events::process::process(zpt::message _received, zpt::call_context::ptr _context)
  : __received{ _received }
  , __context{ _context } {}

zpt::events::process::~process() {
#ifndef PROPAGATE_EXCEPTION
    try {
#endif
        auto _transport = zpt::TRANSPORT_LAYER() //
                            .get(this->__stream->transport());
        if (this->__to_send->status() != 100) {
            if ((_transport->has_capability(zpt::transport_capability::SYNCHRONOUS) &&
                 this->__received->performative() != zpt::Reply) ||
                (this->__to_send != nullptr && this->__to_send->status() != 0)) {
                if (this->__to_send == nullptr) {
                    this->__to_send = _transport->make_reply(this->__received);
                }
                if (this->__to_send->status() == 0) { this->__to_send->status(204); }

                this->__dispatcher->trigger<zpt::events::send>(
                  this->__polling, this->__stream, this->__to_send);
                return;
            }
        }
        this->__polling->unmute(this->__stream);
#ifndef PROPAGATE_EXCEPTION
    }
    catch (std::bad_alloc const& _e) {
        this->catch_error(_e, this->__dispatcher);
    }
    catch (std::exception const& _e) {
        this->catch_error(_e, this->__dispatcher);
    }
#endif
}

auto zpt::events::process::transport_type() const -> std::string const& {
    return this->__stream->transport();
}

auto zpt::events::process::received() const -> zpt::message const { return this->__received; }

auto zpt::events::process::to_send() -> zpt::message { return this->__to_send; }

auto zpt::events::process::context() const -> zpt::call_context::ptr { return this->__context; }

auto zpt::events::process::context(zpt::call_context::ptr _context) -> process& {
    this->__context = _context;
    return (*this);
}

auto zpt::events::process::initialize(zpt::event_initialization& _init) -> void {
    auto _transport_init = reinterpret_cast<zpt::events::transport_event_init&>(_init);
    this->__dispatcher = _transport_init.__dispatcher.lock();
    this->__polling = _transport_init.__polling;
    this->__stream = _transport_init.__stream;
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get(this->__stream->transport());
    this->__to_send = _transport->make_reply(this->__received);
    this->__to_send->status(0);
}

auto zpt::events::process::authorized() const -> bool { return true; }

auto zpt::events::process::catch_error(std::exception const& _e, zpt::events::dispatcher::ptr)
  -> bool {
    auto _reply = ::report_error(_e, this->__stream, this->__polling);
    if (_reply != nullptr) {
        this
          ->__to_send //
          ->status(_reply->status())
          .body() = _reply->body();
    }
    return true;
}

auto zpt::events::process::catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr)
  -> bool {
    auto _reply = ::report_error(_e, this->__stream, this->__polling);
    if (_reply != nullptr) {
        this
          ->__to_send //
          ->status(_reply->status())
          .body() = _reply->body();
    }
    return true;
}

auto zpt::events::process::catch_error(zpt::failed_expectation const& _e,
                                       zpt::events::dispatcher::ptr) -> bool {
    auto _reply = ::report_error(_e, this->__stream, this->__polling);
    if (_reply != nullptr) {
        this
          ->__to_send //
          ->status(_reply->status())
          .body() = _reply->body();
    }
    return true;
}

zpt::transports::engine::engine(zpt::json _config)
  : __configuration{ _config }
  , __dispatcher{ zpt::allocate_shared<zpt::events::dispatcher>(
      "transport",
      _config("limits")("max_consumer_threads")->ok()
        ? _config("limits")("max_consumer_threads")->integer()
        : 1) } {
    zpt::STREAM_POLLING() //
      ->register_delegate([this](zpt::polling::ptr _poll, zpt::stream _stream) -> bool {
#ifndef PROPAGATE_EXCEPTION
          try {
#endif
              this->__dispatcher->trigger<zpt::events::receive>(
                this->shared_from_this(), _poll, _stream);
              return true;
#ifndef PROPAGATE_EXCEPTION
          }
          catch (std::bad_alloc const& _e) {
              auto _reply = ::report_error(_e, _stream, _poll);
              if (_reply != nullptr) {
                  this->__dispatcher->trigger<zpt::events::send>(_poll, _stream, _reply);
              }
          }
          catch (std::exception const& _e) {
              auto _reply = ::report_error(_e, _stream, _poll);
              if (_reply != nullptr) {
                  this->__dispatcher->trigger<zpt::events::send>(_poll, _stream, _reply);
              }
          }
#endif
          return true;
      });
    auto _event_init = zpt::allocate_shared<zpt::events::transport_event_init>();
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
            _return.merge(_events);
        }
        catch (...) {
        }
    }
    return _return;
}

auto zpt::transports::engine::dispatcher() -> zpt::events::dispatcher::ptr {
    return this->__dispatcher;
}

auto zpt::transports::engine::shutdown() -> zpt::transports::engine& {
    this->__dispatcher->stop_consumers();
    return (*this);
}

auto zpt::events::discard::blocked() const -> bool { return false; }

auto zpt::events::discard::operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
    return zpt::events::finish;
}

auto zpt::events::process_call_reply::blocked() const -> bool { return false; }

auto zpt::events::process_call_reply::operator()(zpt::events::dispatcher::ptr _dispatcher
                                                 [[maybe_unused]]) -> zpt::events::state {
    this->context()->reply(this->received());
    return zpt::events::finish;
}

auto zpt::TRANSPORT_ENGINE(zpt::json _config) -> zpt::transports::engine::ptr {
    static auto _global = std::make_shared<zpt::transports::engine>(_config);
    return _global;
}
