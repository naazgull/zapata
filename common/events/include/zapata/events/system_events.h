#pragma once

#include <zapata/json.h>
#include <zapata/events/dispatcher.h>
#include <zapata/events/resolver.h>

namespace zpt {
enum system_event_type : long {
    BOOTING = 0, //
    FINISHED_BOOT,
    MINION_BOOT_RECEIVED,
    MINION_HELLO_RECEIVED,
    REGISTERED_REMOTE_SERVICE,
    MINION_SHUTDOWN_RECEIVED,
    UNREGISTERED_REMOTE_SERVICE,
    SHUTTING_DOWN,
    EXITING,
    END_EVENTS
};

class system_event {
  public:
    system_event(zpt::system_event_type _type, zpt::json const& _data = zpt::undefined);
    ~system_event() = default;

    auto initialize(zpt::event_initialization& _init) -> void;
    auto blocked() const -> bool;
    auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(zpt::failed_expectation const& _e,
                     zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;

  private:
    zpt::system_event_type __type;
    zpt::json __data;
};

namespace system_events {
class resolver_t : public zpt::events::resolver_t {
  public:
    resolver_t() = default;
    resolver_t(resolver_t const&) = delete;
    resolver_t(resolver_t&&) = delete;
    virtual ~resolver_t() = default;

    auto operator=(resolver_t const&) -> resolver_t& = delete;
    auto operator=(resolver_t&&) -> resolver_t& = delete;

    using zpt::events::resolver_t::add;
    using zpt::events::resolver_t::remove;
    auto add(zpt::json const& _service_description) -> resolver_t& override;
    auto add(zpt::message _sent, zpt::events::resolver_callback callback) -> resolver_t& override;
    auto add(zpt::performative _performative,
             zpt::json const& _id,
             zpt::json const& _metadata,
             zpt::events::resolver_callback _callback) -> resolver_t& override;
    auto remove(zpt::message _sent) -> resolver_t& override;
    auto remove(zpt::performative _performative, zpt::json const& _id) -> resolver_t& override;
    auto resolve(zpt::message _received,
                 zpt::events::initializer_t _initializer) const -> std::list<zpt::event> override;
    auto search(zpt::json const& _id,
                std::string const& _provider_id = "") const -> zpt::json override;
    auto list(std::string const& _provider_id = "") const -> zpt::json override;
    auto register_provider(zpt::json const& _provider) -> resolver_t& override;
    auto unregister_provider(std::string const& _id) -> resolver_t& override;
    auto get_provider(std::string const& _id) const -> zpt::json override;
    auto clear() -> resolver_t& override;

  private:
    std::map<unsigned int, std::vector<zpt::events::resolver_callback>> __callbacks;
};
} // namespace system_events
auto SYSTEM_EVENTS_RESOLVER() -> zpt::events::resolver;
} // namespace zpt
