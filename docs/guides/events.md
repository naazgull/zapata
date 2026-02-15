# Events

Zapata uses an event-driven architecture for decoupled communication between components.

## Overview

The event system provides pub/sub messaging where:
- **Publishers** emit events without knowing who handles them
- **Subscribers** register interest in event patterns
- **Dispatchers** route events to matching subscribers

## The Event Dispatcher

`zpt::events::dispatcher<O>` is the core event routing class:

```cpp
#include <zapata/events.h>

// Create a dispatcher
auto dispatcher = zpt::events::make_dispatcher<zpt::json>();

// Subscribe to events
dispatcher->on("user.created",
    [](zpt::json _event) {
        auto user = _event["body"];
        zlog("New user: " + std::string(user["name"]), zpt::info);
    });

// Publish an event
dispatcher->trigger("user.created", {
    "body", { "id", 42, "name", "Alice" }
});
```

## The Operation Concept

Event handlers must satisfy the `zpt::events::Operation` concept:

```cpp
template<typename T>
concept Operation = requires(T _t) {
    // Must be callable with appropriate signature
};
```

This enables compile-time checking that handlers are compatible with the dispatcher.

## Event Patterns

### Simple Events

```cpp
dispatcher->on("order.placed", handle_order);
dispatcher->on("order.shipped", handle_shipping);
dispatcher->on("order.delivered", handle_delivery);
```

### Pattern Matching

Events can be routed by URI patterns:

```cpp
// Match specific paths
dispatcher->on("/api/users", handle_users);
dispatcher->on("/api/users/{id}", handle_user);

// Performative-based routing
dispatcher->on(zpt::Get, "/api/users", handle_list);
dispatcher->on(zpt::Post, "/api/users", handle_create);
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
| `events::call` | Invoke a remote endpoint |

### Lifecycle Events

```cpp
// Plugin lifecycle
dispatcher->on("plugin.loaded", on_plugin_load);
dispatcher->on("plugin.started", on_plugin_start);
dispatcher->on("plugin.stopped", on_plugin_stop);
```

## Publishing Events

```cpp
// Fire and forget
dispatcher->trigger("audit.log", {
    "action", "user.login",
    "user_id", 42,
    "timestamp", zpt::json::date()
});

// With performative
dispatcher->trigger(zpt::Post, "/events/notification", {
    "body", { "message", "Something happened" }
});
```

## Custom Events

Define application-specific events:

```cpp
// Define event names as constants
namespace events {
    constexpr auto USER_CREATED = "app.user.created";
    constexpr auto ORDER_PLACED = "app.order.placed";
    constexpr auto CACHE_EXPIRED = "app.cache.expired";
}

// Subscribe
dispatcher->on(events::USER_CREATED,
    [](zpt::json _event) {
        // Send welcome email, update analytics, etc.
    });

// Publish from handler
boot.add_handler(zpt::Post, "/api/users",
    [&dispatcher](zpt::performative _method,
                  zpt::json _envelope,
                  zpt::json _opts) -> zpt::json {
        auto user = create_user(_envelope["body"]);

        dispatcher->trigger(events::USER_CREATED, {
            "body", user
        });

        return { "status", 201, "body", user };
    });
```

## Thread Safety

- Event subscription is thread-safe
- Event publication is thread-safe
- Handler execution is serialized per-event
- Use `zpt::json::clone()` if handlers modify shared state

## See Also

- [Events API Reference](../api-reference/events.md) - Dispatcher and resolver API
- [Architecture Overview](../architecture/overview.md) - Event-driven design
- [Building REST APIs](rest-api.md) - Events in REST context
