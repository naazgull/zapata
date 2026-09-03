# Ontology API Reference

The ontology module provides protocol-agnostic message types and performatives (HTTP-like methods) that work across different transport protocols.

## Module Overview

**Header:** `<zapata/ontology.h>`

| Component | Description |
|-----------|-------------|
| `zpt::performative` | Request methods (Get, Post, Put, Delete, etc.) |
| `zpt::basic_message` | Abstract message interface |
| `zpt::json_message` | JSON-based message implementation |

---

## Performatives

**Header:** `<zapata/ontology/performative.h>`

Performatives are protocol-agnostic request methods that map to HTTP verbs and additional methods for pub/sub and discovery protocols.

### Types

```cpp
using performative = unsigned short;  // Method/verb type
using status = unsigned short;        // Status code type
```

### Standard HTTP Performatives

| Constant | Value | Description |
|----------|-------|-------------|
| `zpt::Get` | 0 | HTTP GET - retrieve resource |
| `zpt::Put` | 1 | HTTP PUT - replace resource |
| `zpt::Post` | 2 | HTTP POST - create resource |
| `zpt::Delete` | 3 | HTTP DELETE - remove resource |
| `zpt::Head` | 4 | HTTP HEAD - metadata only |
| `zpt::Options` | 5 | HTTP OPTIONS - capability query |
| `zpt::Patch` | 6 | HTTP PATCH - partial update |
| `zpt::Reply` | 7 | Response message |

### Extended Performatives

| Constant | Value | Description |
|----------|-------|-------------|
| `zpt::Msearch` | 8 | SSDP M-SEARCH - discovery |
| `zpt::Notify` | 9 | SSDP NOTIFY - announcement |
| `zpt::Trace` | 10 | HTTP TRACE - diagnostic |
| `zpt::Connect` | 11 | HTTP CONNECT - tunnel |
| `zpt::Subscribe` | 12 | Pub/sub subscription |
| `zpt::Inform` | 13 | Push notification |
| `zpt::Performative_end` | 14 | Sentinel value |

### Conversion Functions

```cpp
namespace zpt::ontology {
    // Convert performative to string (e.g., "GET", "POST")
    auto to_str(zpt::performative _performative) -> const char*;

    // Convert string to performative (case-insensitive)
    auto from_str(std::string _performative) -> zpt::performative;
}
```

---

## Messages

**Header:** `<zapata/ontology/message.h>`

### zpt::basic_message

Abstract base class for protocol-agnostic messages.

```cpp
class basic_message {
public:
    basic_message() = default;
    basic_message(basic_message const& _req, bool);
    virtual ~basic_message() = default;

    // Clone
    virtual auto clone() const -> std::shared_ptr<basic_message> = 0;

    // Accessors
    virtual auto performative() const -> zpt::performative = 0;
    virtual auto status() const -> zpt::status = 0;
    virtual auto uri() -> zpt::json& = 0;
    virtual auto uri() const -> zpt::json const = 0;
    virtual auto version() const -> std::string = 0;
    virtual auto scheme() const -> std::string = 0;
    virtual auto resource() const -> zpt::json const = 0;
    virtual auto parameters() const -> zpt::json const = 0;
    virtual auto headers() -> zpt::json& = 0;
    virtual auto headers() const -> zpt::json const = 0;
    virtual auto body() -> zpt::json& = 0;
    virtual auto body() const -> zpt::json const = 0;
    virtual auto keep_alive() const -> bool = 0;
    virtual auto content_type() const -> std::string = 0;

    // Mutators
    virtual auto performative(zpt::performative _performative) -> basic_message& = 0;
    virtual auto status(zpt::status _status) -> basic_message& = 0;
    virtual auto uri(std::string const& _uri) -> basic_message& = 0;
    virtual auto version(std::string const& _version) -> basic_message& = 0;
    virtual auto header(std::string const& _name, std::string const& _value) -> basic_message& = 0;

    // Serialization
    virtual auto to_stream(std::ostream& _out) const -> basic_message const& = 0;
    virtual auto from_stream(std::istream& _in) -> basic_message& = 0;
    virtual auto empty() const -> bool = 0;

    // Reply tracking (final)
    virtual auto acquire_reply() -> bool final;
    virtual auto set_processors(size_t _n_processors) -> basic_message& final;
    virtual auto finish_processor() -> size_t final;

    // Typed copy
    template<typename T>
    auto copy() const -> std::shared_ptr<basic_message>;

    // Stream operators
    friend auto operator<<(std::ostream& _out, basic_message const& _in) -> std::ostream&;
    friend auto operator>>(std::istream& _in, basic_message& _out) -> std::istream&;
};

using message = std::shared_ptr<basic_message>;
```

**Message Components:**
- **Performative**: The HTTP-like method (GET, POST, etc.)
- **URI**: Target resource with path and parameters
- **Headers**: Key-value metadata
- **Body**: Message payload (typically JSON)
- **Status**: Response status code

### zpt::json_message

JSON-based message implementation that stores all data internally as JSON.

```cpp
class json_message : public basic_message {
public:
    json_message();
    json_message(zpt::json const& _other);
    json_message(zpt::message _req, bool);
    virtual ~json_message() = default;

    // All basic_message methods implemented
    auto performative() const -> zpt::performative override;
    auto status() const -> zpt::status override;
    auto uri() -> zpt::json& override;
    auto uri() const -> zpt::json const override;
    // ... etc.

    template<typename T>
    auto operator<<(T _to_add) -> zpt::json_message&;
};
```

### Factory Functions

```cpp
// Create message using standard allocator
template<typename T, typename... Args>
auto make_message(Args... _args) -> zpt::message;

// Create message using memory pool allocator
template<typename T, typename... Args>
auto allocate_message(Args... _args) -> zpt::message;

// Cast message to specific type
template<typename T>
auto message_cast(zpt::message _rhs) -> T&;

// Convert URI JSON to string
auto uri_to_string(zpt::json const& _uri) -> std::string;
```

### Call State Constants

```cpp
constexpr int CALL_STATE_UNPROCESSED = 0;
constexpr int CALL_STATE_SENT = 1;
constexpr int CALL_STATE_SUCCESS_REPLY = 2;
constexpr int CALL_STATE_FAILURE_REPLY = 3;
```

### Class: `zpt::call_context`

Context for tracking an outbound call and its reply. Used with `zpt::events::call` to correlate requests with responses.

#### Type Alias

```cpp
using ptr = std::shared_ptr<call_context>;
```

#### Methods

```cpp
auto state() const -> int;                    // Current call state
auto reply() const -> zpt::message;           // Reply message (null if not yet replied)
auto reply(zpt::message _to_update) -> call_context&;  // Set reply and update state
auto is_replied() const -> bool;              // True if reply available
auto has_error() const -> bool;               // True if reply indicates failure
```

---

## Usage Examples

### Creating Messages

```cpp
#include <zapata/ontology.h>

// Create a JSON message
auto msg = zpt::make_message<zpt::json_message>();

// Configure as a GET request
msg->performative(zpt::Get);
msg->uri("/api/users/123");
msg->header("Accept", "application/json");
msg->header("Authorization", "Bearer token123");

// Create a response
auto reply = zpt::make_message<zpt::json_message>();
reply->performative(zpt::Reply);
reply->status(200);
reply->header("Content-Type", "application/json");
reply->body() = zpt::json{
    "id", 123,
    "name", "John Doe",
    "email", "john@example.com"
};
```

### Converting Performatives

```cpp
// String to performative
auto method = zpt::ontology::from_str("POST");  // Returns zpt::Post

// Performative to string
auto name = zpt::ontology::to_str(zpt::Delete);  // Returns "DELETE"
```

### Message Serialization

```cpp
auto msg = zpt::make_message<zpt::json_message>();
msg->performative(zpt::Get);
msg->uri("/api/data");

// Serialize to stream
std::ostringstream oss;
oss << *msg;

// Deserialize from stream
std::istringstream iss(oss.str());
auto parsed = zpt::make_message<zpt::json_message>();
iss >> *parsed;
```

---

## See Also

- [HTTP Parser](http.md) - HTTP-specific message types
- [Transport](transport.md) - Transport layer that uses messages
- [JSON](json.md) - JSON types used in message bodies
