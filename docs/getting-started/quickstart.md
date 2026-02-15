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

auto main(int _argc, char* _argv[]) -> int {
    // Load configuration
    auto _config = zpt::json::object();
    _config["transport"] = {
        "type", "http",
        "bind", "tcp://0.0.0.0:8080"
    };

    // Bootstrap the framework
    zpt::BOOT(_argc, _argv);
    auto& _boot = zpt::BOOT_ENGINE();

    // Register a GET endpoint
    _boot.add_handler(zpt::Get, "/api/hello",
        [](zpt::performative _method,
           zpt::json _envelope,
           zpt::json _opts) -> zpt::json {
            return {
                "status", 200,
                "body", {
                    "message", "Hello from Zapata!"
                }
            };
        });

    // Register a POST endpoint with body
    _boot.add_handler(zpt::Post, "/api/greet",
        [](zpt::performative _method,
           zpt::json _envelope,
           zpt::json _opts) -> zpt::json {
            auto _name = _envelope["body"]["name"];
            return {
                "status", 200,
                "body", {
                    "message",
                    std::string("Hello, ") + std::string(_name) + "!"
                }
            };
        });

    // Start the server
    _boot.start();
    return 0;
}
```

## Build and Run

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./my-api
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
