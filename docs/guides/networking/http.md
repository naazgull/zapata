# HTTP Protocol

Zapata provides full HTTP/1.1 support with request/response parsing and SSL/TLS.

## Overview

The HTTP module includes:
- Request and response parsing (`basic_request`, `basic_reply`)
- HTTP method mapping to performatives
- Header management
- SSL/TLS via OpenSSL
- Keep-alive connection handling

## Headers

```cpp
#include <zapata/http.h>      // HTTP parser
#include <zapata/net/http.h>  // HTTP transport
```

## HTTP Methods and Performatives

| HTTP Method | Performative | Constant |
|------------|-------------|----------|
| GET | `zpt::Get` | 0 |
| PUT | `zpt::Put` | 1 |
| POST | `zpt::Post` | 2 |
| DELETE | `zpt::Delete` | 3 |
| HEAD | `zpt::Head` | 4 |
| OPTIONS | `zpt::Options` | 5 |
| PATCH | `zpt::Patch` | 6 |

Convert between methods and strings:

```cpp
auto method = zpt::ontology::from_str("POST");  // Returns zpt::Post
auto name = zpt::ontology::to_str(zpt::Get);    // Returns "GET"
```

## Request Parsing

```cpp
// Parse an HTTP request from a stream
auto request = zpt::make_message<zpt::http::basic_request>();
input_stream >> *request;

auto method = request->performative();    // zpt::Get, zpt::Post, etc.
auto uri = request->uri();                // Request URI
auto headers = request->headers();        // Header map
auto body = request->body();              // Body content
auto keep_alive = request->keep_alive();  // Connection: keep-alive?
```

## Response Formatting

```cpp
auto reply = zpt::make_message<zpt::http::basic_reply>();
reply->status(200);
reply->version("1.1");
reply->headers("Content-Type", "application/json");
reply->body() = zpt::json{ "message", "OK" };

output_stream << *reply;
```

## Status Codes

Use standard HTTP status codes:

```cpp
reply->status(200);  // OK
reply->status(201);  // Created
reply->status(204);  // No Content
reply->status(400);  // Bad Request
reply->status(401);  // Unauthorized
reply->status(404);  // Not Found
reply->status(500);  // Internal Server Error
```

## SSL/TLS Support

SSL is configured at the socket level via OpenSSL:

```cpp
#include <zapata/net/socket/socket_stream.h>

// Create an SSL socket stream
zpt::stream::ssl_socket_stream ssl_stream;
ssl_stream.open("example.com", 443);
```

Certificate and key paths are configured in the transport settings.

## See Also

- [HTTP API Reference](../../api-reference/http.md) - Parser API details
- [Transport Guide](transports.md) - Transport abstraction
- [WebSocket Guide](websocket.md) - WebSocket protocol
