#Transport API Reference

This document provides the API reference for Zapata's transport layer.

## Headers

```cpp
#include <zapata/net/transport/http.h>      // HTTP transport
#include <zapata/net/transport/local.h>     // Unix socket / file transports
#include <zapata/net/transport/pipe.h>      // Named pipe transport
#include <zapata/net/transport/self.h>      // In-process transport
#include <zapata/net/transport/tcp.h>       // TCP transport
#include <zapata/net/transport/upnp.h>      // UPnP/SSDP transport
#include <zapata/net/transport/websocket.h> // WebSocket transport
#include <zapata/transport.h>               // Core transport abstractions
#include <zapata/transport/engine.h>        // Transport engine
```

---

## Namespace: `zpt`

Transport types are in `zpt` and `zpt::net::transport` namespaces.

---

## Enum: `zpt::transport_capability`

Capability flags for transport features.

| Value | Description |
|-------|-------------|
| `SYNCHRONOUS` (1) | Supports request-response pattern |
| `PERSISTENT` (2) | Maintains persistent connections |

---

## Class: `zpt::basic_transport`

Abstract base class for protocol transports.

### Methods

```cpp
virtual auto has_capability(std::uint64_t _capability) const -> bool = 0;
```

  Checks if the transport has a specific capability.

  -- -

```cpp virtual auto
  make_request() const -> zpt::message = 0;
```

Creates a new request message for this transport.

---

```cpp
virtual auto make_reply(bool _with_allocator = true) const -> zpt::message = 0;
virtual auto make_reply(zpt::message _request) const -> zpt::message = 0;
```

  Creates a reply message,
  optionally based on a request.

  -- -

```cpp virtual auto
  process_incoming_request(zpt::stream _stream) const -> zpt::message = 0;
virtual auto process_incoming_reply(zpt::stream _stream) const -> zpt::message = 0;
```

  Parses incoming requests /
  replies from a stream.

  -- -

```cpp auto
  receive(zpt::stream _stream) const -> zpt::message;
auto send(zpt::stream _stream, zpt::message _to_send) const -> void;
```

High-level methods for receiving/sending messages (final).

---

## Type Alias: `zpt::transport`

```cpp
using transport = std::shared_ptr<basic_transport>;
```

  Shared pointer to a transport
    .

  -- -

  ##Class
  : `zpt::network::layer`

  Transport registry with content negotiation.

  ## #Type Aliases

```cpp using translate_from_func = std::function<zpt::json(std::istream&)>;
using translate_to_func = std::function<std::string(std::ostream&, zpt::json)>;
```

  ## #Constructor

```cpp
  layer(zpt::json _global_config);
```

  ## #Transport Management

```cpp auto
  add(std::string const& _scheme, zpt::transport _transport) -> layer&;
auto get(std::string const& _scheme) const -> const zpt::transport;
auto remove(std::string const& _scheme) -> layer&;
auto clear() -> layer&;
```

  ## #Resolution

```cpp auto
  resolve(std::string _uri) const -> zpt::transport;
```

Returns the transport for a URI based on its scheme.

### Content Translation

```cpp
auto translate(std::istream& _io, std::string _mime = "*/*") const -> zpt::json;
auto translate(std::ostream& _io, std::string _mime, zpt::json _content) const -> std::string;
```

    Translates content based on MIME type.Supported types : - `application /
    json` -
  JSON serialization - `application / xml` / `text / xml` - XML serialization - `*/*` / `text/plain` - Raw text

### Iteration

```cpp
auto begin() const -> std::map<std::string, zpt::transport>::const_iterator;
auto end() const -> std::map<std::string, zpt::transport>::const_iterator;
```

---

## Factory Functions

### `zpt::TRANSPORT_LAYER`

```cpp
auto TRANSPORT_LAYER(zpt::json _config = nullptr) -> zpt::network::layer&;
```

Returns the global transport layer instance.

### `zpt::make_transport`

```cpp
template<typename T, typename... Args>
auto make_transport(Args... _args) -> zpt::transport;
```

Creates a transport using the memory pool allocator.

---

## Transport Engine

### Class: `zpt::transports::engine`

Coordinates network I/O with event dispatch.

### Constructor

```cpp
engine(zpt::json _config);
```

### Methods

```cpp
auto add_resolver(zpt::events::resolver _resolver) -> engine&;
auto resolve(zpt::message _received, zpt::events::initializer_t _initializer) const
    -> std::list<zpt::event>;
auto dispatcher() -> zpt::events::dispatcher::ptr;
auto shutdown() -> engine&;
```

### `zpt::TRANSPORT_ENGINE`

```cpp
auto TRANSPORT_ENGINE(zpt::json _config = nullptr) -> zpt::transports::engine::ptr;
```

Returns the global transport engine instance.

---

## Transport Events

### Class: `zpt::events::transport_event_init`

Initialization data for transport events.

```cpp
class transport_event_init : public zpt::event_initialization {
  public:
    zpt::events::dispatcher::ptr __dispatcher;
    zpt::polling::ptr __polling;
    zpt::stream __stream;
};
```

### Class: `zpt::events::receive`

Event for receiving messages from streams.

```cpp
receive(zpt::transports::engine::ptr _engine, zpt::polling::ptr _polling, zpt::stream _stream);
```

### Class: `zpt::events::send`

Event for sending messages to streams.

```cpp
send(zpt::polling::ptr _polling, zpt::stream _stream, zpt::message _to_send);
```

### Class: `zpt::events::process`

Abstract base class for message handlers.

```cpp
process(zpt::message _received);
auto received() const -> zpt::message const;
auto to_send() -> zpt::message;
virtual auto blocked() const -> bool = 0;
virtual auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state = 0;
```

### Class: `zpt::events::discard`

Default handler that discards messages.

### Class Template: `zpt::events::call<T>`

Event for making outbound calls with response handling.

```cpp
call(zpt::events::resolver _resolver, zpt::message _send);
```

**Template parameter:** `T` - ProcessOperation type for handling responses.

---

## Available Transports

### HTTP Transport

**Header:** `<zapata/net/transport/http.h>`

**Class:** `zpt::net::transport::http`

**URI schemes:** `http`, `https`

**Capabilities:** SYNCHRONOUS

**Server socket:** `zpt::HTTP_SERVER_SOCKET(port)`

Standard HTTP/1.1 request-response protocol.

---

### TCP Transport

**Header:** `<zapata/net/transport/tcp.h>`

**Class:** `zpt::net::transport::tcp`

**URI schemes:** `tcp`

**Capabilities:** SYNCHRONOUS | PERSISTENT

**Server socket:** `zpt::TCP_SERVER_SOCKET(port)`

Raw TCP with JSON message framing.

---

### WebSocket Transport

**Header:** `<zapata/net/transport/websocket.h>`

**Class:** `zpt::net::transport::websocket`

**URI schemes:** `ws`, `wss`

**Capabilities:** SYNCHRONOUS | PERSISTENT

**Server socket:** `zpt::WEBSOCKET_SERVER_SOCKET(port)`

WebSocket (RFC 6455) bidirectional messaging.

#### WebSocket Utilities

```cpp
namespace zpt::net::ws {
auto handshake(zpt::stream& _stream) -> void;
auto read(zpt::stream& _stream) -> std::tuple<std::string, int>;
auto write(zpt::stream& _stream, std::string const& _in) -> void;
}
```

---

### Unix Socket Transport

**Header:** `<zapata/net/transport/local.h>`

**Class:** `zpt::net::transport::unix_socket`

**URI schemes:** `unix`

**Capabilities:** SYNCHRONOUS | PERSISTENT

**Server socket:** `zpt::UNIX_SERVER_SOCKET(path)`

Unix domain socket for local IPC.

---

### File Transport

**Header:** `<zapata/net/transport/local.h>`

**Class:** `zpt::net::transport::file`

**URI schemes:** `file`

**Capabilities:** SYNCHRONOUS

File-based message I/O.

---

### Named Pipe Transport

**Header:** `<zapata/net/transport/pipe.h>`

**Class:** `zpt::net::transport::pipe_stream`

**URI schemes:** `pipe`

**Capabilities:** SYNCHRONOUS

Inter-process communication via named pipes.

---

### Self Transport

**Header:** `<zapata/net/transport/self.h>`

**Class:** `zpt::net::transport::self`

**URI schemes:** `self`

**Capabilities:** SYNCHRONOUS

In-process message passing without serialization.

---

### UPnP Transport

**Header:** `<zapata/net/transport/upnp.h>`

**Class:** `zpt::net::transport::upnp`

**URI schemes:** `upnp`

UPnP/SSDP device discovery via multicast UDP.

---

## Usage Patterns

### Registering Transports

```cpp
#include <zapata/net/transport/http.h>
#include <zapata/net/transport/websocket.h>
#include <zapata/transport.h>

auto config = zpt::json::object();
auto& layer = zpt::TRANSPORT_LAYER(config);

layer.add("http", zpt::make_transport<zpt::net::transport::http>())
     .add("https", zpt::make_transport<zpt::net::transport::http>())
     .add("ws", zpt::make_transport<zpt::net::transport::websocket>());
```

### Using the Transport Engine

```cpp
#include <zapata/transport/engine.h>

auto engine = zpt::TRANSPORT_ENGINE(config);

// Add a resolver for routing messages
engine->add_resolver(my_resolver);

// Start accepting connections
auto& server = zpt::HTTP_SERVER_SOCKET(8080);
auto polling = zpt::STREAM_POLLING();

polling->register_delegate([&](zpt::polling::ptr p, zpt::stream s) {
    engine->dispatcher()->trigger<zpt::events::receive>(engine, p, s);
    return true;
});

// Main loop
while (!polling->is_in_shutdown()) {
    auto client = server->accept();
    polling->listen_on(client);
    polling->poll();
}
```

### Custom Message Handler

```cpp
class MyHandler : public zpt::events::process {
public:
    MyHandler(zpt::message msg) : process(msg) {}

    bool blocked() const override { return false; }

    zpt::events::state operator()(zpt::events::dispatcher::ptr d) override {
        auto& req = received();

        if (req->performative() == zpt::Get) {
            to_send()->status(200);
            to_send()->body() = { "result", "success" };
        } else {
            to_send()->status(405);
        }

        return zpt::events::finish;
    }
};
```

### Making Outbound Calls

```cpp
#include <zapata/transport/engine.h>

// Create request
auto& layer = zpt::TRANSPORT_LAYER();
auto transport = layer.get("http");
auto request = transport->make_request();

request->performative(zpt::Get);
request->uri()["scheme"] = "http";
request->uri()["domain"] = "api.example.com";
request->uri()["port"] = 80;
request->uri()["path"] = "/v1/users";

// Trigger call with response handler
auto engine = zpt::TRANSPORT_ENGINE();
engine->dispatcher()->trigger<zpt::events::call<MyResponseHandler>>(resolver, request);
```

---

## See Also

- [I/O API](io.md) - Stream infrastructure
- [Events API](events.md) - Event dispatcher
- [HTTP API](http.md) - HTTP message types
