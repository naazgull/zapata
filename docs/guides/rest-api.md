# Building REST APIs

This guide covers endpoint registration, request handling, and response formatting with Zapata's REST engine.

## Setup

```cpp
#include <zapata/rest.h>
```

## Registering Endpoints

### Basic Handlers

Register handlers by performative (HTTP method) and URI pattern:

```cpp
auto& boot = zpt::BOOT_ENGINE();

// GET /api/users
boot.add_handler(zpt::Get, "/api/users",
    [](zpt::performative _method,
       zpt::json _envelope,
       zpt::json _opts) -> zpt::json {
        return {
            "status", 200,
            "body", { zpt::array,
                { "id", 1, "name", "Alice" },
                { "id", 2, "name", "Bob" }
            }
        };
    });
```

### All HTTP Methods

```cpp
boot.add_handler(zpt::Get,    "/api/items",     handle_list);
boot.add_handler(zpt::Post,   "/api/items",     handle_create);
boot.add_handler(zpt::Get,    "/api/items/{id}", handle_get);
boot.add_handler(zpt::Put,    "/api/items/{id}", handle_replace);
boot.add_handler(zpt::Patch,  "/api/items/{id}", handle_update);
boot.add_handler(zpt::Delete, "/api/items/{id}", handle_delete);
boot.add_handler(zpt::Head,   "/api/items/{id}", handle_head);
```

## Request Handling

### The Request Envelope

Every handler receives a JSON envelope containing request data:

```cpp
[](zpt::performative _method,
   zpt::json _envelope,
   zpt::json _opts) -> zpt::json {

    // URI and path parameters
    auto uri = _envelope["uri"];
    auto params = _envelope["params"];   // Path params like {id}

    // Query parameters
    auto query = _envelope["query"];     // ?key=value

    // Headers
    auto headers = _envelope["headers"];
    auto content_type = headers["Content-Type"];

    // Request body (for POST/PUT/PATCH)
    auto body = _envelope["body"];

    // ...
}
```

### Path Parameters

URI templates with `{param}` are extracted into `_envelope["params"]`:

```cpp
// Route: /api/users/{user_id}/posts/{post_id}
boot.add_handler(zpt::Get, "/api/users/{user_id}/posts/{post_id}",
    [](zpt::performative _method,
       zpt::json _envelope,
       zpt::json _opts) -> zpt::json {
        auto user_id = _envelope["params"]["user_id"];
        auto post_id = _envelope["params"]["post_id"];
        // ...
    });
```

### Query Parameters

```cpp
// GET /api/search?q=term&page=2
boot.add_handler(zpt::Get, "/api/search",
    [](zpt::performative _method,
       zpt::json _envelope,
       zpt::json _opts) -> zpt::json {
        auto query = std::string(_envelope["query"]["q"]);
        auto page = int(_envelope["query"]["page"]);
        // ...
    });
```

## Response Formatting

### Success Responses

```cpp
// 200 OK with body
return {
    "status", 200,
    "body", { "id", 1, "name", "Alice" }
};

// 201 Created with Location header
return {
    "status", 201,
    "headers", { "Location", "/api/users/42" },
    "body", { "id", 42, "name", "New User" }
};

// 204 No Content
return { "status", 204 };
```

### Error Responses

```cpp
// 404 Not Found
return {
    "status", 404,
    "body", { "error", "User not found" }
};

// 400 Bad Request
return {
    "status", 400,
    "body", {
        "error", "Validation failed",
        "details", { zpt::array,
            "name is required",
            "email is invalid"
        }
    }
};

// 500 Internal Server Error
return {
    "status", 500,
    "body", { "error", "Internal server error" }
};
```

## Content Negotiation

Set response content type via headers:

```cpp
return {
    "status", 200,
    "headers", {
        "Content-Type", "text/plain"
    },
    "body", "Hello, plain text!"
};
```

## The REST Resolver

`zpt::rest::resolver_t` manages route matching and dispatch:

- Routes are matched in registration order
- Path parameters (`{id}`) match any non-slash segment
- The resolver selects the most specific matching route

## See Also

- [REST Engine API Reference](../api-reference/rest.md) - Detailed API docs
- [Transport Guide](networking/transports.md) - Transport layer details
- [JSON Guide](json.md) - Working with request/response data
- [Configuration](configuration.md) - Server configuration
