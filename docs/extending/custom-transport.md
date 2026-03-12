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
    auto capabilities() const -> int override {
        return zpt::transport::BIND | zpt::transport::CONNECT;
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

```cpp
// In your plugin initialization
zpt::network::layer::add("myproto", zpt::allocate_shared<myproto::transport>());
```

## Step 3: Configure

```json
{
    "transport": {
        "type": "myproto",
        "bind": "myproto://0.0.0.0:9000"
    }
}
```

## Transport Capabilities

| Capability | Constant | Description |
|-----------|----------|-------------|
| Bind | `zpt::transport::BIND` | Can accept incoming connections |
| Connect | `zpt::transport::CONNECT` | Can initiate outgoing connections |

Return a bitwise OR of supported capabilities from `capabilities()`.

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

## See Also

- [Transport API Reference](../api-reference/transport.md) - Transport interface details
- [Transport Guide](../guides/networking/transports.md) - Transport concepts
- [Component Architecture](../architecture/components.md) - Module dependencies
