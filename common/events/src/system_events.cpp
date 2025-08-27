#include <zapata/events/system_events.h>

auto zpt::system_events::resolver_t::add(zpt::json const& _service_description) -> resolver_t& {
    return (*this);
}

auto zpt::system_events::resolver_t::add(zpt::message _sent,
                                         zpt::events::resolver_callback callback) -> resolver_t& {
    return (*this);
}

auto zpt::system_events::resolver_t::add(zpt::performative _performative,
                                         std::string const& _path,
                                         zpt::json const& _metadata,
                                         zpt::events::resolver_callback _callback) -> resolver_t& {
    return (*this);
}

auto zpt::system_events::resolver_t::remove(zpt::message _sent) -> resolver_t& { return (*this); }

auto zpt::system_events::resolver_t::remove(zpt::performative _performative,
                                            std::string const& _path) -> resolver_t& {
    return (*this);
}

auto zpt::system_events::resolver_t::resolve(zpt::message _received,
                                             zpt::events::initializer_t _initializer) const
  -> std::list<zpt::event> {
    return {};
}

auto zpt::system_events::resolver_t::search(std::string const& _path,
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
