# Building REST APIs

This guide covers endpoint registration, request handling, and response formatting with Zapata's REST engine.

## Setup

```cpp
#include <zapata/rest.h>
```

## Registering Endpoints

### Basic Handlers

Handlers are classes that extend `zpt::events::process`. Register them with the resolver:

```cpp
auto& resolver = zpt::REST_RESOLVER();

// GET /api/users
resolver->add<list_handler>("/api/users");
```

The handler class implements the Operation concept:

```cpp
class list_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }

    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        // Access request data via this->received()
        // Send response via this->to_send()
        this->to_send()->status(200)->body() = zpt::json{
            "status", 200,
            "body", { zpt::array,
                { "id", 1, "name", "Alice" },
                { "id", 2, "name", "Bob" }
            }
        };
        return zpt::events::finish;
    }
};
```

### All HTTP Methods

```cpp
resolver->add<list_handler>    ("/api/items");
resolver->add<create_handler>  ("/api/items");
resolver->add<get_handler>     ("/api/items/{}");
resolver->add<replace_handler> ("/api/items/{}");
resolver->add<update_handler>  ("/api/items/{}");
resolver->add<delete_handler>  ("/api/items/{}");
```

## Request Handling

### The Request Envelope

Every handler receives a `zpt::message` accessible via `this->received()`. The message body contains the request data:

```cpp
auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
    // URI path (array of segments: ["api", "users", "42"])
    auto path = this->received()->uri()("path");
    auto segment1 = path(0);  // "api"
    auto segment2 = path(1);  // "users"
    auto segment3 = path(2);  // "42" — path parameter from {}

    // Query parameters
    auto params = this->received()->uri()("params");     // ?key=value

    // Headers
    auto headers = this->received()->headers();
    auto content_type = headers("Content-Type");

    // Request body (for POST/PUT/PATCH)
    auto body = this->received()->body();

    // ...
    return zpt::events::finish;
}
```

### Path Parameters

Route patterns use `{}` as a wildcard that matches any non-slash segment. Path segments are stored in the `path` field of the URI as a JSON array, so the parameter value is accessed by indexing into `uri("path")`:

```cpp
// Route: /api/users/{}/posts/{}
resolver->add<post_handler>("/api/users/{}/posts/{}");

class post_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto _path = this->received()->uri()("path");
        auto user_id = _path(2);  // segment index 2: the first {} value
        auto post_id = _path(4);  // segment index 4: the second {} value
        return zpt::events::finish;
    }
};
```

### Query Parameters

```cpp
// GET /api/search?q=term&page=2
resolver->add<search_handler>("/api/search");

class search_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto params = this->received()->uri()("params");
        auto query = std::string(params("q"));
        auto page = int(params("page"));
        return zpt::events::finish;
    }
};
```

## Response Formatting

### Success Responses

Send responses via `this->to_send()`:

```cpp
// 200 OK with body
this->to_send()->status(200)->body() = zpt::json{
    "id", 1, "name", "Alice"
};

// 201 Created with Location header
this->to_send()->status(201)
    ->header("Location", "/api/users/42")
    ->body() = { "id", 42, "name", "New User" };

// 204 No Content
this->to_send()->status(204);
```

### Error Responses

```cpp
// 404 Not Found
this->to_send()->status(404)->body() = zpt::json{
    "error", "User not found"
};

// 400 Bad Request
this->to_send()->status(400)->body() = zpt::json{
    "error", "Validation failed",
    "details", { zpt::array,
        "name is required",
        "email is invalid"
    }
};

// 500 Internal Server Error
this->to_send()->status(500)->body() = zpt::json{
    "error", "Internal server error"
};
```

## Content Negotiation

Set response content type via headers:

```cpp
this->to_send()->status(200)
    ->header("Content-Type", "text/plain")
    ->body() = "Hello, plain text!";
```

## The REST Resolver

`zpt::rest::resolver_t` manages route matching and dispatch, inheriting from `zpt::events::resolver_t`.

- Routes are matched in registration order
- Route patterns use `{}` as a wildcard matching any non-slash segment, accessed via `uri("path")(index)`
- The resolver selects the most specific matching route

### Handler Registration Patterns

**Template-based (Operation classes) — recommended:**
```cpp
// Handler class extends zpt::events::process
resolver->add<handler_class>("/path");
```

All examples in the codebase use this pattern. The handler class implements
`zpt::events::Operation` (typically by extending `zpt::events::process`),
and the resolver creates instances when matching requests arrive.

## See Also

- [REST Engine API Reference](../api-reference/rest.md) - Detailed API docs
- [Transport Guide](networking/transports.md) - Transport layer details
- [JSON Guide](json.md) - Working with request/response data
- [Configuration](configuration.md) - Server configuration
