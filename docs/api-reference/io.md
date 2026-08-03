# I/O API Reference

This document provides the API reference for Zapata's I/O streaming infrastructure.

## Headers

```cpp
#include <zapata/streams.h>          // Core streams + epoll polling
#include <zapata/net/socket.h>       // TCP/UDP/Unix socket streams
#include <zapata/io/pipe.h>          // Named pipe streams
```

---

## Namespace: `zpt`

All I/O types are in the `zpt` namespace.

---

## Enum: `zpt::stream_state`

Stream processing states used with `zpt::polling`.

| Value | Description |
|-------|-------------|
| `IDLE` | Stream is idle, not being processed |
| `WAITING` | Stream is waiting for I/O readiness |
| `PROCESSING` | Stream is being processed by a delegate |
| `ERRORING_OUT` | Stream encountered an error |

---

## Class: `zpt::basic_stream`

Abstract stream wrapper with file descriptor support for epoll integration.

### Type Aliases

```cpp
using ostream_manipulator = std::ostream& (*)(std::ostream&);
```

### Constructors

```cpp
basic_stream();
basic_stream(std::ios& _rhs);
basic_stream(std::unique_ptr<std::iostream> _underlying);
```
**Note:** Copy and move constructors are deleted.

### Operators

```cpp
auto operator=(int _rhs) -> basic_stream&;         // Set file descriptor
auto operator*() -> std::iostream&;                 // Access underlying stream
operator int();                                     // Get file descriptor
```

### Read/Write Methods

```cpp
template<typename T>
auto read(T& _out) -> basic_stream&;

template<typename T>
auto write(T _in) -> basic_stream&;

template<typename T>
auto operator>>(T& _out) -> basic_stream&;

template<typename T>
auto operator<<(T _in) -> basic_stream&;

auto operator<<(ostream_manipulator _in) -> basic_stream&;
```

For class types (except `std::string`), these methods call `_out->from_stream()` and `_in->to_stream()` respectively.

### Control Methods

```cpp
virtual auto close() -> basic_stream&;
virtual auto shutdown() -> basic_stream&;
```

### Metadata Methods

```cpp
virtual auto transport(const std::string& _rhs) -> basic_stream&;
virtual auto transport() -> std::string&;
virtual auto uri(const std::string& _rhs) -> basic_stream&;
virtual auto uri() -> std::string&;
virtual auto state() -> stream_state&;
```

---

## Type Alias: `zpt::stream`

```cpp
using stream = std::shared_ptr<zpt::basic_stream>;
```
Shared pointer to a stream, used throughout the framework.

---

## Class: `zpt::polling`

Epoll-based I/O multiplexer for efficient stream handling.

### Type Aliases

```cpp
using ptr = std::shared_ptr<polling>;
using delegate_fn_type = std::function<bool(zpt::polling::ptr, zpt::stream)>;
```

### Constants

```cpp
constexpr static int MAX_EVENT_PER_POLL{ 100 };
```

### Constructor

```cpp
polling();
```
Creates an epoll instance.

### Lifecycle Methods

```cpp
auto close() -> zpt::polling&;
auto shutdown() -> zpt::polling&;
auto is_in_shutdown() const -> bool;
```

### Stream Management

```cpp
auto listen_on(zpt::stream _stream) -> zpt::polling&;
auto mute(zpt::stream _stream) -> zpt::polling&;
auto unmute(zpt::stream _stream) -> zpt::polling&;
```

| Method | Description |
|--------|-------------|
| `listen_on` | Adds a stream to be monitored for I/O |
| `mute` | Temporarily stops monitoring a stream |
| `unmute` | Resumes monitoring a muted stream |

### Delegate Registration

```cpp
auto register_delegate(delegate_fn_type _callback) -> zpt::polling&;
```
Registers a callback invoked when streams are ready. The delegate receives the polling instance and the ready stream.

**Delegate return value:**
- `true` - Keep monitoring the stream
- `false` - Remove stream from polling

### Event Loop

```cpp
auto poll() -> zpt::polling&;
```
Waits for I/O events and dispatches to registered delegates.

---

## Class: `zpt::event_stream`

Stream wrapper for `eventfd`-based signaling without I/O.

### Constructor

```cpp
event_stream();
```
Creates an `eventfd` for inter-thread signaling.

### Methods

```cpp
auto read_without_io(std::any& _out) -> event_stream& override;
auto write_without_io(std::any const& _in) -> event_stream& override;
```
Transfers data via internal storage without actual I/O operations.

---

## Factory Functions

### `zpt::STREAM_POLLING`

```cpp
auto STREAM_POLLING() -> zpt::polling::ptr;
```
Returns the global stream polling instance.

### `zpt::make_stream`

```cpp
template<typename T, typename... Args>
static auto make_stream(Args... _args) -> zpt::stream;
```
Creates a stream wrapping a specific iostream type.

**Template parameters:**
- `T` - The underlying iostream type (e.g., `socketstream`, `pipestream`)
- `Args` - Constructor argument types

**Example:**

```cpp
auto sock = zpt::make_stream<zpt::socketstream>("localhost", 8080, zpt::NO_SSL, IPPROTO_TCP);
auto pipe = zpt::make_stream<zpt::pipestream>("my-pipe");
```

### `zpt::stream_cast`

```cpp
template<typename T>
auto stream_cast(zpt::stream& _rhs) -> T&;
```
Casts a stream to access its underlying iostream type.

---

## Socket Streams

### Constants

```cpp
constexpr char const* ADDR_ANONYMOUS = "";
constexpr bool NO_SSL = false;
constexpr bool USE_SSL = true;
#define UNIXPROTO_RAW -2  // Unix domain socket protocol
```

### Helper Functions

```cpp
auto ssl_error_print(SSL* _ssl, int _ret) -> std::string;
auto ssl_error_print(unsigned long _error = 0) -> std::string;
auto is_multicast_address(std::string const& _ip) -> bool;
```

---

## Class Template: `zpt::basic_socketbuf<Char>`

Low-level socket buffer implementing `std::basic_streambuf`.

### Type Aliases

```cpp
using socketbuf = basic_socketbuf<char>;
using wsocketbuf = basic_socketbuf<wchar_t>;
```

### Socket Control

```cpp
auto get_socket() -> int;
auto set_socket(int _sock) -> void;
auto set_context(SSL_CTX* _ctx) -> void;
auto set_protocol(short _protocol) -> void;
```

### Address Access

```cpp
auto address() -> zpt::sockaddr_t&;  // Local address
auto peer() -> zpt::sockaddr_t&;     // Remote peer address
```

### Properties

| Method | Description |
|--------|-------------|
| `ssl() -> bool&` | SSL/TLS enabled |
| `host() -> std::string&` | Host name |
| `port() -> int&` | Port number |
| `protocol() -> short` | Protocol (TCP/UDP/Unix) |
| `timeout() -> unsigned long long&` | I/O timeout |

### Error State

```cpp
auto error_code() -> unsigned int&;
auto error_string() -> std::string&;
```

---

## Class Template: `zpt::basic_socketstream<Char>`

iostream-compatible socket stream supporting TCP, UDP, Unix sockets, and SSL/TLS.

### Type Aliases

```cpp
using socketstream = zpt::basic_socketstream<char>;
using wsocketstream = zpt::basic_socketstream<wchar_t>;
```

### Constructors

```cpp
// Default constructor
basic_socketstream();

// TCP/UDP with optional SSL
basic_socketstream(std::string const& _host, std::uint16_t _port, bool _ssl, short _protocol);
basic_socketstream(bool _ssl, short _protocol);

// From existing socket (TCP/UDP)
basic_socketstream(int s, zpt::sockaddrin_t& _address, bool _ssl, short _protocol);

// Unix domain socket
basic_socketstream(std::string const& _path);
basic_socketstream(int s, zpt::sockaddrun_t& _address);
```

### Operators

```cpp
operator int();        // Returns socket file descriptor
operator std::string(); // Returns URI (e.g., "tcp://host:port")
```

### Connection Methods

```cpp
auto open(std::string const& _host, std::uint16_t _port, bool _ssl = false,
          short _protocol = IPPROTO_TCP) -> bool;
auto open(std::string const& _path) -> bool;  // Unix socket
auto close() -> void;
auto is_open() -> bool;
auto ready() -> bool;
```

### Socket Configuration

```cpp
auto assign(int _sockfd) -> void;
auto assign(int _sockfd, SSL_CTX* _ctx) -> void;
auto unassign() -> void;
auto set_peer(std::string const& address, int port) -> void;
```

### Properties

```cpp
auto ssl() -> bool&;
auto host() -> std::string&;
auto port() -> int&;
auto protocol() -> short;
```

### Error Handling

```cpp
auto buffer() -> __buf_type&;
auto is_error() -> bool;
auto error_code() -> unsigned int&;
auto error_string() -> std::string&;
```

---

## Class Template: `zpt::basic_serversocketstream<Char>`

Server socket for accepting incoming connections.

### Constructors

```cpp
basic_serversocketstream();
basic_serversocketstream(std::uint16_t _port);    // TCP server
basic_serversocketstream(std::string const& _path); // Unix server
```

### Methods

```cpp
auto bind(std::uint16_t _port) -> bool;
auto bind(std::string const& _path) -> bool;
auto accept() -> zpt::stream;
auto close() -> void;
auto is_open() -> bool;
auto ready() -> bool;
```

### Wrapper Classes

```cpp
class serversocketstream;   // char wrapper with shared_ptr semantics
class wserversocketstream;  // wchar_t wrapper
```

---

## Pipe Streams

### Class Template: `zpt::basic_pipebuf<Char>`

Buffer for pipe-based I/O.

### Constructor

```cpp
basic_pipebuf(bool initialize = true);
```

### Methods

```cpp
auto open() -> void;
auto close() -> void;
auto output_fd() -> int;  // Write end
auto input_fd() -> int;   // Read end
```

---

### Class Template: `zpt::basic_pipestream<Char>`

iostream-compatible named pipe stream.

### Type Alias

```cpp
using pipestream = zpt::basic_pipestream<char>;
```

### Constructors

```cpp
basic_pipestream();
basic_pipestream(std::string const& _pipe_name);
```

### Operators

```cpp
operator int();         // Returns input file descriptor
operator std::string(); // Returns URI ("pipe:/name")
```

### Methods

```cpp
auto open(std::string const& _pipe_name) -> void;
auto close() -> void;
auto is_open() -> bool;
```

---

## Usage Patterns

### Basic Socket Client

```cpp
#include <zapata/net/socket.h>

// Connect to server
auto stream = zpt::make_stream<zpt::socketstream>(
    "example.com", 80, zpt::NO_SSL, IPPROTO_TCP);

// Send HTTP request
*stream << "GET / HTTP/1.1\r\n"
        << "Host: example.com\r\n\r\n"
        << std::flush;

// Read response
std::string line;
std::getline(**stream, line);
```

### SSL/TLS Client

```cpp
auto stream = zpt::make_stream<zpt::socketstream>(
    "example.com", 443, zpt::USE_SSL, IPPROTO_TCP);

*stream << "GET / HTTP/1.1\r\n"
        << "Host: example.com\r\n\r\n"
        << std::flush;
```

### TCP Server with Polling

```cpp
#include <zapata/streams.h>
#include <zapata/net/socket.h>

int main() {
    auto poll = zpt::STREAM_POLLING();

    // Set up delegate
    poll->register_delegate([](zpt::polling::ptr p, zpt::stream s) {
        std::string data;
        s >> data;
        std::cout << "Received: " << data << std::endl;
        return true;  // Keep stream
    });

    // Create server
    zpt::serversocketstream server(8080);

    // Accept loop
    while (!poll->is_in_shutdown()) {
        auto client = server->accept();
        poll->listen_on(client);
        poll->poll();
    }
}
```

### Unix Domain Socket

```cpp
// Server
zpt::serversocketstream server("/tmp/my.sock");

// Client
auto client = zpt::make_stream<zpt::socketstream>("/tmp/my.sock");
```

### Inter-Process Pipe

```cpp
#include <zapata/io/pipe.h>

auto pipe = zpt::make_stream<zpt::pipestream>("my-channel");

// Write to pipe (typically in parent process)
*pipe << "Hello from parent" << std::flush;

// Read from pipe (typically in child process after fork)
std::string msg;
*pipe >> msg;
```

### Event Stream for Signaling

```cpp
#include <zapata/streams.h>

auto poll = zpt::STREAM_POLLING();
auto signal = zpt::allocate_shared<zpt::event_stream>();

poll->register_delegate([](zpt::polling::ptr p, zpt::stream s) {
    std::any data;
    s->read_without_io(data);
    auto value = std::any_cast<int>(data);
    std::cout << "Signaled with: " << value << std::endl;
    return true;
});

poll->listen_on(signal);

// From another thread:
signal->write_without_io(std::make_any<int>(42));
```

---

## See Also

- [Events API](events.md) - Event dispatcher using streams
- [HTTP API](http.md) - HTTP message handling
