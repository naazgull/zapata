#include <zapata/events/system_events.h>

zpt::system_event::system_event(zpt::system_event_type _type, zpt::json const& _data) {}

auto zpt::system_event::initialize(zpt::event_initialization& _init) -> void {}

auto zpt::system_event::blocked() const -> bool { return false; }

auto zpt::system_event::catch_error(std::exception const& _e,
                                    zpt::events::dispatcher::ptr _dispatcher) -> bool {
    return false;
}

auto zpt::system_event::catch_error(std::bad_alloc const& _e,
                                    zpt::events::dispatcher::ptr _dispatcher) -> bool {
    return false;
}

auto zpt::system_event::catch_error(zpt::failed_expectation const& _e,
                                    zpt::events::dispatcher::ptr _dispatcher) -> bool {
    return false;
}

auto zpt::system_event::operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state {
    return zpt::events::finish;
}

auto zpt::system_events::resolver_t::add(zpt::json const& _service_description) -> resolver_t& {
    return (*this);
}

auto zpt::system_events::resolver_t::add(zpt::message _sent,
                                         zpt::events::resolver_callback callback) -> resolver_t& {
    return (*this);
}

auto zpt::system_events::resolver_t::add(zpt::performative _performative,
                                         zpt::json const& _id,
                                         zpt::json const& _metadata,
                                         zpt::events::resolver_callback _callback) -> resolver_t& {
    auto _type = _id->integer();
    this->__callbacks[_type].push_back(_callback);
    return (*this);
}

auto zpt::system_events::resolver_t::remove(zpt::message _sent) -> resolver_t& { return (*this); }

auto zpt::system_events::resolver_t::remove(zpt::performative _performative,
                                            zpt::json const& _id) -> resolver_t& {
    return (*this);
}

auto zpt::system_events::resolver_t::resolve(zpt::message _received,
                                             zpt::events::initializer_t _initializer) const
  -> std::list<zpt::event> {
    return {};
}

auto zpt::system_events::resolver_t::search(zpt::json const& _id,
                                            std::string const& _provider_id) const -> zpt::json {}

auto zpt::system_events::resolver_t::list(std::string const& _provider_id) const -> zpt::json {}

auto zpt::system_events::resolver_t::register_provider(zpt::json const& _provider) -> resolver_t& {
    return (*this);
}

auto zpt::system_events::resolver_t::unregister_provider(std::string const& _id) -> resolver_t& {
    return (*this);
}

auto zpt::system_events::resolver_t::get_provider(std::string const& _id) const -> zpt::json {}

auto zpt::system_events::resolver_t::clear() -> resolver_t& { return (*this); }

auto zpt::SYSTEM_EVENTS_RESOLVER() -> zpt::events::resolver {
    static zpt::events::resolver _global = std::make_shared<zpt::system_events::resolver_t>();
    return _global;
}
