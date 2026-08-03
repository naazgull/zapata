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

All transports implement `zpt::transport::basic_transport<T>` using CRTP:

```cpp
template<typename T>
class basic_transport : public zpt::transport::base {
public:
    // Declare capabilities
    virtual auto has_capability(std::uint64_t _capability) const -> bool = 0;

    // Create a request message
    virtual auto make_request() const -> zpt::message = 0;

    // Create a reply message
    virtual auto make_reply(bool _with_allocator = true) const -> zpt::message = 0;
    virtual auto make_reply(zpt::message _request) const -> zpt::message = 0;

    // Receive/send from a stream
    virtual auto receive(zpt::stream _stream) const -> zpt::message = 0;
    virtual auto send(zpt::stream _stream, zpt::message _to_send) const -> void = 0;
};
```

## Transport Capabilities

Each transport declares its capabilities:

| Capability | Constant | Description |
|-----------|----------|-------------|
| Synchronous | `zpt::transport::SYNCHRONOUS` (1) | Supports request-response pattern |
| Persistent | `zpt::transport::PERSISTENT` (2) | Maintains persistent connections |

HTTP supports both. Self (in-process) supports only synchronous.

## Available Transports

| Transport | Header | Capabilities | Description |
|-----------|--------|-------------|-------------|
| HTTP | `<zapata/http.h>` | SYNCHRONOUS | HTTP/1.1 with SSL/TLS |
| WebSocket | `<zapata/websocket.h>` | SYNCHRONOUS, PERSISTENT | RFC 6455 WebSockets |
| TCP | `<zapata/tcp.h>` | SYNCHRONOUS, PERSISTENT | Raw TCP sockets |
| Local | `<zapata/local.h>` | SYNCHRONOUS, PERSISTENT | Unix domain sockets |
| Pipe | `<zapata/pipe.h>` | SYNCHRONOUS | Named pipes (FIFO) |
| Self | `<zapata/self.h>` | SYNCHRONOUS | In-process callbacks |
| UPnP | `<zapata/upnp.h>` | SYNCHRONOUS | UPnP/SSDP discovery |

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

Bind transports to network addresses:

```json
{
    "transport": {
        "type": "http",
        "bind": "tcp://0.0.0.0:8080"
    }
}
```

### Multiple Transports

```json
{
    "transports": [
        { "type": "http", "bind": "tcp://0.0.0.0:8080" },
        { "type": "ws", "bind": "tcp://0.0.0.0:8081" },
        { "type": "local", "bind": "unix:///tmp/myapp.sock" }
    ]
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
