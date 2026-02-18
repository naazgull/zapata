/**
 * @file system_events.h
 * @brief System lifecycle events and their resolver.
 *
 * Defines the system event types (boot, shutdown, etc.) and provides a
 * specialized resolver for mapping system event types to handlers.
 *
 * @see zpt::events::dispatcher
 * @see zpt::events::resolver_t
 */

#pragma once

#include <zapata/events/dispatcher.h>
#include <zapata/events/resolver.h>
#include <zapata/json.h>

namespace zpt {

/**
 * @brief System lifecycle event types.
 *
 * Enumerates the phases of the application lifecycle that can trigger
 * system events. Handlers can be registered for each event type.
 */
enum system_event_type : long long {
    BOOTING = 0,                 ///< Application is starting up.
    FINISHED_BOOT,               ///< Boot sequence completed.
    MINION_BOOT_RECEIVED,        ///< Worker process boot signal received.
    MINION_HELLO_RECEIVED,       ///< Worker process hello handshake received.
    REGISTERED_REMOTE_SERVICE,   ///< Remote service registered.
    MINION_SHUTDOWN_RECEIVED,    ///< Worker process shutdown signal received.
    UNREGISTERED_REMOTE_SERVICE, ///< Remote service unregistered.
    SHUTTING_DOWN,               ///< Application is shutting down.
    EXITING,                     ///< Application is exiting.
    END_EVENTS                   ///< Sentinel value.
};

/**
 * @brief Operation for system lifecycle events.
 *
 * Implements the Operation concept for system events. Each system event
 * carries a type and optional data, and is dispatched through the resolver.
 *
 * @see zpt::system_event_type
 */
class system_event {
  public:
    system_event() = default;
    /** @brief Constructs from an incoming message. */
    system_event(zpt::message _received);
    /** @brief Constructs from an incoming message and context. */
    system_event(zpt::message _received, zpt::call_context::ptr _context);
    /** @brief Constructs with a specific event type and optional data. */
    system_event(zpt::system_event_type _type, zpt::json const& _data = zpt::undefined);
    ~system_event() = default;

    virtual auto initialize(zpt::event_initialization& _init) -> void final;
    virtual auto blocked() const -> bool;
    virtual auto authorized() const -> bool;
    virtual auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool;
    virtual auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool;
    virtual auto catch_error(zpt::failed_expectation const& _e,
                             zpt::events::dispatcher::ptr _dispatcher) -> bool;
    virtual auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;

  protected:
    zpt::system_event_type __type; ///< The lifecycle event type.
    zpt::message __received;       ///< Associated message data.
};

namespace system_events {

/**
 * @brief Resolver for system lifecycle events.
 *
 * Specialized resolver that maps `zpt::system_event_type` values to
 * registered Operation handlers. Used internally by the boot engine
 * to dispatch lifecycle events.
 *
 * @see zpt::events::resolver_t
 * @see zpt::system_event
 */
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

    /**
     * @brief Registers an Operation handler for a system event type.
     * @tparam T Operation type (must satisfy Operation concept).
     * @param _type System event type to handle.
     */
    template<zpt::events::Operation T>
    auto add(zpt::system_event_type _type) -> resolver_t&;
    auto add(zpt::json const& _service_description) -> resolver_t& override;
    auto add(zpt::message _sent,
             zpt::call_context::ptr _context,
             zpt::events::resolver_callback callback) -> resolver_t& override;
    auto add(zpt::performative _performative,
             zpt::json const& _id,
             zpt::json const& _metadata,
             zpt::events::resolver_callback _callback) -> resolver_t& override;
    template<zpt::events::Operation T>
    auto remove(zpt::system_event_type _type) -> resolver_t&;
    auto remove(zpt::message _sent) -> resolver_t& override;
    auto remove(zpt::performative _performative, zpt::json const& _id) -> resolver_t& override;
    auto resolve(zpt::message _received, zpt::events::initializer_t _initializer) const
      -> std::list<zpt::event> override;
    auto search(zpt::json const& _id, std::string const& _provider_id = "") const
      -> zpt::json override;
    auto list(std::string const& _provider_id = "") const -> zpt::json override;
    auto register_provider(zpt::json const& _provider) -> resolver_t& override;
    auto unregister_provider(std::string const& _id) -> resolver_t& override;
    auto get_provider(std::string const& _id) const -> zpt::json override;
    auto clear() -> resolver_t& override;

  private:
    std::map<zpt::system_event_type, std::map<zpt::json, zpt::events::resolver_callback>>
      __callbacks;
};
/** @brief Shared pointer type for system events resolver. */
using resolver = std::shared_ptr<resolver_t>;

/**
 * @brief Returns a unique JSON identifier for an Operation type.
 * @tparam T Operation type.
 * @return JSON containing the type's hash code.
 */
template<zpt::events::Operation T>
auto get_id() -> zpt::json;
} // namespace system_events

/**
 * @brief Returns the global system events resolver instance.
 * @return Shared pointer to the system events resolver.
 */
auto SYSTEM_EVENTS_RESOLVER() -> zpt::system_events::resolver;
} // namespace zpt

template<zpt::events::Operation T>
auto zpt::system_events::resolver_t::add(zpt::system_event_type _type) -> resolver_t& {
    this->add<T>(zpt::system_events::get_id<T>(), zpt::json{ static_cast<long long>(_type) });
    return (*this);
}

template<zpt::events::Operation T>
auto zpt::system_events::resolver_t::remove(zpt::system_event_type _type) -> resolver_t& {
    this->__callbacks[_type].erase(zpt::system_events::get_id<T>());
    return (*this);
}

template<zpt::events::Operation T>
auto zpt::system_events::get_id() -> zpt::json {
    return zpt::json{ typeid(T).hash_code() };
}
