# Hello World Example

A minimal REST API with Zapata.

## Files

- `plugin.cpp` - Plugin with REST handler
- `config.json` - Server configuration
- `CMakeLists.txt` - Plugin build configuration

## plugin.cpp

```cpp
#include <zapata/rest.h>
#include <zapata/startup.h>

// Simple handler — responds with a greeting
class hello_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        this->to_send()->status(200)->body() = zpt::json{
            "message", "Hello, World!"
        };
        return zpt::events::finish;
    }
};

// Parameterized handler — greets by name from URI path
class hello_name_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto _path = this->received()->uri()("path");
        auto name = _path(1)->string();
        this->to_send()->status(200)->body() = zpt::json{
            "message", std::string("Hello, ") + name + "!"
        };
        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading hello-world plugin", zpt::info);
    auto _resolver = zpt::REST_RESOLVER();
    _resolver->add<hello_handler>("/hello")
             ->add<hello_name_handler>("/hello/{}");
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloading hello-world plugin", zpt::info);
    auto _resolver = zpt::REST_RESOLVER();
    _resolver->remove<hello_handler>("/hello")
             ->remove<hello_name_handler>("/hello/{}");
}
```

## config.json

```json
{
    "identity": {
        "id": "hello-world-uuid",
        "name": "hello-world"
    },
    "log": { "level": 6, "format": 1 },
    "load": [
        { "name": "builtin:http" },
        { "name": "builtin:rest" },
        { "name": "hello-world", "source": "libhello-world.so", "requires": [ "builtin:rest" ] }
    ],
    "resources": { "limits": { "max_heap_allocation": 0 } },
    "dispatcher": { "limits": { "max_workers": 4 } },
    "http": { "bind": "0.0.0.0", "port": 8080 },
    "transport": { "default": "http", "limits": { "max_workers": 16 } },
    "rest": { "prefix": "/api" }
}
```

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.18)
project(hello-world CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZAPATA REQUIRED
    zapata-base zapata-parser-json zapata-events
    zapata-ontology zapata-net-transport zapata-net-http
    zapata-engine-startup zapata-engine-rest zapata-engine-transport
)

add_library(hello-world SHARED plugin.cpp)
target_include_directories(hello-world PRIVATE ${ZAPATA_INCLUDE_DIRS})
target_link_libraries(hello-world ${ZAPATA_LIBRARIES})
```

## Build and Run

The plugin builds as a shared library (`libhello-world.so`). It is loaded by the Zapata host process at runtime via the `load` configuration.

```bash
mkdir build && cd build
cmake .. && make
zpt --config ../config.json
```

No bootstrap or `main` is needed — the framework's host process loads plugins through `_zpt_load_` and manages the event loop.

## Test

```bash
curl http://localhost:8080/api/hello
# {"message":"Hello, World!"}

curl http://localhost:8080/api/hello/Zapata
# {"message":"Hello, Zapata!"}
```

## See Also

- [Building REST APIs](../../guides/rest-api.md) - REST guide
- [REST Engine API Reference](../../api-reference/rest.md) - API details
- [Quickstart](../../getting-started/quickstart.md) - Getting started
