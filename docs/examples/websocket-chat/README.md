# WebSocket Chat Example

A simple real-time chat using WebSocket transport.

## Files

- `main.cpp` - Server with WebSocket message handling
- `CMakeLists.txt` - Build configuration

## main.cpp

```cpp
#include <zapata/rest.h>
#include <zapata/websocket.h>

auto main(int _argc, char* _argv[]) -> int {
    zpt::BOOT(_argc, _argv);
    auto& boot = zpt::BOOT_ENGINE();

    // Store connected clients
    auto clients = std::make_shared<std::vector<zpt::json>>();

    // Handle incoming chat messages
    boot.add_handler(zpt::Post, "/ws/chat",
        [clients](zpt::performative _method,
                  zpt::json _envelope,
                  zpt::json _opts) -> zpt::json {
            auto message = _envelope["body"];
            auto sender = std::string(message["from"]);
            auto text = std::string(message["text"]);

            zlog("Chat: " + sender + ": " + text, zpt::info);

            // Broadcast to all clients
            return {
                "status", 200,
                "body", {
                    "type", "message",
                    "from", sender,
                    "text", text,
                    "timestamp", zpt::json::date()
                }
            };
        });

    // Handle join events
    boot.add_handler(zpt::Post, "/ws/join",
        [](zpt::performative _method,
           zpt::json _envelope,
           zpt::json _opts) -> zpt::json {
            auto username = std::string(_envelope["body"]["username"]);
            zlog("User joined: " + username, zpt::info);

            return {
                "status", 200,
                "body", {
                    "type", "system",
                    "text", username + " has joined the chat"
                }
            };
        });

    boot.start();
    return 0;
}
```

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.18)
project(websocket-chat CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZAPATA REQUIRED
    zapata-base zapata-parser-json zapata-events zapata-ontology
    zapata-net-transport zapata-net-http zapata-net-websocket
    zapata-engine-startup zapata-engine-rest zapata-engine-transport
)

add_executable(websocket-chat main.cpp)
target_include_directories(websocket-chat PRIVATE ${ZAPATA_INCLUDE_DIRS})
target_link_libraries(websocket-chat ${ZAPATA_LIBRARIES})
```

## Configuration

`config.json`:

```json
{
    "transports": [
        { "type": "http", "bind": "tcp://0.0.0.0:8080" },
        { "type": "ws", "bind": "tcp://0.0.0.0:8081" }
    ]
}
```

## Test

Connect with a WebSocket client (e.g., `websocat`):

```bash
# Terminal 1: Start server
./websocket-chat --config config.json

# Terminal 2: Connect as user
websocat ws://localhost:8081/ws/chat

# Send JSON messages:
{"from": "Alice", "text": "Hello everyone!"}
```
