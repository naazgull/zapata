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
| `CONNECTED` | Stream has successfully connected to a peer |
| `IDLE` | Stream is idle, not being processed |
| `WAITING` | Stream is waiting for I/O readiness |
| `PROCESSING` | Stream is being processed by a delegate |
| `ERRORING_OUT` | Stream encountered an error |
| `DISCONNECTED` | Stream has been disconnected |
| `GRACEFULLY_CLOSE` | Stream should be closed |

---

## Class: `zpt::basic_stream`

Abstract stream wrapper with file descriptor support for epoll integration. Inherits from `std::enable_shared_from_this<basic_stream>`.

### Type Aliases

```cpp
typedef std::ostream& (*ostream_manipulator)(std::ostream&);
```

### Constructors

```cpp
basic_stream(std::string const& _transport);
template<typename T, typename... Args>
basic_stream(std::in_place_type_t<T>, std::string const& _transport, Args... _args);
```
**Note:** Copy and move constructors are deleted. The template constructor wraps an iostream of type `T` with the given transport scheme (e.g., `"tcp"`, `"unix"`).

### Operators

```cpp
auto operator=(int _rhs) -> basic_stream&;         // Set file descriptor
auto operator*() -> std::iostream&;                 // Access underlying stream
virtual operator int();                             // Get file descriptor
```

### Read/Write Methods

```cpp
template<typename T>
auto read(T& _out) -> basic_stream&;

template<typename T>
auto write(T _in) -> basic_stream&;

virtual auto read_without_io(std::any& _out) -> basic_stream&;
virtual auto write_without_io(std::any const& _in) -> basic_stream&;

virtual auto has_next() const -> bool;

template<typename T>
auto operator>>(T& _out) -> basic_stream&;

template<typename T>
auto operator<<(T _in) -> basic_stream&;

auto operator<<(ostream_manipulator _in) -> basic_stream&;
```

For class types (except `std::string`), these methods call `_out->from_stream()` and `_in->to_stream()` respectively. `read_without_io` / `write_without_io` transfer data via internal storage without actual I/O. `has_next()` returns true if more messages are available.

### Control Methods

```cpp
virtual auto close() -> basic_stream&;
virtual auto upgrade(std::string const& _to_transport) -> basic_stream&;
virtual auto event_data() -> void*;
```

`event_data()` returns the stable per-stream block (holding this stream's UUID and set index) that is stored in `epoll_event::data.ptr`.

### Metadata Methods

```cpp
virtual auto transport() -> std::string&;
virtual auto uri() -> std::string&;
virtual auto state(stream_state _state) -> basic_stream&;
virtual auto state() -> stream_state;
virtual auto persistent() -> bool;
virtual auto metadata(std::any _metadata) -> basic_stream&;
virtual auto metadata() const -> std::any const&;
virtual auto uuid() const -> zpt::uuid const& final;
```

### Peer Configuration

```cpp
template<typename IOStream>
auto set_peer(std::string const& _address, unsigned int _port) -> basic_stream&;
```

---

## Type Alias: `zpt::stream`

```cpp
using stream = std::shared_ptr<zpt::basic_stream>;
```
Shared pointer to a stream, used throughout the framework.

---

## Class: `zpt::stream_set`

Fixed-capacity, thread-safe container of polled streams. Each stream occupies a slot addressed by an integer drawn from a free-index pool, and every slot carries a lock-free claim token: the stream's 128-bit UUID while it is released (present in the poll, ready to be taken), or zero while it is claimed (muted out of the poll) or the slot is free. Claiming and releasing are compare-and-swap operations on that token, so the per-slot arrays are accessed without a mutex; only the UUID and URI lookup maps are guarded by a shared mutex.

`zpt::polling` holds one `stream_set` (`__polled_streams`) of the streams it is currently monitoring.

### Constructor

```cpp
stream_set(size_t _capacity);
```
Creates a set with `_capacity` slots.

### Methods

```cpp
auto insert(zpt::stream _stream) -> bool;
auto erase(zpt::stream _stream) -> bool;
auto upgrade(zpt::stream _stream, std::string const& _transport) -> zpt::stream_set&;
auto acquire(zpt::stream _stream) -> bool;
auto acquire(size_t _index, __uint128_t _uuid) -> zpt::stream;
auto acquired(zpt::stream _stream) const -> bool;
auto release(zpt::stream _stream) -> bool;
auto get(zpt::uuid const& _stream_id) const -> zpt::stream;
auto get(std::string const& _uri) const -> zpt::stream;
auto close() -> zpt::stream_set&;
```

| Method | Description |
|--------|-------------|
| `insert` | Inserts a stream and marks it ready (released); returns `false` if the set is full |
| `erase` | Removes a claimed stream and frees its slot |
| `upgrade` | Changes the underlying transport of a stream (updates the URI index) |
| `acquire(stream)` | Claims (mutes) a stream by its identity; returns `false` if it cannot be claimed |
| `acquire(index, uuid)` | Claims a stream by slot index and UUID; returns the stream, or `nullptr` |
| `acquired` | Returns `true` if the stream is currently claimed (muted) |
| `release` | Releases a claimed stream, marking it ready (unmuted) |
| `get(uuid)` | Looks up a stream by UUID, or `nullptr` |
| `get(uri)` | Looks up a stream by URI, or `nullptr` |
| `close` | Closes all streams in the set and clears it |

**Claim model:** `acquire` swaps the token from the stream's UUID to zero; `release` performs the reverse (zero to UUID). `insert` calls `release`, so an inserted stream starts out ready. A free or claimed slot reads zero.

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
polling(size_t _max_streams);
```
Creates an epoll instance with capacity for up to `_max_streams` polled streams (backed by a `zpt::stream_set`).

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
auto mute(zpt::uuid const& _id) -> zpt::stream;
auto mute(std::string const& _uri) -> zpt::stream;
auto mute(size_t _idx, __uint128_t _uuid) -> zpt::stream;
auto unmute(zpt::stream _stream) -> zpt::polling&;
auto upgrade(zpt::stream _stream, std::string const& _transport) -> zpt::polling&;
```

| Method | Description |
|--------|-------------|
| `listen_on` | Adds a stream to be monitored for I/O |
| `mute(stream)` | Temporarily stops monitoring a stream |
| `mute(uuid)` | Mutes a stream by UUID, returns the muted stream |
| `mute(uri)` | Mutes a stream by URI, returns the muted stream |
| `mute(idx, uuid)` | Mutes a stream by slot index and UUID (used by the poll loop), returns the muted stream |
| `unmute` | Resumes monitoring a muted stream |
| `upgrade` | Changes a stream's transport in-place |

### Delegate Registration

```cpp
auto register_delegate(delegate_fn_type _callback) -> zpt::polling&;
auto unregister_delegate(delegate_fn_type _callback) -> zpt::polling&;
```
Registers/unregisters a callback invoked when streams are ready. The delegate receives the polling instance and the ready stream.

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
auto STREAM_POLLING(size_t _max_streams = 0) -> zpt::polling::ptr;
```
Returns the global stream polling instance.

**Parameters:**
- `_max_streams` — Maximum number of polled streams the set can hold (default 0).

### `zpt::make_stream`

```cpp
template<typename T, typename... Args>
static auto make_stream(std::string const& _transport, Args... _args) -> zpt::stream;
```
Creates a stream wrapping a specific iostream type.

**Template parameters:**
- `T` - The underlying iostream type (e.g., `socketstream`, `pipestream`)
- `Args` - Constructor argument types

**Parameters:**
- `_transport` - Transport scheme string (e.g., `"tcp"`, `"unix"`, `"pipe"`)
- `_args` - Arguments forwarded to T's constructor

**Example:**

```cpp
auto sock = zpt::make_stream<zpt::socketstream>("tcp", "localhost", 8080, zpt::NO_SSL, IPPROTO_TCP);
auto pipe = zpt::make_stream<zpt::pipestream>("pipe", "my-pipe");
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
    "tcp", "example.com", 80, zpt::NO_SSL, IPPROTO_TCP);

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
    "tcp", "example.com", 443, zpt::USE_SSL, IPPROTO_TCP);

*stream << "GET / HTTP/1.1\r\n"
        << "Host: example.com\r\n\r\n"
        << std::flush;
```

### TCP Server with Polling

```cpp
#include <zapata/streams.h>
#include <zapata/net/socket.h>

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
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
auto client = zpt::make_stream<zpt::socketstream>("unix", "/tmp/my.sock");
```

### Inter-Process Pipe

```cpp
#include <zapata/io/pipe.h>

auto pipe = zpt::make_stream<zpt::pipestream>("pipe", "my-channel");

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
