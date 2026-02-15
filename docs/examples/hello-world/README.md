# Hello World Example

A minimal REST API with Zapata.

## Files

- `main.cpp` - Application entry point
- `CMakeLists.txt` - Build configuration

## main.cpp

```cpp
#include <zapata/rest.h>

auto main(int _argc, char* _argv[]) -> int {
    zpt::BOOT(_argc, _argv);
    auto& boot = zpt::BOOT_ENGINE();

    boot.add_handler(zpt::Get, "/hello",
        [](zpt::performative _method,
           zpt::json _envelope,
           zpt::json _opts) -> zpt::json {
            return {
                "status", 200,
                "body", { "message", "Hello, World!" }
            };
        });

    boot.add_handler(zpt::Get, "/hello/{name}",
        [](zpt::performative _method,
           zpt::json _envelope,
           zpt::json _opts) -> zpt::json {
            auto name = std::string(_envelope["params"]["name"]);
            return {
                "status", 200,
                "body", { "message", "Hello, " + name + "!" }
            };
        });

    boot.start();
    return 0;
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
