# Transport Layer

Zapata's transport abstraction provides protocol-agnostic messaging across different network protocols.

## Concept

The transport layer decouples application logic from specific protocols. A single REST handler works identically whether the request arrives via HTTP, WebSocket, Unix socket, or any other transport.

```
Application Code
       │
       ▼
Transport Abstraction (basic_transport)
       │
  ┌────┼────┬─────┬──────┬──────┬──────┐
  ▼    ▼    ▼     ▼      ▼      ▼      ▼
 HTTP  WS  TCP  Local  Pipe   Self   UPnP
```

## Transport Interface

All transports implement `zpt::basic_transport` (a plain abstract base class):

```cpp
class basic_transport {
public:
    // Declare capabilities
    virtual auto has_capability(std::uint64_t _capability) const -> bool = 0;

    // Create a request message
    virtual auto make_request() const -> zpt::message = 0;

    // Create a reply message
    virtual auto make_reply(bool _with_allocator = true) const -> zpt::message = 0;
    virtual auto make_reply(zpt::message _request) const -> zpt::message = 0;

    // Parse incoming messages from a stream
    virtual auto process_incoming_request(zpt::stream _stream) const -> zpt::message = 0;
    virtual auto process_incoming_reply(zpt::stream _stream) const -> zpt::message = 0;

    // Copy a message
    virtual auto copy(zpt::message const& _to_copy) const -> zpt::message = 0;

    // Final methods (not override-able):
    //   receive(stream) — dispatches to process_incoming_request or _reply based on context
    //   send(stream, message) — serializes and writes to stream
};
```

## Transport Capabilities

Each transport declares its capabilities using bitmask flags:

| Capability | Constant | Description |
|-----------|----------|-------------|
| Synchronous | `zpt::SYNCHRONOUS` (1) | Supports request-response pattern |
| Persistent | `zpt::PERSISTENT` (2) | Maintains persistent connections |
| Pub/Sub | `zpt::PUB_SUB` (4) | Follows publish/subscribe flow |
| Upgraded | `zpt::UPGRADED` (8) | Transport upgraded from another transport |

## Available Transports

| Transport | Header | Capabilities | Description |
|-----------|--------|-------------|-------------|
| HTTP | `<zapata/http.h>` | SYNCHRONOUS | HTTP/1.1 with SSL/TLS |
| WebSocket | `<zapata/websocket.h>` | PERSISTENT, UPGRADED | RFC 6455 WebSockets |
| TCP | `<zapata/tcp.h>` | SYNCHRONOUS | Raw TCP sockets |
| Local (Unix) | `<zapata/local.h>` | SYNCHRONOUS | Unix domain sockets |
| Pipe | `<zapata/pipe.h>` | SYNCHRONOUS | Named pipes (FIFO) |
| Self | `<zapata/self.h>` | SYNCHRONOUS | In-process callbacks |
| UPnP | `<zapata/upnp.h>` | SYNCHRONOUS | UPnP/SSDP discovery |
| MQTT | `<zapata/mqtt.h>` | PUB_SUB | MQTT broker integration |
| AMQP | `<zapata/amqp.h>` | PUB_SUB | AMQP message queue |

## The Network Layer Registry

`zpt::network::layer` manages transport registration and lookup:

```cpp
// Get the global transport layer
auto& layer = zpt::TRANSPORT_LAYER(config);

// Transports register themselves by scheme
layer.add("http", zpt::make_transport<zpt::net::transport::http>())
     .add("https", zpt::make_transport<zpt::net::transport::http>())
     .add("ws", zpt::make_transport<zpt::net::transport::websocket>())
     .add("tcp", zpt::make_transport<zpt::net::transport::tcp>());

// Look up a transport by scheme
auto http_transport = layer.get("http");
```

## Configuration

Each transport has its own config section (`http`, `tcp`, `ws`, `local`, etc.) and the `transport.default` field selects the primary transport:

```json
{
    "identity": { "id": "uuid", "name": "my-app" },
    "log": { "level": 6, "format": 1 },
    "load": [
        { "name": "builtin:http" },
        { "name": "builtin:ws" },
        { "name": "builtin:rest" },
        { "name": "builtin:upnp" }
    ],
    "resources": { "limits": { "max_heap_allocation": 0 } },
    "dispatcher": { "limits": { "max_workers": 4 } },
    "http": { "bind": "0.0.0.0", "port": 8080 },
    "ws": { "bind": "0.0.0.0", "port": 8081 },
    "local": { "path": "/tmp/myapp.sock" },
    "upnp": { "bind": "239.192.1.2", "port": 7979 },
    "transport": { "default": "http", "limits": { "max_workers": 16 } }
}
```

## The Transport Engine

The transport engine coordinates I/O with event dispatch:

```cpp
auto engine = zpt::TRANSPORT_ENGINE(config);
engine->add_resolver(my_resolver);
```

## See Also

- [Transport API Reference](../../api-reference/transport.md) - Detailed API
- [HTTP Guide](http.md) - HTTP protocol details
- [WebSocket Guide](websocket.md) - WebSocket usage
- [Custom Transports](../../extending/custom-transport.md) - Writing your own
