# WebSocket Chat Example

A simple real-time chat application using WebSocket transport.

## Files

- `plugin.cpp` - Plugin with WebSocket message handlers
- `config.json` - Server configuration
- `CMakeLists.txt` - Plugin build configuration

## plugin.cpp

```cpp
#include <zapata/rest.h>
#include <vector>

// Store connected clients
static std::shared_ptr<std::vector<zpt::message>> g_clients;

// Handle incoming chat messages — POST /ws/chat
class chat_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto& clients = *g_clients;
        auto message = this->received()->body();
        auto sender = std::string(message("from")->string());
        auto text = std::string(message("text")->string());

        zlog("Chat: " + sender + ": " + text, zpt::info);

        // Broadcast to all connected clients
        auto response = zpt::json{
            "type", "message",
            "from", sender,
            "text", text,
            "timestamp", zpt::json::date()
        };
        this->to_send()->status(200)->body() = response;
        return zpt::events::finish;
    }
};

// Handle join events — POST /ws/join
class join_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto username = std::string(this->received()->body()("username")->string());
        zlog("User joined: " + username, zpt::info);

        this->to_send()->status(200)->body() = zpt::json{
            "type", "system",
            "text", username + " has joined the chat"
        };
        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading websocket-chat plugin", zpt::info);
    g_clients = std::make_shared<std::vector<zpt::message>>();

    auto _resolver = zpt::REST_RESOLVER();
    _resolver->add<chat_handler>("/ws/chat")
             ->add<join_handler>("/ws/join");
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloading websocket-chat plugin", zpt::info);
    auto _resolver = zpt::REST_RESOLVER();
    _resolver->remove<chat_handler>("/ws/chat")
             ->remove<join_handler>("/ws/join");
}
```

## config.json

```json
{
    "identity": {
        "id": "websocket-chat-uuid",
        "name": "websocket-chat"
    },
    "log": { "level": 6, "format": 1 },
    "load": [
        { "name": "builtin:http" },
        { "name": "builtin:ws" },
        { "name": "builtin:rest" },
        { "name": "websocket-chat", "source": "libwebsocket-chat.so",
          "requires": [ "builtin:rest" ] }
    ],
    "resources": { "limits": { "max_heap_allocation": 1048576 } },
    "dispatcher": { "limits": { "max_workers": 4 } },
    "http": { "bind": "0.0.0.0", "port": 8080 },
    "transport": { "default": "http", "limits": { "max_workers": 16 } }
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

add_library(websocket-chat SHARED plugin.cpp)
target_include_directories(websocket-chat PRIVATE ${ZAPATA_INCLUDE_DIRS})
target_link_libraries(websocket-chat ${ZAPATA_LIBRARIES})
```

## Test

Connect with a WebSocket client (e.g., `websocat`):

```bash
# Terminal 1: Start server with the Zapata host
zpt --config config.json

# Terminal 2: Connect as user
websocat ws://localhost:8080/ws/chat

# Send JSON messages:
{"from": "Alice", "text": "Hello everyone!"}
```

## See Also

- [WebSocket Guide](../../guides/networking/websocket.md) - WebSocket details
- [Transport Guide](../../guides/networking/transports.md) - Transport layer
