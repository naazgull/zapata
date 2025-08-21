#pragma once

namespace zpt {
enum system_event_type : unsigned int { BOOTING = 0, FINISHED_BOOT, SHUTTING_DOWN, EXTING };

class system_event {
    system_event(zpt::system_event_type _type);
    ~system_event() = default;

    auto initialize(zpt::event_initialization& _init) -> void;
    auto blocked() const -> bool;
    auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    auto catch_error(zpt::failed_expectation const& _e, zpt::events::dispatcher::ptr _dispatcher);
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;

  private:
    zpt::system_event_type type;
};

auto register_system_listener(zpt::system_event_type type, zpt::system_listener) -> void;

} // namespace zpt
