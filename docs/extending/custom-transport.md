# Writing Custom Transports

Implement a custom transport protocol by subclassing the transport abstraction layer.

## Overview

Custom transports implement `zpt::transport::basic_transport<T>` using the CRTP pattern, enabling your protocol to integrate seamlessly with Zapata's REST engine and event system.

## Step 1: Define the Transport Class

```cpp
#include <zapata/transport.h>

namespace myproto {

class transport : public zpt::transport::basic_transport<myproto::transport> {
  public:
    transport() = default;
    ~transport() = default;

    // Declare supported capabilities
    auto has_capability(std::uint64_t _capability) const -> bool override {
        return _capability == zpt::transport::SYNCHRONOUS;
    }

    // Receive an incoming message
    auto receive(zpt::message _message) -> void override {
        // Parse your protocol's wire format into a zpt::message
        // Dispatch to the event system
    }

    // Send an outgoing message
    auto send(zpt::message _message) -> void override {
        // Serialize zpt::message into your protocol's wire format
        // Write to the network
    }

    // Process a message through the handler chain
    auto process(zpt::message _message) -> void override {
        // Route the message to the appropriate handler
    }
};

} // namespace myproto
```

## Step 2: Register the Transport

Transports register themselves with the transport layer:

```cpp
// In your plugin initialization
auto& layer = zpt::TRANSPORT_LAYER();
layer.add("myproto", zpt::make_transport<myproto::transport>());
```

## Step 3: Configure

Add your transport plugin to the `load` array, and add transport-specific config:

```json
{
    "identity": { "id": "uuid", "name": "my-app" },
    "log": { "level": 6, "format": 1 },
    "load": [
        { "name": "builtin:myproto" },
        { "name": "builtin:rest" },
        { "name": "builtin:upnp" }
    ],
    "resources": { "limits": { "max_heap_allocation": 0 } },
    "dispatcher": { "limits": { "max_workers": 4 } },
    "myproto": { "bind": "0.0.0.0", "port": 9000 },
    "upnp": { "bind": "239.192.1.2", "port": 7979 },
    "transport": { "default": "myproto", "limits": { "max_workers": 16 } }
}
```

Set `transport.default` to your protocol name to make it the primary transport.

## Transport Capabilities

| Capability | Constant | Description |
|-----------|----------|-------------|
| Synchronous | `zpt::transport::SYNCHRONOUS` (1) | Supports request-response pattern |
| Persistent | `zpt::transport::PERSISTENT` (2) | Maintains persistent connections |

Return true from `has_capability()` for each capability your transport supports.

## Message Conversion

Convert between your protocol format and `zpt::message`:

```cpp
auto receive(zpt::message _message) -> void override {
    // Your protocol data → zpt::message
    _message->performative(zpt::Get);
    _message->uri("/api/resource");
    _message->headers("X-Custom", "value");
    _message->body() = zpt::json{ "data", parsed_data };
}

auto send(zpt::message _message) -> void override {
    // zpt::message → your protocol format
    auto method = _message->performative();
    auto uri = _message->uri();
    auto body = _message->body();
    // Serialize and send...
}
```

## Message Factory

Override `make_request()` and `make_reply()` to create protocol-appropriate messages:

```cpp
auto make_request() const -> zpt::message override {
    return zpt::make_message<zpt::json_message>();
}

auto make_reply(bool _with_allocator = true) const -> zpt::message override {
    return zpt::make_message<zpt::json_message>();
}
```

## See Also

- [Transport API Reference](../api-reference/transport.md) - Transport interface details
- [Transport Guide](../guides/networking/transports.md) - Transport concepts
- [Component Architecture](../architecture/components.md) - Module dependencies
