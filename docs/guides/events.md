# Events

Zapata uses an event-driven architecture for decoupled communication between components.

## Overview

The event system provides pub/sub messaging where:
- **Publishers** emit events without knowing who handles them
- **Subscribers** register interest in event patterns
- **Dispatchers** route events to matching subscribers

## The Event Dispatcher

`zpt::events::dispatcher` is the core event routing class:

```cpp
#include <zapata/events.h>

// Create a dispatcher
auto dispatcher = zpt::DISPATCHER(4, 1000);
dispatcher->start_consumers();

// Trigger an event of a specific type
dispatcher->trigger<my_event_type>(arg1, arg2);
```

## The Operation Concept

Event handlers must satisfy the `zpt::events::Operation` concept. The simplest way to implement this is to extend `zpt::events::process`:

```cpp
class my_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        // Process the received message
        auto body = this->received()->body();
        this->to_send()->status(200)->body() = body;
        return zpt::events::finish;
    }
};
```

### Required Methods

For custom Operation types (not extending `process`):

| Method | Description |
|--------|-------------|
| `initialize(event_initialization&)` | Called when event is created |
| `blocked() -> bool` | Return true if event should wait |
| `authorized() -> bool` | Return true if event is authorized |
| `catch_error(exception, dispatcher) -> bool` | Handle errors |
| `operator()(dispatcher) -> state` | Execute the operation |

## Event Patterns

### Simple Events

```cpp
// Register handler
resolver->add<my_handler>("/api/events");
```

### Pattern Matching

Events can be routed by URI patterns:

```cpp
// Match specific paths
resolver->add<users_handler>("/api/users");
resolver->add<user_handler>("/api/users/{id}");
```

### Performative-Based Routing

```cpp
// Register by HTTP method
resolver->add<zpt::Get, list_handler>("/api/users");
resolver->add<zpt::Post, create_handler>("/api/users");
```

## Built-in Event Types

### Transport Events

The transport engine uses events for request lifecycle:

| Event Class | Purpose |
|-------------|---------|
| `events::transport_event_init` | Transport initialization |
| `events::receive` | Incoming message received |
| `events::send` | Outgoing message to send |
| `events::process` | Process a request |
| `events::discard` | Discard a message |
| `events::call<T>` | Invoke a remote endpoint |

### Lifecycle Events

```cpp
// Subscribe to system events
SYSTEM_EVENTS_RESOLVER()->add<my_boot_handler>(
    zpt::system_event_type::FINISHED_BOOT);
```

## Publishing Events

```cpp
// Trigger an event
dispatcher->trigger<my_event_type>(arg1, arg2);

// Or create and enqueue explicitly
auto event = zpt::make_event<my_event_type>(arg1, arg2);
dispatcher->trigger(event);
```

## Making Outbound Calls

To call a remote endpoint and wait for the response:

```cpp
// Create request
auto request = zpt::make_message<zpt::json_message>();
request->performative(zpt::Post);
request->uri("/remote/endpoint");
request->body() = { "data", "value" };

// Make async call with response handler
zpt::make_call<response_handler>(resolver, request);
```

## Custom Events

Define application-specific event handlers:

```cpp
class user_created_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto user = this->received()->body();
        // Send welcome email, update analytics, etc.
        return zpt::events::finish;
    }
};

// Register the handler
resolver->add<user_created_handler>("/events/user.created");
```

## Thread Safety

- Event subscription is thread-safe
- Event publication (trigger) is thread-safe
- Handler execution is serialized per-event
- Use `zpt::json::clone()` if handlers modify shared state

## See Also

- [Events API Reference](../api-reference/events.md) - Dispatcher and resolver API
- [Architecture Overview](../architecture/overview.md) - Event-driven design
- [Building REST APIs](rest-api.md) - Events in REST context
