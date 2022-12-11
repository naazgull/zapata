#include <zapata/events/engine.h>

auto zpt::EVENTS_ENGINE() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::events::receive::receive(zpt::events::engine& _engine,
                              zpt::polling& _polling,
                              zpt::basic_stream& _stream)
  : __engine{ _engine }
  , __polling{ _polling }
  , __stream{ _stream } {}

zpt::events::receive::~receive() {
    if (this->__unmute) { this->__polling.unmute(this->__stream); }
}

auto zpt::events::receive::blocked() const -> bool { return false; }

auto zpt::events::receive::operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state {
    auto _transport = zpt::global_cast<zpt::network::layer>(zpt::TRANSPORT_LAYER()) //
                        .get(this->__stream.transport());
    try {
        auto _received = _transport->receive(this->__stream);
        auto _events =
          this->__engine.resolve(_received, [this, &_dispatcher](zpt::events::process& _event) {
              _event.initialize(_dispatcher, this->__polling, this->__stream);
          });
        if (_events.size() == 0) {
            auto _reply = _transport->make_reply();
            _reply->status(404);
            _transport->send(this->__stream, _reply);
            this->__unmute = true;
        }
        else {
            for (auto _event : _events) { _dispatcher.trigger(_event); }
        }
    }
    catch (zpt::SyntaxErrorException const& _e) {
        auto _reply = _transport->make_reply();
        _reply->status(500);
        _reply->headers()["Content-Type"] = "application/json";
        _reply->body() = { "error",    500,            //
                           "type",     "Protocol",     //
                           "category", "Syntax Error", //
                           "message",  _e.what() };
        _transport->send(this->__stream, _reply);
        this->__unmute = true;
    }
    catch (zpt::failed_expectation const& _e) {
        auto _reply = _transport->make_reply();
        _reply->status(500);
        _reply->headers()["Content-Type"] = "application/json";
        _reply->body() = { "error",    500,                  //
                           "type",     "Functional",         //
                           "category", "Failed Expectation", //
                           "message",  _e.what() };
        _transport->send(this->__stream, _reply);
        this->__unmute = true;
    }
    return zpt::events::finish;
}

zpt::events::send::send(zpt::polling& _polling, zpt::basic_stream& _stream, zpt::message _to_send)
  : __polling{ _polling }
  , __stream{ _stream }
  , __to_send{ _to_send } {}

zpt::events::send::~send() { this->__polling.unmute(this->__stream); }

auto zpt::events::send::blocked() const -> bool { return false; }

auto zpt::events::send::operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state {
    auto _transport = zpt::global_cast<zpt::network::layer>(zpt::TRANSPORT_LAYER()) //
                        .get(this->__stream.transport());
    this->__to_send->headers()["Content-Type"] = "application/json";
    _transport->send(this->__stream, this->__to_send);
    return zpt::events::finish;
}

zpt::events::process::process(zpt::message _received)
  : __received{ _received } {}

zpt::events::process::~process() {
    if (this->__to_send->status() != 0) {
        this->__dispatcher->trigger<zpt::events::send>(
            *this->__polling, *this->__stream, this->__to_send);
    }
}

auto zpt::events::process::initialize(zpt::events::dispatcher& _dispatcher,
                                      zpt::polling& _polling,
                                      zpt::basic_stream& _stream) -> process& {
    this->__dispatcher = _dispatcher;
    this->__polling = _polling;
    this->__stream = _stream;
    auto _transport = zpt::global_cast<zpt::network::layer>(zpt::TRANSPORT_LAYER()) //
                        .get(this->__stream->transport());
    this->__to_send = _transport->make_reply(this->__received);
    this->__to_send->status(0);
    return (*this);
}

auto zpt::events::process::received() const -> zpt::message const { return this->__received; }

auto zpt::events::process::to_send() -> zpt::message { return this->__to_send; }

zpt::events::engine::engine(zpt::json _config)
  : __configuration{ _config } {
    zpt::global_cast<zpt::polling>(zpt::STREAM_POLLING()) //
      .register_delegate([this](zpt::polling& _poll, zpt::basic_stream& _stream) -> bool {
          zpt::global_cast<zpt::events::dispatcher>(zpt::DISPATCHER())
            .trigger<zpt::events::receive>(*this, _poll, _stream);
          return true;
      });
}

auto zpt::events::engine::add_resolver(zpt::events::resolver _resolver) -> zpt::events::engine& {
    this->__resolvers.push_back(_resolver);
    return (*this);
}

auto zpt::events::engine::resolve(zpt::message _received, initializer_t _initializer) const
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
