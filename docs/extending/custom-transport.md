# Writing Custom Transports

Implement a custom transport protocol by subclassing `zpt::basic_transport`.

## Overview

Custom transports implement the `zpt::basic_transport` abstract base class, enabling your protocol to integrate seamlessly with Zapata's REST engine and event system.

## Step 1: Define the Transport Class

```cpp
#include <zapata/transport.h>

namespace myproto {

class transport : public zpt::basic_transport {
  public:
    // Declare supported capabilities
    auto has_capability(std::uint64_t _capability) const -> bool override {
        return (_capability & zpt::SYNCHRONOUS) == _capability;
    }

    // Create a new request message
    auto make_request() const -> zpt::message override {
        return zpt::make_message<zpt::json_message>();
    }

    // Create a new reply message
    auto make_reply(bool _with_allocator = true) const -> zpt::message override {
        return zpt::make_message<zpt::json_message>();
    }

    // Create a reply in response to a specific request
    auto make_reply(zpt::message _request) const -> zpt::message override {
        auto msg = zpt::make_message<zpt::json_message>();
        msg->conversation_id(_request->conversation_id());
        return msg;
    }

    // Parse an incoming request from a stream
    auto process_incoming_request(zpt::stream _stream) const -> zpt::message override {
        auto msg = make_request();
        // Read and parse your protocol's wire format from _stream
        // Populate msg with parsed data (performative, uri, headers, body)
        return msg;
    }

    // Parse an incoming reply from a stream
    auto process_incoming_reply(zpt::stream _stream) const -> zpt::message override {
        auto msg = make_reply(true);
        // Read and parse your protocol's wire format from _stream
        // Populate msg with parsed data
        return msg;
    }

    // Create a copy of the given message
    auto copy(zpt::message const& _to_copy) const -> zpt::message override {
        return _to_copy->clone();
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
| Synchronous | `zpt::SYNCHRONOUS` (1) | Supports request-response pattern |
| Persistent | `zpt::PERSISTENT` (2) | Maintains persistent connections |
| Pub/Sub | `zpt::PUB_SUB` (4) | Follows publish/subscribe flow |
| Upgraded | `zpt::UPGRADED` (8) | Transport upgraded from another transport |

Capabilities are bitmask flags. Return true from `has_capability()` if your transport supports the queried capability:

```cpp
auto has_capability(std::uint64_t _capability) const -> bool override {
    return (_capability & (zpt::SYNCHRONOUS | zpt::PERSISTENT)) == _capability;
}
```

## Message Parsing and Serialization

The transport's job is to convert between its wire format and `zpt::message`. This happens in `process_incoming_request()` and `process_incoming_reply()`:

```cpp
auto process_incoming_request(zpt::stream _stream) const -> zpt::message override {
    auto msg = make_request();
    // Read your protocol's wire format from _stream
    // Populate the message with parsed data:
    msg->performative(zpt::Get);
    msg->uri("/api/resource");
    msg->headers("X-Custom", "value");
    msg->body() = zpt::json{ "data", parsed_data };
    return msg;
}
```

Sending is handled by the `send()` method (which is `final` in the base class) — it calls your transport's protocol-specific serialization through the stream interface.

## See Also

- [Transport API Reference](../api-reference/transport.md) - Transport interface details
- [Transport Guide](../guides/networking/transports.md) - Transport concepts
- [Component Architecture](../architecture/components.md) - Module dependencies
