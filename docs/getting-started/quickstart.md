# Quickstart

Build your first REST API with Zapata in under 5 minutes.

## Prerequisites

Make sure you have [installed Zapata](installation.md) and its dependencies.

## Create a Project

```bash
mkdir my-api && cd my-api
```

Create `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.18)
project(my-api CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZAPATA REQUIRED
    zapata-base
    zapata-parser-json
    zapata-events
    zapata-ontology
    zapata-io-stream
    zapata-io-socket
    zapata-net-transport
    zapata-net-http
    zapata-engine-startup
    zapata-engine-rest
    zapata-engine-transport
)

add_library(my-api SHARED plugin.cpp)
target_include_directories(my-api PRIVATE ${ZAPATA_INCLUDE_DIRS})
target_link_libraries(my-api ${ZAPATA_LIBRARIES})
```

## Write Your API

Create `plugin.cpp`:

```cpp
#include <zapata/rest.h>
#include <zapata/startup.h>

// Handle GET /api/hello
class hello_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        this->to_send()->status(200)->body() = zpt::json{
            "message", "Hello from Zapata!"
        };
        return zpt::events::finish;
    }
};

// Handle POST /api/greet
class greet_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto name = std::string(this->received()->body()("name")->string());
        this->to_send()->status(200)->body() = zpt::json{
            "message", std::string("Hello, ") + name + "!"
        };
        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading my-api plugin", zpt::info);
    auto _resolver = zpt::REST_RESOLVER();

    // Register GET /api/hello
    _resolver->add<hello_handler>("/api/hello");

    // Register POST /api/greet
    _resolver->add<greet_handler>("/api/greet");
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloading my-api plugin", zpt::info);
    auto _resolver = zpt::REST_RESOLVER();
    _resolver->remove<hello_handler>("/api/hello")
             ->remove<greet_handler>("/api/greet");
}
```

## Build and Run

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Create `config.json`:

```json
{
    "identity": {
        "id": "my-api-uuid",
        "name": "my-api"
    },
    "log": { "level": 6, "format": 1 },
    "load": [
        { "name": "builtin:http" },
        { "name": "builtin:rest" },
        { "name": "my-api", "source": "libmy-api.so", "requires": [ "builtin:rest" ] }
    ],
    "resources": { "limits": { "max_heap_allocation": 0 } },
    "dispatcher": { "limits": { "max_workers": 4 } },
    "http": { "bind": "0.0.0.0", "port": 8080 },
    "transport": { "default": "http", "limits": { "max_workers": 16 } },
    "rest": { "prefix": "/api" }
}
```

Run with the `zpt` host process:

```bash
zpt --config config.json
```

The server starts on port 8080.

## Test Your API

```bash
# GET request
curl http://localhost:8080/api/hello
# {"message":"Hello from Zapata!"}

# POST with body
curl -X POST http://localhost:8080/api/greet \
     -H "Content-Type: application/json" \
     -d '{"name": "World"}'
# {"message":"Hello, World!"}
```

## What's Next

- [Project Structure](project-structure.md) - Organize your code as it grows
- [Building REST APIs](../guides/rest-api.md) - Endpoints, routing, and middleware
- [Working with JSON](../guides/json.md) - JSON parsing and manipulation
- [Configuration](../guides/configuration.md) - Configuration options
