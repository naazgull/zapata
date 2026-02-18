/**
 * @file resolver.h
 * @brief Event resolver interface for mapping messages to event handlers.
 *
 * Provides the abstract resolver interface that maps incoming messages to
 * registered event handlers (Operations). The resolver is the routing core
 * of the event system, matching performatives and URIs to callbacks.
 *
 * @see zpt::events::dispatcher
 * @see zpt::system_events::resolver_t
 */

#pragma once

#include <list>
#include <zapata/events/dispatcher.h>
#include <zapata/ontology.h>

namespace zpt {
namespace events {
namespace zpt {
namespace events {

/** @brief Function type for event initialization callbacks. */
using initializer_t = std::function<void(zpt::event _event)>;
/** @brief Function type for resolver callbacks that create events from messages. */
using resolver_callback = std::function<zpt::event(zpt::message, zpt::events::initializer_t)>;

/**
 * @brief Abstract base class for event resolvers.
 *
 * Maps incoming messages to registered event handlers. Implementations
 * maintain a registry of handlers keyed by performative and URI pattern,
 * and resolve incoming messages to matching events.
 *
 * @par Handler Registration
 * Handlers are registered via `add<T>()` where T satisfies the Operation
 * concept. The resolver stores a callback that creates events of type T
 * when a matching message arrives.
 *
 * @par Provider Management
 * Resolvers also support service provider registration for distributed
 * service discovery.
 *
 * @see zpt::system_events::resolver_t for system event implementation
 */
class resolver_t {
  public:
    resolver_t() = default;
    virtual ~resolver_t() = default;

    /**
     * @brief Registers an Operation handler for any performative.
     * @tparam T Operation type (must satisfy Operation concept).
     * @param _id Handler identifier (typically a URI pattern).
     * @param _metadata Optional metadata for the handler.
     */
    template<zpt::events::Operation T>
    auto add(zpt::json const& _id, zpt::json const& _metadata = zpt::undefined) -> resolver_t&;

    /**
     * @brief Registers an Operation handler for a specific performative.
     * @tparam T Operation type.
     * @param _performative HTTP method to match (Get, Post, etc.).
     * @param _id Handler identifier.
     * @param _metadata Optional metadata.
     */
    template<zpt::events::Operation T>
    auto add(zpt::performative _performative,
             zpt::json const& _id,
             zpt::json const& _metadata = zpt::undefined) -> resolver_t&;
    /** @brief Registers a handler from a service description. */
    virtual auto add(zpt::json const& _service_description) -> resolver_t& = 0;
    /** @brief Registers a callback for a sent message (request/reply pairing). */
    virtual auto add(zpt::message _sent,
                     zpt::call_context::ptr _context,
                     zpt::events::resolver_callback callback) -> resolver_t& = 0;
    /** @brief Registers a raw callback for a performative and ID. */
    virtual auto add(zpt::performative _performtive,
                     zpt::json const& _id,
                     zpt::json const& _metadata,
                     zpt::events::resolver_callback _callback) -> resolver_t& = 0;

    /** @brief Removes an Operation handler (any performative). */
    template<zpt::events::Operation T>
    auto remove(zpt::json const& _id) -> resolver_t&;
    /** @brief Removes an Operation handler for a specific performative. */
    template<zpt::events::Operation T>
    auto remove(zpt::performative _performative, zpt::json const& _id) -> resolver_t&;
    /** @brief Removes a message-based handler. */
    virtual auto remove(zpt::message _sent) -> resolver_t& = 0;
    /** @brief Removes a handler by performative and ID. */
    virtual auto remove(zpt::performative _performtive, zpt::json const& _id) -> resolver_t& = 0;

    /**
     * @brief Resolves a message to matching event handlers.
     * @param _received Incoming message to resolve.
     * @param _initializer Callback to initialize created events.
     * @return List of events matching the message.
     */
    virtual auto resolve(zpt::message _received, initializer_t _initializer) const
      -> std::list<zpt::event> = 0;

    /** @brief Searches for handlers matching an ID pattern. */
    virtual auto search(zpt::json const& _id, std::string const& _provider_id = "") const
      -> zpt::json = 0;
    /** @brief Lists all registered handlers. */
    virtual auto list(std::string const& _provider_id = "") const -> zpt::json = 0;
    /** @brief Registers a service provider. */
    virtual auto register_provider(zpt::json const& _service_description) -> resolver_t& = 0;
    /** @brief Unregisters a service provider. */
    virtual auto unregister_provider(std::string const& _id) -> resolver_t& = 0;
    /** @brief Retrieves a service provider by ID. */
    virtual auto get_provider(std::string const& _id) const -> zpt::json = 0;
    /** @brief Removes all registered handlers and providers. */
    virtual auto clear() -> resolver_t& = 0;
};
/** @brief Shared pointer type for resolvers. */
using resolver = std::shared_ptr<resolver_t>;

template<zpt::events::Operation T>
auto make_callback(zpt::message _received,
                   zpt::call_context::ptr _context,
                   zpt::events::initializer_t _initializer) -> zpt::event;
} // namespace events
} // namespace zpt

template<zpt::events::Operation T>
auto zpt::events::make_callback(zpt::message _received,
                                zpt::call_context::ptr _context,
                                zpt::events::initializer_t _initializer) -> zpt::event {

    zpt::event _event;
    if (_context != nullptr) { _event = zpt::make_event<T>(_received, _context); }
    else { _event = zpt::make_event<T>(_received); }
    _initializer(_event);
    return _event;
}

template<zpt::events::Operation T>
auto zpt::events::resolver_t::add(zpt::json const& _id, zpt::json const& _metadata) -> resolver_t& {
    return this->add<T>(zpt::Performative_end, _id, _metadata);
}

template<zpt::events::Operation T>
auto zpt::events::resolver_t::add(zpt::performative _performative,
                                  zpt::json const& _id,
                                  zpt::json const& _metadata) -> resolver_t& {
    return this->add(_performative, _id, _metadata, zpt::events::make_callback<T>);
}

template<zpt::events::Operation T>
auto zpt::events::resolver_t::remove(zpt::json const& _id) -> resolver_t& {
    return this->remove<T>(zpt::Performative_end, _id);
}

template<zpt::events::Operation T>
auto zpt::events::resolver_t::remove(zpt::performative _performative, zpt::json const& _id)
  -> resolver_t& {
    return this->remove(_performative, _id);
}
