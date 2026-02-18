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
class basic_transport {
public:
    // Receive an incoming message
    virtual auto receive(zpt::message _message) -> void = 0;

    // Send an outgoing message
    virtual auto send(zpt::message _message) -> void = 0;

    // Process a message through the handler chain
    virtual auto process(zpt::message _message) -> void = 0;
};
```

## Transport Capabilities

Each transport declares its capabilities:

| Capability | Description |
|-----------|-------------|
| `BIND` | Can listen for incoming connections |
| `CONNECT` | Can initiate outgoing connections |

HTTP supports both. Self (in-process) supports only `CONNECT`.

## Available Transports

| Transport | Header | Bind | Connect | Description |
|-----------|--------|:----:|:-------:|-------------|
| HTTP | `<zapata/http.h>` | Yes | Yes | HTTP/1.1 with SSL/TLS |
| WebSocket | `<zapata/websocket.h>` | Yes | Yes | RFC 6455 WebSockets |
| TCP | `<zapata/tcp.h>` | Yes | Yes | Raw TCP sockets |
| Local | `<zapata/local.h>` | Yes | Yes | Unix domain sockets |
| Pipe | `<zapata/pipe.h>` | Yes | Yes | Named pipes (FIFO) |
| Self | `<zapata/self.h>` | No | Yes | In-process callbacks |
| UPnP | `<zapata/upnp.h>` | Yes | Yes | UPnP/SSDP discovery |

## The Network Layer Registry

`zpt::network::layer` manages transport registration and lookup:

```cpp
// Transports register themselves
zpt::network::layer::add("http", http_transport);
zpt::network::layer::add("ws", ws_transport);

// Look up a transport by scheme
auto& transport = zpt::network::layer::get("http");
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

## See Also

- [Transport API Reference](../../api-reference/transport.md) - Detailed API
- [HTTP Guide](http.md) - HTTP protocol details
- [WebSocket Guide](websocket.md) - WebSocket usage
- [Custom Transports](../../extending/custom-transport.md) - Writing your own
