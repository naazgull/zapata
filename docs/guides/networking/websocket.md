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

## Configuration

```json
{
    "transports": [
        {
            "type": "ws",
            "bind": "tcp://0.0.0.0:8081"
        }
    ]
}
```

## Handling WebSocket Messages

WebSocket messages are delivered through the same event system as HTTP:

```cpp
auto& boot = zpt::BOOT_ENGINE();

// Handle incoming WebSocket messages
boot.add_handler(zpt::Post, "/ws/chat",
    [](zpt::performative _method,
       zpt::json _envelope,
       zpt::json _opts) -> zpt::json {
        auto message = _envelope["body"]["message"];

        // Process and respond
        return {
            "status", 200,
            "body", {
                "type", "message",
                "content", message
            }
        };
    });
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
