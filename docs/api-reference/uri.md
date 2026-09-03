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

## Class: `zpt::URIParser`

Wrapper around the re2c/bison generated parser for URIs. Provides programmatic access to URI parsing with configurable input/output streams.

**Header:** `<zapata/uri/URIParser.h>`

### Constructor

```cpp
URIParser(std::istream& _in = std::cin, std::ostream& _out = std::cout);
```

**Parameters:**
- `_in` - Input stream to read URI strings from (default: stdin)
- `_out` - Output stream for parser logging (default: stdout)

### Destructor

```cpp
virtual ~URIParser();
```

### Methods

#### `switchRoots`

Sets the JSON root node to populate during parsing.

The parser will write URI components into this JSON object.

**Parameters:**
- `_root` - JSON object to populate with URI components

**Example:**
```cpp
zpt::json root;
auto parser = zpt::URIParser(std::cin, std::cout);
parser->switchRoots(root);
parser->parse();
// root now contains: { "scheme", "http", "host", "example.com", ... }
```

---

#### `switchStreams`

Switches the input/output streams for parsing.

Allows reusing the parser instance with different streams without recreating it.

**Parameters:**
- `_in` - Input stream to read URI strings from
- `_out` - Output stream for parser logging

**Example:**
```cpp
std::ifstream file("uris.txt");
std::ofstream log("parser.log");

auto parser = zpt::URIParser();
parser->switchStreams(file, log);
parser->parse();  // Parses from file, logs to log file
```

---

#### `clear`

Clears the internal structures after parsing.

Resets parser state for reuse with a new URI string.

**Example:**
```cpp
zpt::json root;
auto parser = zpt::URIParser();

parser->switchRoots(root);
parser->parse();  // Parses first URI
parser->clear();  // Reset for next URI

parser->parse();  // Parses second URI
```

---

### Usage Pattern

```cpp
#include <zapata/uri/URIParser.h>
#include <fstream>

// Parse multiple URIs from a file
std::ifstream input("uris.txt");
std::ofstream output("parsed_uris.json");

zpt::json results = zpt::json::array();

auto parser = zpt::URIParser(input, output);
parser->switchRoots(results);

std::string uri;
while (std::getline(input, uri)) {
    // Clear for next URI
    parser->clear();
    // Parse (reads from the input stream)
    parser->parse();
    // Access parsed result
    if (!results->is_null()) {
        std::cout << "Scheme: " << results("scheme") << std::endl;
        std::cout << "Host: " << results("host") << std::endl;
    }
}
```

### Notes

- The URIParser uses the re2c/bison generated parser (`URITokenizer` base class)
- Each call to `parse()` consumes exactly one URI string
- For streaming URI input, consider using `zpt::uri::parse()` instead which reads from streams
- The parser is not thread-safe; create separate instances for each thread

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
auto to_string(zpt::json const& _uri, bool _not_first = false) -> std::string;
```

**Parameters:**
- `_uri` - JSON object containing URI with params
- `_not_first` - If `true`, omits the leading `?` (for appending to an existing query string)

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
