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
    minion_boot(zpt::message _received);
    ~minion_boot() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

/**
 * @brief Event operation for handling worker shutdown signals.
 *
 * Processes shutdown messages from worker processes leaving the cluster.
 */
class minion_shutdown : public zpt::events::process {
  public:
    minion_shutdown(zpt::message _received);
    ~minion_shutdown() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

/**
 * @brief Event operation for handling worker hello handshakes.
 *
 * Processes hello messages for worker registration and capability exchange.
 */
class minion_hello : public zpt::events::process {
  public:
    minion_hello(zpt::message _received);
    ~minion_hello() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

/**
 * @brief Event operation for listing registered services.
 *
 * Handles requests for the list of registered service endpoints.
 */
class services_list : public zpt::events::process {
  public:
    services_list(zpt::message _received);
    ~services_list() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

namespace services {
/** @brief Broadcasts service registration to connected nodes. */
auto broadcast(std::string const& _path, zpt::json const& _config) -> void;
}
} // namespace rest
} // namespace zpt
