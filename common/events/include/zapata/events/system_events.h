#pragma once

#include <zapata/json.h>
#include <zapata/events/dispatcher.h>

namespace zpt {
enum system_event_type : unsigned int {
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
    system_event(zpt::system_event_type _type, zpt::json const& _data = zpt::undefined);
    ~system_event() = default;

    auto initialize(zpt::event_initialization& _init) -> void;
    auto blocked() const -> bool;
    auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(zpt::failed_expectation const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;

  private:
    zpt::system_event_type __type;
    zpt::json __data;
};

template<zpt::events::Operation T>
auto register_system_listener(zpt::system_event_type _type) -> void;
template<zpt::events::Operation T>
auto unregister_system_listener(zpt::system_event_type _type) -> void;

} // namespace zpt
