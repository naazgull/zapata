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
 * @file dispatcher.h
 * @brief Event dispatcher with consumer thread pool.
 *
 * Provides a queue-based event dispatch system where events are enqueued
 * by producers and processed by a pool of consumer threads.
 *
 * @see zpt::events::dispatcher
 * @see zpt::abstract_event
 */

#pragma once

#include <memory>
#include <zapata/allocator.h>
#include <zapata/base.h>
#include <zapata/json.h>
#include <zapata/lockfree.h>

namespace zpt {

/** @brief Forward declaration of abstract event interface. */
class abstract_event;
/** @brief Shared pointer type for events. */
using event = zpt::allocator<zpt::abstract_event>::unique_pointer;

/**
 * @brief Base class for event initialization data.
 *
 * Subclass this to pass initialization data to events when they are triggered.
 */
class event_initialization {
  public:
    /** @brief Shared pointer type for event initialization data. */
    using ptr = std::shared_ptr<event_initialization>;
};

/**
 * @brief Event system namespace.
 */
namespace events {

/**
 * @brief Event processing result states.
 */
enum state {
    retrigger = -2, ///< Re-queue event for another processing cycle
    ready = -1,     ///< Event is ready but not yet processed
    finish = 0,     ///< Event completed successfully
    abort = 1       ///< Event aborted (error occurred)
};

/**
 * @brief Event dispatcher with consumer thread pool.
 *
 * Manages a lock-free queue of events and a pool of consumer threads
 * that process events asynchronously. Events can be triggered from
 * any thread and will be processed by available consumers.
 *
 * @par Lifecycle
 * 1. Create dispatcher with `zpt::DISPATCHER(n_consumers, max_queue_size)`
 * 2. Register event handlers or initialization data
 * 3. Call `start_consumers()` to begin processing
 * 4. Trigger events with `trigger<T>(args...)`
 * 5. Call `trap()` to block until shutdown, or `stop_consumers()` to stop
 *
 * @par Example
 * @code
 * auto dispatcher = zpt::DISPATCHER(4);  // 4 consumers
 * dispatcher->start_consumers();
 * dispatcher->trigger<MyEvent>("some", "args");
 * dispatcher->trap();  // Block until shutdown
 * @endcode
 */
class dispatcher : public std::enable_shared_from_this<dispatcher> {
  public:
    using ptr = std::shared_ptr<dispatcher>;
    using weak_ptr = std::weak_ptr<dispatcher>;

    /**
     * @brief Constructs a dispatcher.
     * @param _name Dispatcher name (for logging).
     * @param _max_consumers Maximum consumer threads.
     * @param _max_queue_size Maximum number of elements allowed in the queue (resource management
     *                        cap).
     * @return Shared pointer to the dispatcher.
     */
    dispatcher(std::string const& _name, long _max_consumers, size_t _max_queue_size = 10000);
    /**
     * @brief Destructor. Stops consumers if running.
     *
     * Sets the shutdown flag, joins all consumer threads, and cleans up
     * internal state.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~dispatcher();

    /** @brief Sets initialization data passed to new events.
     * @param _event_init Event initialization data pointer.
     * @return Reference to this dispatcher. */
    auto set_event_initialization(zpt::event_initialization::ptr _event_init) -> dispatcher&;
    /**
     * @brief Starts consumer threads.
     * @param n_consumers Number to start (0 = use max_consumers).
     * @return Reference to this dispatcher.
     */
    auto start_consumers(long n_consumers = 0) -> dispatcher&;
    /** @brief Signals consumers to stop and waits for completion.
     * @return Reference to this dispatcher. */
    auto stop_consumers() -> dispatcher&;
    /** @brief Enqueues an existing event for processing.
     * @param _event Event to enqueue.
     * @return Reference to this dispatcher. */
    auto trigger(zpt::event _event) -> dispatcher&;
    /**
     * @brief Creates and enqueues an event.
     * @tparam T Event operation type (must satisfy Operation concept).
     * @param _args Arguments forwarded to T's constructor.
     * @return Reference to this dispatcher.
     */
    template<typename T, typename... Args>
    auto trigger(Args&&... _args) -> dispatcher&;
    /** @brief Blocks until dispatcher is shut down.
     * @return Reference to this dispatcher. */
    auto trap() -> dispatcher&;
    /** @brief Checks if shutdown has been initiated.
     * @return True if shutdown is in progress. */
    auto is_in_shutdown() -> bool;
    /** @brief Retrieves the dispatcher's internal state.
     * @return JSON object with dispatcher status (running, queue size, etc.). */
    auto get_state() const -> zpt::json;
    /** @brief Joins all dispatcher threads and waits for them to stop.
     * @return void. */
    static auto join_threads() -> void;

  public:
    /** @brief Lock-free queue holding pending events. */
    zpt::lf::queue<zpt::abstract_event> __queue;
    /** @brief Thread pool for processing events. */
    std::vector<std::thread> __consumers;
    /** @brief Flag indicating shutdown has been initiated. */
    zpt::padded_atomic<bool> __shutdown{ false };
    /** @brief Current number of running consumer threads. */
    zpt::padded_atomic<long> __running_consumers{ 0 };
    /** @brief Maximum number of consumer threads allowed. */
    long __max_consumers{ 2 };
    /** @brief Dispatcher name used for logging. */
    std::string __name{ "" };
    /** @brief Initialization data forwarded to new events on creation. */
    zpt::event_initialization::ptr __event_init{ nullptr };
    /** @brief Number of workers threads initialised by all dispatchers. */
    static inline std::atomic<unsigned int> __n_threads{ 0 };

    /**
     * @brief Main consumer loop: dequeues and processes events until shutdown.
     * @param _consumer_nr Consumer thread index (for logging).
     * @return None (runs until shutdown).
     */
    auto loop(long _consumer_nr) -> void;
};

/**
 * @brief C++20 concept defining the Operation interface for events.
 *
 * Any type used with `trigger<T>()` must satisfy this concept.
 *
 * Required methods:
 * - `initialize(event_initialization&)` - Called when event is created
 * - `blocked() -> bool` - Return true if event should wait
 * - `catch_error(exception, dispatcher)` - Handle errors, return true to retry
 * - `operator()(dispatcher) -> state` - Execute the event
 *
 * @tparam T The operation type to check.
 */
template<typename T>
concept Operation = requires(T t,
                             zpt::event_initialization& _i,
                             zpt::events::dispatcher::ptr _d,
                             std::exception const& _e,
                             std::bad_alloc const& _bae,
                             zpt::failed_expectation const& _fe) {
    { t.initialize(_i) } -> std::convertible_to<void>;
    { t.blocked() } -> std::convertible_to<bool>;
    { t.authorized() } -> std::convertible_to<bool>;
    { t.catch_error(_e, _d) } -> std::convertible_to<bool>;
    { t.catch_error(_bae, _d) } -> std::convertible_to<bool>;
    { t.catch_error(_fe, _d) } -> std::convertible_to<bool>;
    { t(_d) } -> std::convertible_to<zpt::events::state>;
};
} // namespace events

/**
 * @name abstract_event
 * @{
 */

/**
 * @brief Abstract base class for events processed by the dispatcher.
 *
 * Defines the interface that all events must implement. Use `zpt::event_t<T>`
 * or `zpt::make_event<T>()` to create concrete events from Operation types.
 *
 * @par Lifecycle
 * 1. An event is created via `make_event<T>()` and optionally initialized
 *    with `initialize()`.
 * 2. The dispatcher's consumer threads call `blocked()` to check if the
 *    event is ready for processing.
 * 3. When not blocked, `operator()` is invoked to execute the event logic.
 * 4. If an error occurs during processing, the appropriate `catch_error()`
 *    overload is called; returning true re-triggers the event.
 */
class abstract_event {
  public:
    abstract_event() = default;
    virtual ~abstract_event() = default;

    /** @brief Called when event is created with initialization data.
     * @param init_data Event initialization data. */
    virtual auto initialize(zpt::event_initialization& init_data) -> void = 0;
    /** @brief Returns true if event is blocked waiting for something.
     * @return True if event is blocked. */
    virtual auto blocked() const -> bool = 0;
    /** @brief Returns true if event is authorized to be invoked.
     * @return True if authorized. */
    virtual auto authorized() const -> bool = 0;
    /** @brief Handles a generic exception. Return true to re-trigger event.
     * @param _e The caught exception.
     * @param _dispatcher Event dispatcher reference.
     * @return True to re-trigger the event. */
    virtual auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool = 0;
    /** @brief Handles memory allocation failure. Return true to retry.
     * @param _e The caught std::bad_alloc exception.
     * @param _dispatcher Event dispatcher reference.
     * @return True to retry the event. */
    virtual auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool = 0;
    /** @brief Handles expectation failure. Return true to re-trigger.
     * @param _e The caught failed_expectation exception.
     * @param _dispatcher Event dispatcher reference.
     * @return True to re-trigger the event. */
    virtual auto catch_error(zpt::failed_expectation const& _e,
                             zpt::events::dispatcher::ptr _dispatcher) -> bool = 0;
    /** @brief Executes the event operation.
     * @param _dispatcher Event dispatcher reference.
     * @return Processing state (finish, abort, retrigger). */
    virtual auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state = 0;
};
/**@}*/
using event = zpt::allocator<zpt::abstract_event>::unique_pointer;

/**
 * @brief Type-erasing wrapper for Operation types.
 *
 * Wraps any type satisfying the Operation concept and provides the
 * abstract_event interface. Created via `zpt::make_event<T>()`.
 *
 * @tparam T Operation type (must satisfy Operation concept).
 */
template<zpt::events::Operation T>
class event_t : public zpt::abstract_event {
  public:
    /**
     * @brief Constructs event, forwarding args to underlying Operation.
     * @tparam Args Constructor argument types for the underlying Operation.
     * @param _args Arguments forwarded to the underlying Operation's constructor.
     */
    template<typename... Args>
    event_t(Args&&... _args);
    /** @brief Destructor. */
    virtual ~event_t() override = default;

    /** @brief Access underlying operation.
     * @return Reference to the underlying Operation. */
    auto operator*() -> T&;
    /** @brief Access underlying operation (const).
     * @return Const reference to the underlying Operation. */
    auto operator*() const -> T const&;
    /** @brief Delegates to underlying Operation's initialize().
     * @param init_data Event initialization data. */
    virtual auto initialize(zpt::event_initialization& init_data) -> void override final;
    /** @brief Delegates to underlying Operation's blocked().
     * @return Result of underlying Operation's blocked(). */
    virtual auto blocked() const -> bool override final;
    /** @brief Delegates to underlying Operation's authorized().
     * @return Result of underlying Operation's authorized(). */
    virtual auto authorized() const -> bool override final;
    /** @brief Delegates to underlying Operation's catch_error() for generic exceptions.
     * @param _e The caught exception.
     * @param _dispatcher Event dispatcher reference.
     * @return Result of underlying Operation's catch_error(). */
    virtual auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool override final;
    /** @brief Delegates to underlying Operation's catch_error() for allocation failures.
     * @param _e The caught std::bad_alloc exception.
     * @param _dispatcher Event dispatcher reference.
     * @return Result of underlying Operation's catch_error(). */
    virtual auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool override final;
    /** @brief Delegates to underlying Operation's catch_error() for expectation failures.
     * @param _e The caught failed_expectation exception.
     * @param _dispatcher Event dispatcher reference.
     * @return Result of underlying Operation's catch_error(). */
    virtual auto catch_error(zpt::failed_expectation const& _e,
                             zpt::events::dispatcher::ptr _dispatcher) -> bool override final;
    /** @brief Delegates to underlying Operation's operator().
     * @param _dispatcher Event dispatcher reference.
     * @return Result of underlying Operation's operator(). */
    virtual auto operator()(zpt::events::dispatcher::ptr _dispatcher)
      -> zpt::events::state override final;

  private:
    T __underlying;
};

/**
 * @brief Creates an event from an existing Operation instance.
 * @tparam T Operation type.
 * @param _operator Operation instance (will be copied).
 * @return Shared pointer to the event.
 */
template<zpt::events::Operation T>
auto make_event(T _operator) -> zpt::event;

/**
 * @brief Creates an event with forwarded constructor arguments.
 * @tparam T Operation type.
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to T's constructor.
 * @return Shared pointer to the event.
 */
template<zpt::events::Operation T, typename... Args>
auto make_event(Args&&... _args) -> zpt::event;

/**
 * @brief Factory function to create a dispatcher.
 * @param _consumers Number of consumer threads.
 * @param _max_queue_size Maximum size for event queue.
 * @return Shared pointer to the dispatcher.
 */
auto DISPATCHER(long int _consumers = 0, size_t _max_queue_size = 0)
  -> zpt::events::dispatcher::ptr;

/**
 * @brief Casts an event to access its underlying Operation.
 * @tparam T Expected Operation type.
 * @param _event Event to cast.
 * @return Reference to the underlying Operation.
 */
template<zpt::events::Operation T>
auto event_cast(zpt::event& _event) -> T&;
} // namespace zpt

template<zpt::events::Operation T>
template<typename... Args>
zpt::event_t<T>::event_t(Args&&... _args)
  : __underlying{ std::forward<Args>(_args)... } {}

template<zpt::events::Operation T>
auto zpt::event_t<T>::operator*() -> T& {
    return this->__underlying;
}

template<zpt::events::Operation T>
auto zpt::event_t<T>::operator*() const -> T const& {
    return this->__underlying;
}

template<zpt::events::Operation T>
auto zpt::event_t<T>::initialize(zpt::event_initialization& init_data) -> void {
    return this->__underlying.initialize(init_data);
}

template<zpt::events::Operation T>
auto zpt::event_t<T>::blocked() const -> bool {
    return this->__underlying.blocked();
}

template<zpt::events::Operation T>
auto zpt::event_t<T>::authorized() const -> bool {
    return this->__underlying.authorized();
}

template<zpt::events::Operation T>
auto zpt::event_t<T>::catch_error(std::exception const& _e,
                                  zpt::events::dispatcher::ptr _dispatcher) -> bool {
    return this->__underlying.catch_error(_e, _dispatcher);
}

template<zpt::events::Operation T>
auto zpt::event_t<T>::catch_error(std::bad_alloc const& _e,
                                  zpt::events::dispatcher::ptr _dispatcher) -> bool {
    return this->__underlying.catch_error(_e, _dispatcher);
}

template<zpt::events::Operation T>
auto zpt::event_t<T>::catch_error(zpt::failed_expectation const& _e,
                                  zpt::events::dispatcher::ptr _dispatcher) -> bool {
    return this->__underlying.catch_error(_e, _dispatcher);
}

template<zpt::events::Operation T>
auto zpt::event_t<T>::operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state {
    return this->__underlying(_dispatcher);
}

template<zpt::events::Operation T>
auto zpt::make_event(T _operator) -> zpt::event {
    return zpt::allocate_unique<zpt::event_t<T>>(_operator);
}

template<zpt::events::Operation T, typename... Args>
auto zpt::make_event(Args&&... _args) -> zpt::event {
    return zpt::allocate_unique<zpt::event_t<T>>(std::forward<Args>(_args)...);
}

template<typename T, typename... Args>
auto zpt::events::dispatcher::trigger(Args&&... _args) -> dispatcher& {
    auto _event = zpt::make_event<T>(std::forward<Args>(_args)...);
    if (this->__event_init != nullptr) { _event->initialize(*this->__event_init); }
    this->trigger(std::move(_event));
    return (*this);
}

template<zpt::events::Operation T>
auto zpt::event_cast(zpt::event& _event) -> T& {
    return *static_cast<zpt::event_t<T>&>(*_event.get());
}
