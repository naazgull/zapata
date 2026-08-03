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
resolver->add<get_handler>     ("/api/items/{id}");
resolver->add<replace_handler> ("/api/items/{id}");
resolver->add<update_handler>  ("/api/items/{id}");
resolver->add<delete_handler>  ("/api/items/{id}");
```

## Request Handling

### The Request Envelope

Every handler receives a `zpt::message` accessible via `this->received()`. The message body contains the request data:

```cpp
auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
    // URI and path parameters
    auto uri = this->received()->uri();
    auto params = uri("params");   // Path params like {id}

    // Query parameters
    auto query = uri("query");     // ?key=value

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

URI templates with `{param}` are extracted into the URI's `params` field:

```cpp
// Route: /api/users/{user_id}/posts/{post_id}
resolver->add<post_handler>("/api/users/{user_id}/posts/{post_id}");

class post_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto user_id = this->received()->uri()("params")("user_id");
        auto post_id = this->received()->uri()("params")("post_id");
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
        auto query = std::string(this->received()->uri()("query")("q"));
        auto page = int(this->received()->uri()("query")("page"));
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
- Path parameters (`{id}`) match any non-slash segment
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
