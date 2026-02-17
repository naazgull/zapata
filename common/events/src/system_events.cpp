#include <zapata/events/system_events.h>

zpt::system_event::system_event(zpt::message _received)
  : __received{ _received } {}

zpt::system_event::system_event(zpt::message _received, zpt::call_context::ptr)
  : __received{ _received } {}

zpt::system_event::system_event(zpt::system_event_type _type, zpt::json const& _data)
  : __type{ _type }
  , __received{ zpt::make_message<zpt::json_message>() } {
    this->__received->uri() =
      zpt::json{ "scheme", "events", "raw_path", static_cast<long long>(this->__type) };
    this->__received->body() = _data;
}

auto zpt::system_event::initialize(zpt::event_initialization&) -> void {
    expect(false, "Not implemented for `zpt::system_events`");
}

auto zpt::system_event::blocked() const -> bool { return false; }

auto zpt::system_event::authorized() const -> bool { return true; }

auto zpt::system_event::catch_error(std::exception const& _e, zpt::events::dispatcher::ptr)
  -> bool {
    zlog(_e.what(), zpt::error);
    return true;
}

auto zpt::system_event::catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr)
  -> bool {
    zlog(_e.what(), zpt::error);
    return true;
}

auto zpt::system_event::catch_error(zpt::failed_expectation const& _e, zpt::events::dispatcher::ptr)
  -> bool {
    zlog(_e.what(), zpt::error);
    return true;
}

auto zpt::system_event::operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state {
    auto _events = zpt::SYSTEM_EVENTS_RESOLVER()->resolve(this->__received, [this](zpt::event) {});
    for (auto& _event : _events) { _dispatcher->trigger(_event); }
    return zpt::events::finish;
}

auto zpt::system_events::resolver_t::add(zpt::json const&) -> resolver_t& {
    expect(false, "Not implemented for `zpt::system_events`");
    return (*this);
}

auto zpt::system_events::resolver_t::add(zpt::message,
                                         zpt::call_context::ptr,
                                         zpt::events::resolver_callback) -> resolver_t& {
    expect(false, "Not implemented for `zpt::system_events`");
    return (*this);
}

auto zpt::system_events::resolver_t::add(zpt::performative,
                                         zpt::json const& _id,
                                         zpt::json const& _type,
                                         zpt::events::resolver_callback _callback) -> resolver_t& {
    this->__callbacks[static_cast<zpt::system_event_type>(_type->integer())].insert(
      std::make_pair(_id, _callback));
    return (*this);
}

auto zpt::system_events::resolver_t::remove(zpt::message) -> resolver_t& {
    expect(false, "Not implemented for `zpt::system_events`");
    return (*this);
}

auto zpt::system_events::resolver_t::remove(zpt::performative, zpt::json const& _id)
  -> resolver_t& {
    for (auto& [_, _per_id] : this->__callbacks) { _per_id.erase(_id); }
    return (*this);
}

auto zpt::system_events::resolver_t::resolve(zpt::message _received,
                                             zpt::events::initializer_t _initializer) const
  -> std::list<zpt::event> {
    std::list<zpt::event> _return;

    auto _type = _received->resource()->integer();
    auto _per_id = this->__callbacks.find(static_cast<zpt::system_event_type>(_type));
    if (_per_id != this->__callbacks.end()) {
        for (auto [_, _callback] : _per_id->second) {
            _return.push_back(_callback(_received, nullptr, _initializer));
        }
    }

    return _return;
}

auto zpt::system_events::resolver_t::search(zpt::json const&, std::string const&) const
  -> zpt::json {
    expect(false, "Not implemented for `zpt::system_events`");
    return zpt::undefined;
}

auto zpt::system_events::resolver_t::list(std::string const&) const -> zpt::json {
    expect(false, "Not implemented for `zpt::system_events`");
    return zpt::undefined;
}

auto zpt::system_events::resolver_t::register_provider(zpt::json const&) -> resolver_t& {
    expect(false, "Not implemented for `zpt::system_events`");
    return (*this);
}

auto zpt::system_events::resolver_t::unregister_provider(std::string const&) -> resolver_t& {
    return (*this);
}

auto zpt::system_events::resolver_t::get_provider(std::string const&) const -> zpt::json {
    expect(false, "Not implemented for `zpt::system_events`");
    return zpt::undefined;
}

auto zpt::system_events::resolver_t::clear() -> resolver_t& {
    this->__callbacks.clear();
    return (*this);
}

auto zpt::SYSTEM_EVENTS_RESOLVER() -> zpt::system_events::resolver {
    static zpt::system_events::resolver _global =
      std::make_shared<zpt::system_events::resolver_t>();
    return _global;
}
