/**
 * @file services.h
 * @brief Built-in REST engine service event operations.
 *
 * Defines event operations for worker (minion) lifecycle management
 * and service listing in the distributed REST engine.
 */

#include <iostream>
#include <zapata/rest.h>

namespace zpt {
namespace rest {

/**
 * @brief Event operation for handling worker boot signals.
 *
 * Processes boot messages from worker processes joining the cluster.
 */
class minion_boot : public zpt::events::process {
  public:
    using zpt::events::process::process;
    /**
     * @brief Destructor.
     * @return void (destructors implicitly clean up the object).
     */
    ~minion_boot() = default;
    /**
     * @brief Returns false (boot processing is never blocked).
     * @return False.
     */
    auto blocked() const -> bool;
    /**
     * @brief Registers the booting worker and its services.
     * @param _dispatcher The event dispatcher for triggering events.
     * @return Event state indicating completion.
     */
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

/**
 * @brief Event operation for handling worker shutdown signals.
 *
 * Processes shutdown messages from worker processes leaving the cluster.
 */
class minion_shutdown : public zpt::events::process {
  public:
    using zpt::events::process::process;
    /**
     * @brief Destructor.
     * @return void (destructors implicitly clean up the object).
     */
    ~minion_shutdown() = default;
    /**
     * @brief Returns false (shutdown processing is never blocked).
     * @return False.
     */
    auto blocked() const -> bool;
    /** @brief Unregisters the shutting-down worker and its services.
     * @param _dispatcher The event dispatcher for triggering events.
     * @return Event state indicating completion. */
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

/**
 * @brief Event operation for handling worker hello handshakes.
 *
 * Processes hello messages for worker registration and capability exchange.
 */
class minion_hello : public zpt::events::process {
  public:
    using zpt::events::process::process;
    /**
     * @brief Destructor.
     * @return void (destructors implicitly clean up the object).
     */
    ~minion_hello() = default;
    /**
     * @brief Returns false (hello processing is never blocked).
     * @return False.
     */
    auto blocked() const -> bool;
    /** @brief Processes the hello handshake and exchanges capabilities.
     * @param _dispatcher The event dispatcher for triggering events.
     * @return Event state indicating completion. */
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

/**
 * @brief Event operation for handling worker state query requests.
 *
 * Responds to state requests with current memory pool and queue
 * diagnostics for the worker.
 */
class minion_state : public zpt::events::process {
  public:
    using zpt::events::process::process;
    /**
     * @brief Destructor.
     * @return void (destructors implicitly clean up the object).
     */
    ~minion_state() = default;
    /**
     * @brief Returns false (state queries are never blocked).
     * @return False.
     */
    auto blocked() const -> bool;
    /** @brief Responds with current worker diagnostics.
     * @param _dispatcher The event dispatcher for triggering events.
     * @return Event state indicating completion. */
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

/**
 * @brief Event operation for listing registered services.
 *
 * Handles requests for the list of registered service endpoints.
 */
class services_list : public zpt::events::process {
  public:
    using zpt::events::process::process;
    /**
     * @brief Destructor.
     * @return void (destructors implicitly clean up the object).
     */
    ~services_list() = default;
    /**
     * @brief Returns false (listing is never blocked).
     * @return False.
     */
    auto blocked() const -> bool;
    /** @brief Responds with the list of registered services.
     * @param _dispatcher The event dispatcher for triggering events.
     * @return Event state indicating completion. */
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

namespace services {
/**
 * @brief Broadcasts service registration to connected nodes.
 * @param _path Service path identifier.
 * @param _config Service configuration JSON.
 * @return void
 */
auto broadcast(std::string const& _path, zpt::json const& _config) -> void;
} // namespace services
} // namespace rest
} // namespace zpt
