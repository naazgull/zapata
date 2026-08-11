/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file rest.h
 * @brief REST resolver implementation.
 *
 * Implements the event resolver interface for REST API routing.
 * Routes incoming HTTP requests to registered handlers based on
 * URI patterns and HTTP methods (performatives).
 *
 * @see zpt::rest::resolver_t
 */

#pragma once

#include <zapata/catalog.h>
#include <zapata/events.h>
#include <zapata/ontology.h>
#include <zapata/rest/pending_messages.h>
#include <zapata/startup.h>
#include <zapata/transport.h>
#include <zapata/transport/engine.h>

namespace zpt {
namespace rest {

/**
 * @brief REST API request resolver.
 *
 * Routes incoming HTTP requests to registered handlers based on
 * URI patterns and HTTP methods. Supports service discovery and
 * distributed node coordination.
 *
 * @par Example
 * @code
 * auto resolver = zpt::REST_RESOLVER(config);
 *
 * // Register a handler
 * resolver->add(zpt::Get, "/api/users/:id", {},
 *     [](zpt::message req) -> zpt::events::resolver_callback::result_type {
 *         return zpt::make_event<MyHandler>(req);
 *     });
 *
 * // Add to transport engine
 * zpt::TRANSPORT_ENGINE(config)->add_resolver(resolver);
 * @endcode
 */
class resolver_t : public zpt::events::resolver_t {
  public:
    /**
     * @brief Constructs the REST resolver with the given configuration.
     * @param _rest_config Configuration JSON object for the REST resolver.
     * @return void (constructors implicitly initialize the object).
     */
    resolver_t(zpt::json _rest_config);
    resolver_t(resolver_t const&) = delete;
    resolver_t(resolver_t&&) = delete;
    /**
     * @brief Destructor.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~resolver_t() = default;

    auto operator=(resolver_t const&) -> resolver_t& = delete;
    auto operator=(resolver_t&&) -> resolver_t& = delete;

    using zpt::events::resolver_t::add;
    using zpt::events::resolver_t::remove;
    /**
     * @brief Registers a REST service from its JSON description.
     * @param _service_description Service description JSON object.
     * @return Reference to this resolver.
     */
    auto add(zpt::json const& _service_description) -> resolver_t& override;
    /**
     * @brief Registers a callback for a sent message's reply.
     * @param _sent The outbound message to track.
     * @param _context The call context for the request.
     * @param _callback Callback to invoke when the reply arrives.
     * @return Reference to this resolver.
     */
    auto add(zpt::message _sent,
             zpt::call_context::ptr _context,
             zpt::events::resolver_callback callback) -> resolver_t& override;
    /**
     * @brief Registers a callback for a performative/URI pattern combination.
     * @param _performative HTTP performative (e.g., zpt::Get, zpt::Post).
     * @param _id URI pattern or handler identifier.
     * @param _metadata Optional handler metadata.
     * @param _callback Resolver callback to invoke on match.
     * @return Reference to this resolver.
     */
    auto add(zpt::performative _performative,
             zpt::json const& _id,
             zpt::json const& _metadata,
             zpt::events::resolver_callback _callback) -> resolver_t& override;
    /**
     * @brief Removes the callback registered for a sent message.
     * @param _sent The message whose callback should be removed.
     * @return Reference to this resolver instance.
     */
    auto remove(zpt::message _sent) -> resolver_t& override;
    /**
     * @brief Removes callback for a performative/URI combination.
     * @param _performative The HTTP performative (GET, POST, etc.).
     * @param _id The URI pattern or identifier.
     * @return Reference to this resolver instance.
     */
    auto remove(zpt::performative _performative, zpt::json const& _id) -> resolver_t& override;
    /**
     * @brief Resolves an incoming request to matching REST handlers.
     * @param _received Incoming message.
     * @param _initializer Event initializer function.
     * @return List of resolved events.
     */
    auto resolve(zpt::message _received, zpt::events::initializer_t _initializer) const
      -> std::list<zpt::event> override;
    /**
     * @brief Searches for registered services matching the given URI pattern.
     * @param _id URI pattern to search for.
     * @param _provider_id Optional provider filter.
     * @return JSON array of matching services.
     */
    auto search(zpt::json const& _id, std::string const& _provider_id = "") const
      -> zpt::json override;
    /**
     * @brief Lists all registered REST services.
     * @param _provider_id Optional provider filter.
     * @return JSON array of all registered services.
     */
    auto list(std::string const& _provider_id = "") const -> zpt::json override;
    /**
     * @brief Registers a service provider node.
     * @param _provider Provider description JSON.
     * @return Reference to this resolver.
     */
    auto register_provider(zpt::json const& _provider) -> zpt::rest::resolver_t& override;
    /**
     * @brief Unregisters a service provider node by ID.
     * @param _id Provider ID to remove.
     * @return Reference to this resolver.
     */
    auto unregister_provider(std::string const& _id) -> zpt::rest::resolver_t& override;
    /**
     * @brief Returns provider metadata by ID.
     * @param _id Provider ID to look up.
     * @return JSON object with provider details.
     */
    auto get_provider(std::string const& _id) const -> zpt::json override;

  private:
    /** @brief Catalog mapping URI patterns to callback indices. */
    zpt::catalog<std::string, zpt::json>::ptr __catalog{ nullptr };
    /** @brief Vector of registered REST callback handlers. */
    std::vector<zpt::events::resolver_callback> __callbacks;
    /** @brief Store for pending request/response callbacks. */
    mutable zpt::rest::pending_messages __pending_requests;
    /** @brief Configuration passed to the resolver at construction. */
    zpt::json __configuration;
};
} // namespace rest

/**
 * @brief Returns the global REST resolver instance.
 * @param _config Optional configuration (used only on first call).
 * @return Shared pointer to the REST resolver.
 */
auto REST_RESOLVER(zpt::json _config = nullptr) -> zpt::events::resolver;
} // namespace zpt
