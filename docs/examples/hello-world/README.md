# Hello World Example

A minimal REST API with Zapata.

## Files

- `main.cpp` - Application entry point
- `config.json` - Server configuration
- `CMakeLists.txt` - Build configuration

## main.cpp

```cpp
#include <zapata/rest.h>

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
        auto name = this->received()->uri()("params")("name")->string();
        this->to_send()->status(200)->body() = zpt::json{
            "message", std::string("Hello, ") + name + "!"
        };
        return zpt::events::finish;
    }
};

auto main(int _argc, char* _argv[]) -> int {
    zpt::BOOT(_argc, _argv);

    auto& resolver = zpt::REST_RESOLVER();

    // Register handlers by path
    resolver->add<hello_handler>("/hello")
        ->add<hello_name_handler>("/hello/{name}");

    // Start transport engine and block until shutdown
    zpt::TRANSPORT_ENGINE();
    zpt::DISPATCHER()->trap();

    return 0;
}
```

## config.json

```json
{
    "transport": {
        "type": "http",
        "bind": "tcp://0.0.0.0:8080"
    }
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

add_executable(hello-world main.cpp)
target_include_directories(hello-world PRIVATE ${ZAPATA_INCLUDE_DIRS})
target_link_libraries(hello-world ${ZAPATA_LIBRARIES})
```

## Build and Run

```bash
mkdir build && cd build
cmake .. && make
./hello-world --config ../config.json
```

## Test

```bash
curl http://localhost:8080/hello
# {"message":"Hello, World!"}

curl http://localhost:8080/hello/Zapata
# {"message":"Hello, Zapata!"}
```

## See Also

- [Building REST APIs](../../guides/rest-api.md) - REST guide
- [REST Engine API Reference](../../api-reference/rest.md) - API details
- [Quickstart](../../getting-started/quickstart.md) - Getting started
