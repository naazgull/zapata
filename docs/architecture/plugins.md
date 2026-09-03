# Plugin System

Zapata uses a plugin architecture where protocols, storage backends, and application logic are loaded as shared libraries at runtime.

## Plugin Lifecycle

```
┌──────────┐    ┌──────────┐    ┌──────────┐
│  LOADED  │───▶│ LOADED   │───▶│ STOPPED  │
└──────────┘    │ (running)│    └──────────┘
                └──────────┘
      _zpt_load_()    _zpt_unload__()
```

### States

| Constant | Value | Description |
|----------|-------|-------------|
| `PLUGIN_STATE_UNLOADED` | 0 | Plugin not yet loaded |
| `PLUGIN_STATE_IN_SHUTDOWN` | 1 | Plugin is shutting down |
| `PLUGIN_STATE_LOADED` | 2 | Plugin is loaded and running |

## The Plugin Class

`zpt::plugin` manages individual plugin lifecycles:

```cpp
class plugin {
public:
    plugin(zpt::json _options, zpt::json _config);

    auto name() -> std::string&;           // Plugin name
    auto source() -> std::string&;         // Shared library path
    auto config() -> zpt::json&;           // Plugin configuration
    auto is_shutdown_ongoing() -> bool;    // True if shutdown in progress
    auto is_loaded() -> bool;              // True if plugin is loaded
    auto is_unloaded() -> bool;            // True if plugin is unloaded
    auto add_thread(std::function<void()> _callback) -> plugin&;  // Register worker thread
};
```

## The Boot Engine

`zpt::startup::boot` orchestrates the startup sequence:

```cpp
// Get the boot engine
auto& boot = zpt::BOOT();

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

Plugins are specified in the `load` array of the configuration:

```json
{
    "identity": { "id": "uuid", "name": "my-app" },
    "log": { "level": 6, "format": 1 },
    "load": [
        { "name": "builtin:http" },
        { "name": "builtin:rest" },
        { "name": "http-transport", "source": "/usr/local/lib/libzapata-net-http.so",
          "requires": [ "builtin:rest" ] },
        { "name": "sqlite-storage", "source": "/usr/local/lib/libzapata-storage-sqlite.so",
          "requires": [ "builtin:rest" ] }
    ],
    "resources": { "limits": { "max_heap_allocation": 0 } },
    "dispatcher": { "limits": { "max_workers": 4 } },
    "http": { "bind": "0.0.0.0", "port": 8080 },
    "transport": { "default": "http", "limits": { "max_workers": 16 } }
}
```

Each entry in `load` specifies:
- `name` — plugin identifier (use `builtin:` prefix for built-in plugins)
- `source` — path to the shared library (for custom plugins)
- `requires` — array of plugin names that must be loaded first

## Writing a Custom Plugin

### 1. Create the Plugin Library

Plugins export `_zpt_load_()` and `_zpt_unload_()` functions:

```cpp
// my_plugin.cpp
#include <zapata/rest.h>

// Called when the plugin is loaded
extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    auto& config = _plugin.config();
    zlog("My plugin loaded with config: " + config.stringify(), zpt::info);

    // Register REST endpoints
    auto resolver = zpt::REST_RESOLVER();
    resolver->add<my_handler>("/api/my-endpoint");

    // Register a worker thread
    _plugin.add_thread([]() {
        // Background work loop
        while (!true) { /* check shutdown */ std::this_thread::sleep_for(std::chrono::seconds(1)); }
    });
}

// Called when the plugin is unloaded
extern "C" auto _zpt_unload_(zpt::plugin& _plugin) -> void {
    zlog("My plugin unloaded", zpt::info);
}
```

### 2. Build as Shared Library

```cmake
add_library(my-plugin SHARED src/my_plugin.cpp)
target_link_libraries(my-plugin zapata-base zapata-engine-rest)
set_target_properties(my-plugin PROPERTIES PREFIX "")
```

### 3. Register in Configuration

Add to the `load` array in your configuration file:

```json
{
    "identity": { "id": "uuid", "name": "my-app" },
    "log": { "level": 6, "format": 1 },
    "load": [
        { "name": "builtin:http" },
        { "name": "builtin:rest" },
        { "name": "my-plugin", "source": "./libmy-plugin.so",
          "requires": [ "builtin:rest" ] }
    ],
    "resources": { "limits": { "max_heap_allocation": 0 } },
    "dispatcher": { "limits": { "max_workers": 4 } },
    "http": { "bind": "0.0.0.0", "port": 8080 },
    "transport": { "default": "http", "limits": { "max_workers": 16 } }
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

## Worker Threads

Plugins can register worker threads that run alongside the main event loop:

```cpp
extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    _plugin.add_thread([]() {
        // This runs in a separate thread
        while (!_plugin.is_shutdown_ongoing()) {
            // Do periodic work
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });
}
```

## See Also

- [Architecture Overview](overview.md) - High-level design
- [Component Architecture](components.md) - Module dependencies
- [REST Engine API](../api-reference/rest.md) - REST engine details
- [Custom Transport Guide](../extending/custom-transport.md) - Writing transport plugins
