# Plugin System

Zapata uses a plugin architecture where protocols, storage backends, and application logic are loaded as shared libraries at runtime.

## Plugin Lifecycle

```
┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
│  LOADED  │───▶│  BOOTED  │───▶│ RUNNING  │───▶│ STOPPED  │
└──────────┘    └──────────┘    └──────────┘    └──────────┘
   dlopen()      initialize()     start()        shutdown()
```

### States

| Constant | Value | Description |
|----------|-------|-------------|
| `BOOT_ENGINE` | 0 | Engine initialization phase |
| `SEARCH_ENGINE` | 1 | Engine discovery phase |
| `LOAD` | 0 | Plugin loaded into memory |
| `INITIALIZE` | 1 | Plugin initialized |
| `START` | 2 | Plugin running |
| `STOP` | 3 | Plugin shutting down |

## The Plugin Class

`zpt::plugin` manages individual plugin lifecycles:

```cpp
class plugin {
public:
    plugin(std::string const& _name, zpt::json _config);

    auto name() const -> std::string;
    auto config() const -> zpt::json;

    auto load() -> plugin&;         // dlopen the shared library
    auto initialize() -> plugin&;   // Call plugin init function
    auto start() -> plugin&;        // Begin processing
    auto stop() -> plugin&;         // Graceful shutdown
};
```

## The Boot Engine

`zpt::startup::boot` orchestrates the startup sequence:

```cpp
// Get the boot engine
auto& boot = zpt::BOOT_ENGINE();

// Access global configuration
auto config = zpt::GLOBAL_CONFIG();

// Access service identity
auto identity = zpt::IDENTITY();
```

The boot sequence:
1. Parse command-line arguments
2. Load configuration file
3. Discover and load plugins
4. Initialize all plugins in dependency order
5. Start event loops and transports

## Configuration-Driven Loading

Plugins are specified in configuration:

```json
{
    "plugins": [
        {
            "name": "http-transport",
            "path": "/usr/local/lib/libzapata-net-http.so",
            "config": {
                "bind": "tcp://0.0.0.0:8080"
            }
        },
        {
            "name": "sqlite-storage",
            "path": "/usr/local/lib/libzapata-storage-sqlite.so",
            "config": {
                "path": "./data/app.db"
            }
        }
    ]
}
```

## Writing a Custom Plugin

### 1. Create the Plugin Library

```cpp
// my_plugin.cpp
#include <zapata/startup.h>

extern "C" auto zpt_plugin_init(zpt::json _config) -> void {
    // Register handlers, initialize resources
    zlog("My plugin initialized", zpt::info);
}

extern "C" auto zpt_plugin_destroy() -> void {
    // Cleanup resources
    zlog("My plugin destroyed", zpt::info);
}
```

### 2. Build as Shared Library

```cmake
add_library(my-plugin SHARED src/my_plugin.cpp)
target_link_libraries(my-plugin zapata-base zapata-engine-startup)
```

### 3. Register in Configuration

```json
{
    "plugins": [
        {
            "name": "my-plugin",
            "path": "./lib/libmy-plugin.so"
        }
    ]
}
```

## Built-in Plugins

Zapata includes several built-in plugins:

| Plugin | Library | Purpose |
|--------|---------|---------|
| HTTP Transport | `libzapata-net-http` | HTTP/1.1 server |
| WebSocket Transport | `libzapata-net-websocket` | WebSocket server |
| TCP Transport | `libzapata-net-tcp` | Raw TCP connections |
| SQLite Storage | `libzapata-storage-sqlite` | SQLite database |
| MySQL Storage | `libzapata-storage-mysqlx` | MySQL database |
| Lua Bridge | `libzapata-bridge-lua` | Lua scripting |

## See Also

- [Architecture Overview](overview.md) - High-level design
- [Component Architecture](components.md) - Module dependencies
- [REST Engine API](../api-reference/rest.md) - REST engine details
