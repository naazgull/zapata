# HTTP API Reference

This document provides the API reference for the Zapata HTTP parsing module.

## Headers

```cpp
#include <zapata/http.h>           // Main aggregate header
#include <zapata/http/HTTPObj.h>   // Message types only
```

---

## Namespace: `zpt::http`

HTTP protocol types and utilities.

---

## Enum: `zpt::http::status`

HTTP status codes enumeration.

### Informational (1xx)

| Constant | Value | Description |
|----------|-------|-------------|
| `HTTP100` | 100 | Continue |
| `HTTP101` | 101 | Switching Protocols |
| `HTTP102` | 102 | Processing |

### Success (2xx)

| Constant | Value | Description |
|----------|-------|-------------|
| `HTTP200` | 200 | OK |
| `HTTP201` | 201 | Created |
| `HTTP202` | 202 | Accepted |
| `HTTP203` | 203 | Non-Authoritative Information |
| `HTTP204` | 204 | No Content |
| `HTTP205` | 205 | Reset Content |
| `HTTP206` | 206 | Partial Content |
| `HTTP207` | 207 | Multi-Status |
| `HTTP208` | 208 | Already Reported |
| `HTTP226` | 226 | IM Used |

### Redirection (3xx)

| Constant | Value | Description |
|----------|-------|-------------|
| `HTTP300` | 300 | Multiple Choices |
| `HTTP301` | 301 | Moved Permanently |
| `HTTP302` | 302 | Found |
| `HTTP303` | 303 | See Other |
| `HTTP304` | 304 | Not Modified |
| `HTTP305` | 305 | Use Proxy |
| `HTTP306` | 306 | (Unused) |
| `HTTP307` | 307 | Temporary Redirect |
| `HTTP308` | 308 | Permanent Redirect |

### Client Errors (4xx)

| Constant | Value | Description |
|----------|-------|-------------|
| `HTTP400` | 400 | Bad Request |
| `HTTP401` | 401 | Unauthorized |
| `HTTP402` | 402 | Payment Required |
| `HTTP403` | 403 | Forbidden |
| `HTTP404` | 404 | Not Found |
| `HTTP405` | 405 | Method Not Allowed |
| `HTTP406` | 406 | Not Acceptable |
| `HTTP407` | 407 | Proxy Authentication Required |
| `HTTP408` | 408 | Request Timeout |
| `HTTP409` | 409 | Conflict |
| `HTTP410` | 410 | Gone |
| `HTTP411` | 411 | Length Required |
| `HTTP412` | 412 | Precondition Failed |
| `HTTP413` | 413 | Payload Too Large |
| `HTTP414` | 414 | URI Too Long |
| `HTTP415` | 415 | Unsupported Media Type |
| `HTTP416` | 416 | Range Not Satisfiable |
| `HTTP417` | 417 | Expectation Failed |
| `HTTP422` | 422 | Unprocessable Entity |
| `HTTP423` | 423 | Locked |
| `HTTP424` | 424 | Failed Dependency |
| `HTTP425` | 425 | (Unassigned) |
| `HTTP426` | 426 | Upgrade Required |
| `HTTP427` | 427 | (Unassigned) |
| `HTTP428` | 428 | Precondition Required |
| `HTTP429` | 429 | Too Many Requests |
| `HTTP430` | 430 | (Unassigned) |
| `HTTP431` | 431 | Request Header Fields Too Large |
| `HTTP451` | 451 | Unavailable For Legal Reasons |

### Server Errors (5xx)

| Constant | Value | Description |
|----------|-------|-------------|
| `HTTP500` | 500 | Internal Server Error |
| `HTTP501` | 501 | Not Implemented |
| `HTTP502` | 502 | Bad Gateway |
| `HTTP503` | 503 | Service Unavailable |
| `HTTP504` | 504 | Gateway Timeout |
| `HTTP505` | 505 | HTTP Version Not Supported |
| `HTTP506` | 506 | Variant Also Negotiates |
| `HTTP507` | 507 | Insufficient Storage |
| `HTTP508` | 508 | Loop Detected |
| `HTTP509` | 509 | (Unassigned) |
| `HTTP510` | 510 | Not Extended |
| `HTTP511` | 511 | Network Authentication Required |

---

## Global Constants

### `method_names`

```cpp
inline const char* method_names[] = {
    "GET", "PUT", "POST", "DELETE", "HEAD", "OPTIONS",
    "PATCH", "REPLY", "M-SEARCH", "NOTIFY", "TRACE", "CONNECT"
};
```

### `status_names`

Array mapping status codes to their text descriptions.

---

## Class: `zpt::http::basic_message`

Base class for HTTP messages.

### Accessor Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `performative()` | `zpt::performative` | HTTP method |
| `status()` | `zpt::status` | Status code |
| `uri()` | `zpt::json&` | Request URI |
| `version()` | `std::string` | HTTP version |
| `scheme()` | `std::string` | URI scheme |
| `resource()` | `zpt::json` | Resource path |
| `parameters()` | `zpt::json` | Query parameters |
| `headers()` | `zpt::json&` | HTTP headers |
| `body()` | `zpt::json&` | Message body |
| `keep_alive()` | `bool` | Connection keep-alive |
| `content_type()` | `std::string` | Content-Type header |
| `anchor()` | `std::string` | URI fragment |
| `empty()` | `bool` | Check if uninitialized |

### Mutator Methods

| Method | Description |
|--------|-------------|
| `performative(perf)` | Set HTTP method |
| `status(status)` | Set status code |
| `uri(string)` | Set request URI |
| `version(string)` | Set HTTP version |
| `body(string)` | Set body content |
| `header(name, value)` | Set/add header |

---

## Class: `zpt::http::basic_request`

HTTP request message.

### Constructors

```cpp
basic_request();
basic_request(zpt::message _request, bool);
```

### Methods

| Method | Description |
|--------|-------------|
| `clone() const` | Returns a cloned copy of this request |
| `to_stream(ostream&)` | Serialize to stream |
| `from_stream(istream&)` | Parse from stream |

### Example

```cpp
zpt::http::basic_request req;
req.performative(zpt::Get);
req.uri("/api/users");
req.version("1.1");
req.header("Host", "api.example.com");
req.header("Accept", "application/json");
req.to_stream(socket);
```

---

## Class: `zpt::http::basic_reply`

HTTP response message.

### Constructors

```cpp
basic_reply();
basic_reply(zpt::message _request, bool);
```

### Methods

| Method | Description |
|--------|-------------|
| `clone() const` | Returns a cloned copy of this reply |
| `to_stream(ostream&)` | Serialize to stream |
| `from_stream(istream&)` | Parse from stream |

### Example

```cpp
zpt::http::basic_reply reply;
reply.status(zpt::http::HTTP200);
reply.version("1.1");
reply.header("Content-Type", "application/json");
reply.header("Content-Length", std::to_string(body.size()));
reply.body(R"({"status": "ok"})");
reply.to_stream(socket);
```

---

## Type Aliases

```cpp
using request = std::shared_ptr<basic_request>;
using reply = std::shared_ptr<basic_reply>;
```

---

## Free Functions

### Initialization

```cpp
void zpt::init(zpt::http::basic_request& _out);
void zpt::init(zpt::http::basic_reply& _out);
```

Initialize HTTP messages with default values.

### `zpt::http::retrieve`

**Header:** `<zapata/http/retrieve.h>`

```cpp
auto retrieve(zpt::message _to_send) -> zpt::message;
```
Sends an HTTP request message and returns the reply. Opens a TCP socket, writes the request, and reads the response. Supports both HTTP and HTTPS based on the message URI scheme.

### `zpt::http::resolve`

**Header:** `<zapata/http/retrieve.h>`

```cpp
auto resolve(std::string const& _domain) -> zpt::json;
```
Performs a DNS lookup and returns IPv4 and IPv6 addresses found. Returns a JSON object with `"ipv4"` and `"ipv6"` arrays.

---

## User-Defined Literals

### `_HTTP_REQUEST`

Parse an HTTP request from a string literal.

```cpp
auto operator"" _HTTP_REQUEST(const char* _string, size_t _length) -> zpt::message;
```

**Example:**
```cpp
auto req = R"(GET /api/users HTTP/1.1
Host: api.example.com
Accept: application/json

)"_HTTP_REQUEST;

auto method = req->performative();  // zpt::Get
auto host = req->headers()["Host"]; // "api.example.com"
```

### `_HTTP_REPLY`

Parse an HTTP response from a string literal.

```cpp
auto operator"" _HTTP_REPLY(const char* _string, size_t _length) -> zpt::message;
```

**Example:**
```cpp
auto reply = R"(HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: 15

{"status":"ok"})"_HTTP_REPLY;

auto status = reply->status();  // 200
auto body = reply->body();      // {"status":"ok"}
```

---

## Usage Patterns

### Handling Requests

```cpp
void handle_request(std::istream& in, std::ostream& out) {
    // Parse incoming request
    zpt::http::basic_request req;
    req.from_stream(in);

    // Process based on method and path
    if (req.performative() == zpt::Get && req.resource() == "/api/health") {
        zpt::http::basic_reply reply;
        reply.status(zpt::http::HTTP200);
        reply.header("Content-Type", "application/json");
        reply.body(R"({"healthy": true})");
        reply.to_stream(out);
    }
}
```

### Building Requests

```cpp
zpt::http::basic_request build_api_request(
    std::string const& method,
    std::string const& path,
    zpt::json body
) {
    zpt::http::basic_request req;

    if (method == "GET") req.performative(zpt::Get);
    else if (method == "POST") req.performative(zpt::Post);
    else if (method == "PUT") req.performative(zpt::Put);
    else if (method == "DELETE") req.performative(zpt::Delete);

    req.uri(path);
    req.version("1.1");
    req.header("Content-Type", "application/json");

    if (body->ok()) {
        std::string body_str = body.stringify();
        req.header("Content-Length", std::to_string(body_str.size()));
        req.body(body_str);
    }

    return req;
}
```

### Error Responses

```cpp
zpt::http::basic_reply error_response(zpt::http::status code, std::string const& message) {
    zpt::http::basic_reply reply;
    reply.status(code);
    reply.header("Content-Type", "application/json");

    zpt::json body = {
        "error", true,
        "message", message,
        "code", static_cast<int>(code)
    };

    std::string body_str = body.stringify();
    reply.header("Content-Length", std::to_string(body_str.size()));
    reply.body(body_str);

    return reply;
}

// Usage
auto not_found = error_response(zpt::http::HTTP404, "Resource not found");
auto bad_request = error_response(zpt::http::HTTP400, "Invalid request body");
```

---

## See Also

- [JSON API Reference](json.md)
- [URI API Reference](uri.md)
- [Transport Layer Guide](../guides/networking/transports.md)
