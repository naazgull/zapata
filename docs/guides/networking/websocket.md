# WebSocket Communication

Zapata supports RFC 6455 WebSocket connections for real-time bidirectional messaging.

## Overview

WebSocket provides:
- Full-duplex communication over a single TCP connection
- Low-latency message delivery
- Binary and text frame support
- Automatic ping/pong keepalive

## Setup

```cpp
#include <zapata/websocket.h>
```

Link against `zapata-net-websocket`:

```cmake
pkg_check_modules(ZAPATA REQUIRED zapata-net-websocket)
```

## How It Works

### Connection Upgrade

WebSocket connections start as HTTP and upgrade:

```
Client                    Server
  │  GET / HTTP/1.1          │
  │  Upgrade: websocket      │
  │  Connection: Upgrade     │
  │  Sec-WebSocket-Key: ...  │
  │─────────────────────────▶│
  │                          │
  │  HTTP/1.1 101 Switching  │
  │  Upgrade: websocket      │
  │  Sec-WebSocket-Accept: . │
  │◀─────────────────────────│
  │                          │
  │  ◀═══ WebSocket ═══▶     │
  │  (bidirectional frames)  │
```

### Message Framing

WebSocket messages are sent as frames:

| Frame Type | Description |
|-----------|-------------|
| Text (0x1) | UTF-8 text data |
| Binary (0x2) | Binary data |
| Close (0x8) | Connection close |
| Ping (0x9) | Keepalive request |
| Pong (0xA) | Keepalive response |

## WebSocket Utilities

```cpp
namespace zpt::net::ws {
    // Perform the WebSocket handshake
    auto handshake(zpt::stream& _stream) -> void;

    // Read a WebSocket frame
    auto read(zpt::stream& _stream) -> std::tuple<std::string, int>;

    // Write a WebSocket frame
    auto write(zpt::stream& _stream, std::string const& _in) -> void;
}
```

## Handling WebSocket Messages

WebSocket connections use the same event system as HTTP. Register handlers for WebSocket paths:

```cpp
#include <zapata/rest.h>

class ws_echo_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        this->to_send()->status(200)->body() = zpt::json{
            "echo", this->received()->body()
        };
        return zpt::events::finish;
    }
};

auto& resolver = zpt::REST_RESOLVER();
resolver->add<ws_echo_handler>("/ws/echo");
```

## Configuration

Configure WebSocket in your transport config:

```json
{
    "transport": {
        "type": "ws",
        "bind": "tcp://0.0.0.0:8081"
    }
}
```

Or combine with HTTP:

```json
{
    "transports": [
        { "type": "http", "bind": "tcp://0.0.0.0:8080" },
        { "type": "ws", "bind": "tcp://0.0.0.0:8081" }
    ]
}
```

## Use Cases

- **Real-time notifications** - Push updates to connected clients
- **Chat applications** - Bidirectional message exchange
- **Live data feeds** - Stream data changes to subscribers
- **Collaborative editing** - Synchronize document state

## See Also

- [Transport Guide](transports.md) - Transport abstraction layer
- [HTTP Guide](http.md) - HTTP protocol (WebSocket upgrade source)
- [Events Guide](../events.md) - Event-driven message handling
