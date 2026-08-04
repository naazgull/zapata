# URI API Reference

This document provides the API reference for the Zapata URI parsing module.

## Headers

```cpp
#include <zapata/uri.h>        // Main aggregate header
#include <zapata/uri/uri.h>    // Core functions only
```

---

## Namespace: `zpt::uri`

URI parsing and serialization functions.

### Functions

#### `parse`

Parses a URI string into a structured JSON object.

```cpp
auto parse(std::string const& _in, zpt::JSONType _type = zpt::JSObject) -> zpt::json;
auto parse(std::istream& _in, zpt::JSONType _type = zpt::JSObject) -> zpt::json;
```

**Parameters:**
- `_in` - URI string or input stream
- `_type` - Output format (`JSObject` for named fields, `JSArray` for path segments)

**Returns:** JSON object with URI components

**Example:**
```cpp
auto uri = zpt::uri::parse("https://user:pass@api.example.com:8080/v1/users?limit=10&offset=0#section");
```

**Result structure:**
```json
{
    "scheme": "https",
    "user": "user",
    "password": "pass",
    "host": "api.example.com",
    "port": 8080,
    "path": "/v1/users",
    "params": {
        "limit": "10",
        "offset": "0"
    },
    "anchor": "section"
}
```

---

#### `to_string`

Converts a parsed URI back to string form.

```cpp
auto to_string(zpt::json const& _uri) -> std::string;
```

**Parameters:**
- `_uri` - JSON object containing URI components

**Returns:** Reconstructed URI string

**Example:**
```cpp
zpt::json uri = {
    "scheme", "https",
    "host", "api.example.com",
    "port", 8080,
    "path", "/v1/users"
};
std::string url = zpt::uri::to_string(uri);
// "https://api.example.com:8080/v1/users"
```

---

#### `to_regex`

Converts a URI with path parameters to a regex pattern for route matching.

```cpp
auto to_regex(zpt::json const& _in) -> zpt::json;
```

**Parameters:**
- `_in` - URI JSON with path containing placeholders (e.g., `:id`, `{userId}`)

**Returns:** JSON with compiled regex pattern

**Example:**
```cpp
auto uri = zpt::uri::parse("/api/users/:id/posts/:postId");
auto pattern = zpt::uri::to_regex(uri);
// Pattern matches: /api/users/123/posts/456
```

---

#### `to_regex_object` / `to_regex_array`

Variant functions for specific output formats.

```cpp
auto to_regex_object(zpt::json const& _in) -> zpt::json;
auto to_regex_array(zpt::json const& _in) -> zpt::json;
```

---

## Namespace: `zpt::uri::path`

Path-specific serialization.

### Functions

#### `to_string`

Extracts and serializes only the path component.

```cpp
auto to_string(zpt::json const& _uri) -> std::string;
```

**Example:**
```cpp
zpt::json uri = { "path", "/v1/users", "params", { "q", "test" } };
std::string path = zpt::uri::path::to_string(uri);
// "/v1/users"
```

---

## Namespace: `zpt::uri::address`

Host/port serialization.

### Functions

#### `to_string`

Extracts and serializes the host:port portion.

```cpp
auto to_string(zpt::json const& _uri) -> std::string;
```

**Example:**
```cpp
zpt::json uri = { "host", "example.com", "port", 8080 };
std::string addr = zpt::uri::address::to_string(uri);
// "example.com:8080"
```

---

## Namespace: `zpt::uri::params`

Query parameter serialization.

### Functions

#### `to_string`

Serializes query parameters to URL-encoded string.

```cpp
auto to_string(zpt::json const& _uri) -> std::string;
```

**Example:**
```cpp
zpt::json uri = { "params", { "q", "hello world", "page", "1" } };
std::string qs = zpt::uri::params::to_string(uri);
// "q=hello%20world&page=1"
```

---

## URI Components

When parsing a URI, the following components may be extracted:

| Component | Type | Description |
|-----------|------|-------------|
| `scheme` | string | Protocol (http, https, ws, etc.) |
| `user` | string | Username from authority |
| `password` | string | Password from authority |
| `host` | string | Hostname or IP address |
| `port` | integer | Port number |
| `path` | string | Resource path |
| `params` | object | Query string parameters |
| `anchor` | string | Fragment identifier |

---

## Usage Patterns

### Parsing Request URIs

```cpp
// From HTTP request
std::string request_uri = "/api/users?page=2&limit=50";
auto uri = zpt::uri::parse(request_uri);

// Access components
std::string path = uri("path");
int page = uri("params")("page");
int limit = uri("params")("limit");
```

### Building URIs

```cpp
zpt::json uri = {
    "scheme", "https",
    "host", "api.example.com",
    "path", "/v1/resources",
    "params", {
        "filter", "active",
        "sort", "created_at"
    }
};

std::string url = zpt::uri::to_string(uri);
// "https://api.example.com/v1/resources?filter=active&sort=created_at"
```

### Route Matching

```cpp
// Define route pattern
auto pattern = zpt::uri::to_regex(zpt::uri::parse("/users/:userId/posts/:postId"));

// Match incoming request
auto request_uri = zpt::uri::parse("/users/42/posts/100");
if (pattern("regex") == request_uri("path")) {
    // Extract parameters from match groups
}
```

---

## See Also

- [JSON API Reference](json.md)
- [HTTP API Reference](http.md)
