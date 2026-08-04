# Events API Reference

This document provides the API reference for the Zapata event dispatch system.

## Headers

```cpp
#include <zapata/events.h>           // All event types
#include <zapata/events/dispatcher.h> // Dispatcher only
#include <zapata/events/resolver.h>   // Resolver interface
#include <zapata/ontology.h>          // Message types
#include <zapata/transport/engine.h>  // Process base class
```

---

## Namespace: `zpt::events`

Event dispatch and resolution types.

---

## Enum: `zpt::events::state`

Event processing result codes.

| Value | Description |
|-------|-------------|
| `retrigger` (-2) | Re-queue event for another processing cycle |
| `ready` (-1) | Event ready but not yet processed |
| `finish` (0) | Event completed successfully |
| `abort` (1) | Event aborted due to error |

---

## Concept: `zpt::events::Operation`

C++20 concept defining the interface for event operation types.

### Required Methods

```cpp
template<typename T>
concept Operation = requires(T t,
                             zpt::event_initialization& _i,
                             zpt::events::dispatcher::ptr _d) {
    { t.initialize(_i) } -> std::convertible_to<void>;
    { t.blocked() } -> std::convertible_to<bool>;
    { t.authorized() } -> std::convertible_to<bool>;
    { t.catch_error(std::declval<std::exception const&>(), _d) } -> std::convertible_to<bool>;
    { t.catch_error(std::declval<std::bad_alloc const&>(), _d) } -> std::convertible_to<bool>;
    { t.catch_error(std::declval<zpt::failed_expectation const&>(), _d) } -> std::convertible_to<bool>;
    { t(_d) } -> std::convertible_to<zpt::events::state>;
};
```

### Method Descriptions

| Method | Description |
|--------|-------------|
| `initialize(event_initialization&)` | Called when event is created |
| `blocked() -> bool` | Return true if event should wait (re-queued) |
| `authorized() -> bool` | Return true if event is authorized to execute |
| `catch_error(exception, dispatcher) -> bool` | Handle errors; return true to retry |
| `operator()(dispatcher) -> state` | Execute the event operation |

### Example Implementation

```cpp
struct MyOperation {
    std::string data;

    MyOperation(std::string d) : data(std::move(d)) {}

    void initialize(zpt::event_initialization& init) {}
    bool blocked() const { return false; }
    bool authorized() const { return true; }

    bool catch_error(std::exception const& e, zpt::events::dispatcher::ptr d) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
    bool catch_error(std::bad_alloc const& e, zpt::events::dispatcher::ptr d) { return true; }
    bool catch_error(zpt::failed_expectation const& e, zpt::events::dispatcher::ptr d) { return false; }

    zpt::events::state operator()(zpt::events::dispatcher::ptr d) {
        std::cout << "Processing: " << data << std::endl;
        return zpt::events::finish;
    }
};
```

### Using zpt::events::process

The easiest way to implement an operation is to extend `zpt::events::process`:

```cpp
class my_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        // this->received() gives access to the request
        // this->to_send() gives the response message
        this->to_send()->status(200)->body() = zpt::json{ "status", "ok" };
        return zpt::events::finish;
    }
};
```

---

## Class: `zpt::events::dispatcher`

Event dispatcher with consumer thread pool.

### Type Aliases

```cpp
using ptr = std::shared_ptr<dispatcher>;
```

### Constructor

```cpp
dispatcher(std::string const& _name, long _max_consumers, long _max_producers);
```

**Parameters:**
- `_name` - Dispatcher name for logging
- `_max_consumers` - Maximum consumer threads
- `_max_producers` - Maximum producer threads (for queue sizing)

### Methods

#### `set_event_initialization`

```cpp
auto set_event_initialization(zpt::event_initialization::ptr _event_init) -> dispatcher&;
```

Sets initialization data passed to all new events.

#### `start_consumers`

```cpp
auto start_consumers(long n_consumers = 0) -> dispatcher&;
```

Starts consumer threads.

**Parameters:**
- `n_consumers` - Number to start (0 = max_consumers)

#### `stop_consumers`

```cpp
auto stop_consumers() -> dispatcher&;
```

Signals consumers to stop and waits for them to complete.

#### `trigger`

```cpp
auto trigger(zpt::event _event) -> dispatcher&;

template<typename T, typename... Args>
auto trigger(Args&&... _args) -> dispatcher&;
```

Enqueues an event for processing.

**Template version:** Creates an event of type T with forwarded arguments.

#### `trap`

```cpp
auto trap() -> dispatcher&;
```

Blocks the calling thread until the dispatcher is shut down.

#### `is_in_shutdown`

```cpp
auto is_in_shutdown() -> bool;
```

Returns true if shutdown has been initiated.

---

## Class: `zpt::abstract_event`

Abstract base class for dispatchable events.

### Methods

| Method | Description |
|--------|-------------|
| `initialize(event_initialization&)` | Initialize from shared data |
| `blocked() -> bool` | Check if blocked waiting |
| `authorized() -> bool` | Check authorization |
| `catch_error(exception, dispatcher) -> bool` | Handle error |
| `operator()(dispatcher) -> state` | Execute event |

---

## Class Template: `zpt::event_t<T>`

Type-erasing wrapper for Operation types.

### Constructor

```cpp
template<typename... Args>
event_t(Args&&... _args);
```

Constructs the underlying Operation with forwarded arguments.

### Methods

| Method | Description |
|--------|-------------|
| `operator*() -> T&` | Access underlying operation |

---

## Factory Functions

### `zpt::DISPATCHER`

```cpp
auto DISPATCHER(long int _consumers = 0, size_t _max_queue_size = 0)
    -> zpt::events::dispatcher::ptr;
```

Creates a dispatcher with the specified capacity.

**Parameters:**
- `_consumers` - Number of consumer threads
- `_max_queue_size` - Maximum queue size

### `zpt::make_event`

```cpp
template<zpt::events::Operation T>
auto make_event(T _operator) -> zpt::event;

template<zpt::events::Operation T, typename... Args>
auto make_event(Args&&... _args) -> zpt::event;
```

Creates an event from an Operation type.

### `zpt::event_cast`

```cpp
template<zpt::events::Operation T>
auto event_cast(zpt::event& _event) -> T&;
```

Casts an event to access its underlying Operation.

### `zpt::make_call`

```cpp
template<typename T, typename... Args>
auto make_call(zpt::events::resolver_t& _resolver, zpt::message _request, Args... _args);
```

Creates an outbound call event. `T` is the response handler type.

---

## Class: `zpt::events::resolver_t`

Abstract interface for mapping messages to event handlers.

### Methods

#### `add` (template, Operation class)

```cpp
template<zpt::events::Operation T>
auto add(zpt::json const& _id, zpt::json const& _metadata = zpt::undefined) -> resolver_t&;

template<zpt::events::Operation T>
auto add(zpt::performative _performative,
         zpt::json const& _id,
         zpt::json const& _metadata = zpt::undefined) -> resolver_t&;
```

Registers an Operation class as an event handler.

#### `add` (callback)

```cpp
virtual auto add(zpt::json const& _service_description) -> resolver_t& = 0;
virtual auto add(zpt::message _sent,
                 zpt::events::resolver_callback callback) -> resolver_t& = 0;
virtual auto add(zpt::performative _performtive,
                 zpt::json const& _id,
                 zpt::json const& _metadata,
                 zpt::events::resolver_callback _callback) -> resolver_t& = 0;
```

#### `remove`

```cpp
template<zpt::events::Operation T>
auto remove(zpt::json const& _id) -> resolver_t&;

template<zpt::events::Operation T>
auto remove(zpt::performative _performative, zpt::json const& _id) -> resolver_t&;
```

Unregisters an event handler.

#### `resolve`

```cpp
virtual auto resolve(zpt::message _received, initializer_t _initializer) const
    -> std::list<zpt::event> = 0;
```

Finds event handlers matching a message.

**Returns:** List of events to dispatch.

---

## System Events

### Enum: `zpt::system_event_type`

Built-in system lifecycle events.

| Value | Description |
|-------|-------------|
| `BOOTING` | System is starting up |
| `FINISHED_BOOT` | Boot sequence complete |
| `MINION_BOOT_RECEIVED` | Worker node boot notification |
| `MINION_HELLO_RECEIVED` | Worker node hello message |
| `REGISTERED_REMOTE_SERVICE` | Remote service registered |
| `MINION_SHUTDOWN_RECEIVED` | Worker node shutdown |
| `UNREGISTERED_REMOTE_SERVICE` | Remote service removed |
| `SHUTTING_DOWN` | System shutting down |
| `EXITING` | Final exit |

### Class: `zpt::system_event`

System event operation implementing the Operation concept.

```cpp
system_event();
system_event(zpt::message _received);
system_event(zpt::system_event_type _type, zpt::json const& _data = zpt::undefined);
```

### `zpt::SYSTEM_EVENTS_RESOLVER`

```cpp
auto SYSTEM_EVENTS_RESOLVER() -> zpt::system_events::resolver;
```

Returns the global system events resolver.

---

## Process Base Class

`zpt::events::process` (from `<zapata/transport/engine.h>`) provides a ready-made implementation of the Operation concept for REST handlers.

### Methods

| Method | Description |
|--------|-------------|
| `received() const` | Get the received message |
| `to_send()` | Get the response message |
| `context() const` | Get the call context |
| `context(ptr)` | Set the call context |

### Usage Pattern

```cpp
class my_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        // Access request
        auto body = this->received()->body();
        // Send response
        this->to_send()->status(200)->body() = body;
        return zpt::events::finish;
    }
};
```

---

## Usage Patterns

### Basic Event Processing

```cpp
#include <zapata/events.h>

struct ProcessRequest {
    zpt::message request;

    ProcessRequest(zpt::message req) : request(std::move(req)) {}

    void initialize(zpt::event_initialization&) {}
    bool blocked() const { return false; }
    bool authorized() const { return true; }
    bool catch_error(std::exception const&, zpt::events::dispatcher::ptr) { return false; }
    bool catch_error(std::bad_alloc const&, zpt::events::dispatcher::ptr) { return true; }
    bool catch_error(zpt::failed_expectation const&, zpt::events::dispatcher::ptr) { return false; }

    zpt::events::state operator()(zpt::events::dispatcher::ptr d) {
        if (request->performative() == zpt::Get) {
            handle_get();
        }
        return zpt::events::finish;
    }

private:
    void handle_get() { /* ... */ }
};

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    auto msg = zpt::make_message<zpt::json_message>();
    msg->performative(zpt::Get);
    zpt::DISPATCHER()->trigger<ProcessRequest>(msg);
}
```

### Event with Re-triggering

```cpp
struct RetryableOperation {
    int attempts = 0;
    static constexpr int max_attempts = 3;

    void initialize(zpt::event_initialization&) {}
    bool blocked() const { return false; }
    bool authorized() const { return true; }

    bool catch_error(std::exception const& e, zpt::events::dispatcher::ptr d) {
        return attempts < max_attempts;
    }
    bool catch_error(std::bad_alloc const&, zpt::events::dispatcher::ptr) { return true; }
    bool catch_error(zpt::failed_expectation const&, zpt::events::dispatcher::ptr) { return false; }

    zpt::events::state operator()(zpt::events::dispatcher::ptr d) {
        ++attempts;
        if (some_transient_condition()) {
            return zpt::events::retrigger;
        }
        do_work();
        return zpt::events::finish;
    }

private:
    bool some_transient_condition() { return /* ... */ false; }
    void do_work() { /* ... */ }
};
```

### Making Outbound Calls

```cpp
// Create request
auto request = zpt::make_message<zpt::json_message>();
request->performative(zpt::Post);
request->uri("/remote/endpoint");
request->body() = { "data", "value" };

// Call with response handler
zpt::make_call<response_handler>(resolver, request);
```

---

## See Also

- [Lock-Free API](lockfree.md) - Queue implementation
- [JSON API](json.md) - Message body handling
- [Transport API](transport.md) - Transport events
