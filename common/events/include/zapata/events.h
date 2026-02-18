/*
  This is free and unencumbered software released into the public domain.
*/

/**
 * @file events.h
 * @brief Event-driven programming framework for asynchronous processing.
 *
 * This is the aggregate header for the events module. It provides a
 * dispatcher/consumer pattern for processing events asynchronously,
 * with support for event resolution and system lifecycle events.
 *
 * Key components:
 * - `zpt::events::dispatcher` - Queue-based event dispatch with worker consumers
 * - `zpt::events::resolver_t` - Maps incoming messages to event handlers
 * - `zpt::system_event` - Built-in system lifecycle events
 *
 * @par Example
 * @code
 * #include <zapata/events.h>
 *
 * // Create a dispatcher with 4 consumer threads
 * auto dispatcher = zpt::DISPATCHER(4);
 *
 * // Define an event operation
 * struct MyOperation {
 *     void initialize(zpt::event_initialization&) {}
 *     bool blocked() const { return false; }
 *     bool catch_error(std::exception const&, zpt::events::dispatcher::ptr) { return false; }
 *     bool catch_error(std::bad_alloc const&, zpt::events::dispatcher::ptr) { return false; }
 *     bool catch_error(zpt::failed_expectation const&, zpt::events::dispatcher::ptr) { return false; }
 *
 *     zpt::events::state operator()(zpt::events::dispatcher::ptr d) {
 *         // Process event...
 *         return zpt::events::finish;
 *     }
 * };
 *
 * // Trigger an event
 * dispatcher->trigger<MyOperation>();
 *
 * // Start processing
 * dispatcher->start_consumers();
 * dispatcher->trap();  // Block until shutdown
 * @endcode
 *
 * @see zpt::events::dispatcher
 * @see zpt::events::resolver_t
 */

#pragma once

#include <zapata/events/dispatcher.h>
#include <zapata/events/resolver.h>
#include <zapata/events/system_events.h>
