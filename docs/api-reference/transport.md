#Transport API Reference

This document provides the API reference for Zapata's transport layer.

## Headers

```cpp
#include <zapata/net/transport/amqp.h>      // AMQP transport
#include <zapata/net/transport/http.h>      // HTTP transport
#include <zapata/net/transport/local.h>     // Unix socket / file transports
#include <zapata/net/transport/mqtt.h>      // MQTT transport
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
| `PUB_SUB` (4) | Transport follows pub/sub flow |
| `UPGRADED` (8) | Transport is upgraded from another transport |

---

## Class: `zpt::basic_transport`

Abstract base class for protocol transports. Derived classes implement protocol-specific logic by overriding the pure virtual methods.

### Methods

```cpp
virtual auto has_capability(std::uint64_t _capability) const -> bool = 0;
```
Checks if the transport has a specific capability.

```cpp
virtual auto make_request() const -> zpt::message = 0;
```
Creates a new request message for this transport.

```cpp
virtual auto make_reply(bool _with_allocator = true) const -> zpt::message = 0;
virtual auto make_reply(zpt::message _request) const -> zpt::message = 0;
```
Creates a reply message, optionally based on a request.

```cpp
virtual auto process_incoming_request(zpt::stream _stream) const -> zpt::message = 0;
virtual auto process_incoming_reply(zpt::stream _stream) const -> zpt::message = 0;
```
Parses incoming requests/replies from a stream.

```cpp
virtual auto copy(zpt::message const& _to_copy) const -> zpt::message = 0;
```
Creates a copy of the given message with the transport's protocol and format.

```cpp
virtual auto upgraded_from() const -> std::string const&;
```
Returns the name of the transport this one was upgraded from (empty string if not upgraded).

```cpp
auto receive(zpt::stream _stream) const -> zpt::message final;
auto send(zpt::stream _stream, zpt::message _to_send) const -> void final;
```
High-level methods for receiving/sending messages.

```cpp
virtual auto publish(zpt::message _to_publish) const -> void;
```
Publishes a message to a pub-sub topic (no-op by default).

---

## Type Alias: `zpt::transport`

```cpp
using transport = std::shared_ptr<basic_transport>;
```
Shared pointer to a transport.

---

## Class: `zpt::network::layer`

Transport registry with content negotiation.

### Type Aliases

```cpp
using translate_from_func = std::function<zpt::json(std::istream&)>;
using translate_to_func = std::function<std::string(std::ostream&, zpt::json)>;
```

### Constructor

```cpp
layer(zpt::json _global_config);
```

### Transport Management

```cpp
auto add(std::string const& _scheme, zpt::transport _transport) -> layer&;
auto get(std::string const& _scheme) const -> const zpt::transport;
auto remove(std::string const& _scheme) -> layer&;
auto clear() -> layer&;
```

### Resolution

```cpp
auto resolve(std::string _uri) const -> zpt::transport;
```
Returns the transport for a URI based on its scheme.

### Content Translation

```cpp
auto translate(std::istream& _io, std::string _mime = "*/*") const -> zpt::json;
auto translate(std::ostream& _io, std::string _mime, zpt::json _content) const -> std::string;
```
Translates content based on MIME type. Supported types:
- `application/json` - JSON serialization
- `application/xml` / `text/xml` - XML serialization
- `*/*` / `text/plain` - Raw text

### Iteration

```cpp
auto begin() const -> std::map<std::string, zpt::transport>::const_iterator;
auto end() const -> std::map<std::string, zpt::transport>::const_iterator;
```

### Free Function: `zpt::network::resolve_content_type`

```cpp
auto resolve_content_type(zpt::message _message) -> std::string;
```
Determines the content type from a message's headers (e.g., `"application/json"`).

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

Coordinates network I/O with event dispatch. Inherits from `std::enable_shared_from_this<engine>`.

#### Type Alias

```cpp
using ptr = std::shared_ptr<engine>;
```

### Constructor

```cpp
engine(zpt::json _config);
```

### Methods

```cpp
auto add_resolver(zpt::events::resolver _resolver) -> engine&;
auto remove_resolver(zpt::events::resolver _resolver) -> engine&;
auto resolve(zpt::message _received, zpt::events::initializer_t _initializer) const
    -> std::list<zpt::event>;
auto dispatcher() -> zpt::events::dispatcher::ptr;
auto shutdown() -> engine&;
```

### `zpt::TRANSPORT_ENGINE`

```cpp
auto TRANSPORT_ENGINE(zpt::json _config = zpt::undefined) -> zpt::transports::engine::ptr;
```
Returns the global transport engine instance.

---

## Transport Events

### Class: `zpt::events::transport_event_init`

Initialization data for transport events.

```cpp
class transport_event_init : public zpt::event_initialization {
  public:
    zpt::events::dispatcher::weak_ptr __dispatcher;
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

Abstract base class for message handlers. Subclass this to implement custom message processing.

#### Type Alias

```cpp
using ptr = std::shared_ptr<process>;
```

#### Constructors

```cpp
process(zpt::message _received);
process(zpt::message _received, zpt::call_context::ptr _context);
```

#### Accessors (all `final`)

```cpp
auto transport_type() const -> std::string const& final;
auto stream() const -> zpt::stream final;
auto received() const -> zpt::message const final;
auto to_send() -> zpt::message final;
auto context() const -> zpt::call_context::ptr final;
auto context(zpt::call_context::ptr _context) -> process& final;
```

#### Methods

```cpp
virtual auto initialize(zpt::event_initialization& init) -> void final;
virtual auto blocked() const -> bool;
virtual auto authorized() const -> bool;
virtual auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool final;
virtual auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool final;
virtual auto catch_error(zpt::failed_expectation const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool final;
virtual auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state = 0;
```

`blocked()` defaults to `true`; override to return `false` for non-blocking handlers. `authorized()` defaults to `true`. The only pure virtual is `operator()`.

### Class: `zpt::events::discard`

Default handler that discards messages without sending a response. Inherits from `process`.

### Class Template: `zpt::events::call<T>`

Event for making outbound calls with response handling.

```cpp
template<ProcessOperation T = zpt::events::discard>
class call { ... };
```

**Template parameter:** `T` - ProcessOperation type for handling responses (default: `discard`).

#### Constructor

```cpp
call(zpt::events::resolver _resolver, zpt::call_context::ptr _context, zpt::message _send);
```

### Class: `zpt::events::process_call_reply`

Default processor for call reply messages. Delivers the reply to the `call_context` so the caller can retrieve the response. Inherits from `process`.

### `zpt::make_call`

```cpp
template<ProcessOperation T = zpt::events::process_call_reply>
auto make_call(zpt::events::resolver _resolver, zpt::message _to_send) -> zpt::call_context::ptr;
```
Convenience function that creates a `call<T>` event and triggers it on the dispatcher. Returns the call context for tracking the response.

---

## Available Transports

### HTTP Transport

**Header:** `<zapata/net/transport/http.h>`
**Class:** `zpt::net::transport::http`
**URI schemes:** `http`, `https`
**Capabilities:** SYNCHRONOUS

Standard HTTP/1.1 request-response protocol.

**Server socket:** `zpt::HTTP_SERVER_SOCKET(port)`

### TCP Transport

**Header:** `<zapata/net/transport/tcp.h>`
**Class:** `zpt::net::transport::tcp`
**URI schemes:** `tcp`
**Capabilities:** SYNCHRONOUS

Raw TCP with JSON message framing.

**Server socket:** `zpt::TCP_SERVER_SOCKET(port)`

### WebSocket Transport

**Header:** `<zapata/net/transport/websocket.h>`
**Class:** `zpt::net::transport::websocket`
**URI schemes:** `ws`, `wss`
**Capabilities:** PERSISTENT, UPGRADED

WebSocket (RFC 6455) bidirectional messaging. Upgraded from HTTP.

#### WebSocket Utilities

```cpp
namespace zpt::net::ws {
    auto handshake(zpt::stream& _stream) -> void;
    auto read(std::istream& _stream) -> std::tuple<std::string, int>;
    auto write(std::ostream& _stream, std::string const& _in, bool _mask = false) -> void;
}
```

**Note:** The `handshake()` function has been moved to the HTTP layer for WebSocket upgrade handling. Use `websocket::process_incoming_request()` or `websocket::process_incoming_reply()` instead.

---

#### `ws_message` Class

```cpp
class ws_message : public json_message;
```

JSON message class specialized for WebSocket protocol. Wraps JSON content in WebSocket frame format for transmission.

**Inherits from:** `json_message`

**Methods:**

```cpp
virtual ~ws_message() = default;

auto to_stream(std::ostream& _out) const -> zpt::basic_message const& override;
auto from_stream(std::istream& _in) -> zpt::basic_message& override;
```

**`to_stream`**

Serializes the message to an output stream as a WebSocket text frame.

**Parameters:**
- `_out` - Output stream to write the framed message to

**Returns:** Reference to the output stream for chaining

**Frame format:** FIN=1, opcode=1 (text frame), no masking (client-to-server)

**`from_stream`**

Deserializes a WebSocket frame from an input stream.

**Parameters:**
- `_in` - Input stream containing a WebSocket frame

**Returns:** Reference to the deserialized message

**Raises:** `std::runtime_error` if frame is incomplete or malformed

**Example:**
```cpp
auto msg = std::make_shared<zpt::net::ws::ws_message>();
msg->body() = { "type", "chat", "text", "Hello, World!" };

std::ostringstream out;
msg->to_stream(out);
// Output: [FIN=1, opcode=1, len=17, payload="{"type":"chat","text":"Hello, World!"}"]
```

---

#### `read`

```cpp
namespace zpt::net::ws {
    auto read(std::istream& _stream) -> std::tuple<std::string, int>;
}
```

Reads a WebSocket frame from the input stream.

**Parameters:**
- `_stream` - Input stream to read the frame from (typically socket or file)

**Returns:** Tuple of `(payload_string, opcode_int)`

**Opcode values:**
- `1` - Text frame
- `2` - Binary frame

**Raises:** `std::runtime_error` if frame is incomplete or malformed

**Example:**
```cpp
std::istringstream in("[FIN=1, opcode=1, len=17, payload=\"Hello\"]");
auto [payload, opcode] = zpt::net::ws::read(in);
// payload = "Hello", opcode = 1
```

---

#### `write`

```cpp
namespace zpt::net::ws {
    auto write(std::ostream& _stream, std::string const& _in, bool _mask = false) -> void;
}
```

Writes data as a WebSocket text frame to the output stream.

**Parameters:**
- `_stream` - Output stream to write the frame to
- `_in` - String data to send (UTF-8 encoded text)
- `_mask` - If `true`, enables masking for outbound frames (rarely needed; RFC 6455 requires masking for client-to-server frames)

**Example:**
```cpp
std::ostringstream out;
zpt::net::ws::write(out, "Hello, World!");
// Output: [FIN=1, opcode=1, len=17, payload="Hello, World!"]
```

### Unix Socket Transport

**Header:** `<zapata/net/transport/local.h>`
**Class:** `zpt::net::transport::unix_socket`
**URI schemes:** `unix`
**Capabilities:** SYNCHRONOUS

Unix domain socket for local IPC.

**Server socket:** `zpt::UNIX_SERVER_SOCKET(path)`

### File Transport

**Header:** `<zapata/net/transport/local.h>`
**Class:** `zpt::net::transport::file`
**URI schemes:** `file`
**Capabilities:** SYNCHRONOUS

File-based message I/O.

### Named Pipe Transport

**Header:** `<zapata/net/transport/pipe.h>`
**Class:** `zpt::net::transport::pipe_stream`
**URI schemes:** `pipe`
**Capabilities:** SYNCHRONOUS

Inter-process communication via named pipes.

### Self Transport

**Header:** `<zapata/net/transport/self.h>`
**Class:** `zpt::net::transport::self`
**URI schemes:** `self`
**Capabilities:** SYNCHRONOUS

In-process message passing without serialization.

### UPnP Transport

**Header:** `<zapata/net/transport/upnp.h>`
**Class:** `zpt::net::transport::upnp`
**URI schemes:** `upnp`

UPnP/SSDP device discovery via multicast UDP.

### MQTT Transport

**Header:** `<zapata/net/transport/mqtt.h>`
**Class:** `zpt::net::transport::mqtt`
**URI schemes:** `mqtt`
**Capabilities:** PUB_SUB

MQTT socket communication with JSON message framing. Uses length-prefixed messages for reliable delivery.

### AMQP Transport

**Header:** `<zapata/net/transport/amqp.h>`
**Class:** `zpt::net::transport::amqp`
**URI schemes:** `amqp`
**Capabilities:** PUB_SUB

AMQP socket communication with JSON message framing. Uses length-prefixed messages for reliable delivery.

---

## Email Transport (SMTP)

Zapata provides a high-level SMTP client for sending email messages via the Simple Mail Transfer Protocol.

**Header:** `<zapata/smtp/SMTP.h>`

**Namespace:** `zpt::smtp`

### Class: `SMTPPtr`

Smart pointer wrapper for SMTP client instances with automatic reference counting.

```cpp
class SMTPPtr : public std::shared_ptr<zpt::SMTP> {
  public:
    SMTPPtr();                        // Wraps a new SMTP instance
    virtual ~SMTPPtr();               // Automatically cleans up
};
```

---

### Class: `SMTP`

SMTP client for sending email messages with authentication and SSL/TLS support.

**Constructor**

```cpp
SMTP();
```

Creates an SMTP client with default port 0 (configured at runtime).

**Destructor**

```cpp
virtual ~SMTP();
```

Closes the SMTP connection and frees all resources.

---

#### `credentials`

Sets authentication credentials for SMTP login.

**Parameters:**
- `_user` - Username for authentication
- `_passwd` - Password for authentication

**Example:**
```cpp
smtp->credentials("user@example.com", "password123");
```

---

#### `user`

Returns the configured authentication username.

**Returns:** Username string

---

#### `passwd`

Returns the configured authentication password.

**Returns:** Password string

---

#### `connect`

Connects to an SMTP server using the given connection URI.

The URI scheme determines the protocol and security settings:
- `smtp://host:port` - Plain SMTP
- `smtp+ssl://host:port` - SMTP over SSL/TLS
- `smtp+tls://host:port` - SMTP over STARTTLS
- `esmtp://host:port` - Extended SMTP (optional)
- `esmtp+ssl://host:port` - ESMTP with SSL
- `esmtp+tls://host:port` - ESMTP with STARTTLS

**Parameters:**
- `_connection` - SMTP connection URI

**Example:**
```cpp
// Plain SMTP
smtp->connect("smtp://smtp.example.com:587");

// SMTP with SSL
smtp->connect("smtp+ssl://smtp.example.com:465");

// SMTP with STARTTLS
smtp->connect("smtp+tls://smtp.example.com:587");
```

---

#### `send`

Sends an email message through the connected SMTP server.

**Parameters:**
- `_e_mail` - JSON object containing email fields

**Required fields:**
- `"From"` - Sender email address
- `"To"` - Comma-separated recipient list
- `"Subject"` - Email subject line
- `"Body"` - Email body content

**Optional fields:**
- `"Cc"` - Carbon copy recipients
- `"Bcc"` - Blind carbon copy recipients
- `"Reply-To"` - Reply address

**Example:**
```cpp
zpt::json email = {
    "From", "sender@example.com",
    "To", "recipient@example.com",
    "Subject", "Hello, World!",
    "Body", "This is a test email."
};

smtp->send(email);
```

**Note:** The `send()` method is synchronous and blocks until the email is delivered or the operation fails.

---

#### Private Methods

The following methods are implementation details and not intended for direct use:

```cpp
auto open() -> mailsmtp*;                    // Opens SMTP connection with SSL/TLS
auto close(mailsmtp* _smtp) -> void;          // Closes SMTP session
auto compose(zpt::json _e_mail) -> std::string;  // Composes MIME email string
```

---

### Usage Example

```cpp
#include <zapata/smtp/SMTP.h>

int main() {
    // Create SMTP client
    auto smtp = std::make_shared<zpt::SMTP>();

    // Set credentials
    smtp->credentials("user@example.com", "password");

    // Connect to server
    smtp->connect("smtp+tls://smtp.example.com:587");

    // Create email
    zpt::json email = {
        "From", "sender@example.com",
        "To", "recipient@example.com",
        "Subject", "Test Email",
        "Body", "This is a test message sent via Zapata's SMTP client."
    };

    // Send email
    smtp->send(email);

    // Cleanup (automatically happens on destruction)
    return 0;
}
```

---

### Notes

- The SMTP client uses libetpan for underlying protocol implementation
- SSL/TLS negotiation is handled automatically based on the URI scheme
- The client supports SMTP extensions (ESMTP) including authentication
- Email composition uses MIME with multipart support for attachments
- Connection pooling is not implemented; each `send()` creates a new connection

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
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto& req = *this->received();

        if (req.performative() == zpt::Get) {
            this->to_send()->status(200);
            this->to_send()->body() = { "result", "success" };
        } else {
            this->to_send()->status(405);
        }

        return zpt::events::finish;
    }
};
```

### Making Outbound Calls

```cpp
#include <zapata/transport/engine.h>
#include <zapata/rest/services.h>

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
zpt::make_call<MyResponseHandler>(zpt::REST_RESOLVER(), request);
```

---

## See Also

- [I/O API](io.md) - Stream infrastructure
- [Events API](events.md) - Event dispatcher
- [HTTP API](http.md) - HTTP message types
