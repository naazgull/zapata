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

add_executable(my-api src/main.cpp)
target_include_directories(my-api PRIVATE ${ZAPATA_INCLUDE_DIRS})
target_link_libraries(my-api ${ZAPATA_LIBRARIES})
```

## Write Your API

Create `src/main.cpp`:

```cpp
#include <zapata/rest.h>

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

auto main(int _argc, char* _argv[]) -> int {
    zpt::BOOT(_argc, _argv);

    auto& resolver = zpt::REST_RESOLVER();

    // Register GET /api/hello
    resolver->add<hello_handler>("/api/hello");

    // Register POST /api/greet
    resolver->add<greet_handler>("/api/greet");

    // Start the transport engine
    zpt::TRANSPORT_ENGINE();
    zpt::DISPATCHER()->trap();

    return 0;
}
```

## Build and Run

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./my-api --config ../config.json
```

Create a config file `../config.json`:

```json
{
    "transport": {
        "type": "http",
        "bind": "tcp://0.0.0.0:8080"
    }
}
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
