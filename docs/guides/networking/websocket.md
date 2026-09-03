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
    // Read a WebSocket frame from an input stream
    // Returns tuple of (payload_string, opcode_int)
    auto read(std::istream& _stream) -> std::tuple<std::string, int>;

    // Write data as a WebSocket text frame to an output stream
    auto write(std::ostream& _stream, std::string const& _in, bool _mask = false) -> void;
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

Configure WebSocket alongside HTTP. Add the `builtin:ws` plugin and set both `http` and `ws` config sections:

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
    "upnp": { "bind": "239.192.1.2", "port": 7979 },
    "transport": { "default": "http", "limits": { "max_workers": 16 } }
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
