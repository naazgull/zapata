# Project Structure

How to organize a Zapata-based project as it grows.

## Recommended Layout

```
my-project/
├── CMakeLists.txt          # Top-level build configuration
├── config/
│   ├── development.json    # Dev configuration
│   └── production.json     # Production configuration
├── src/
│   ├── main.cpp            # Entry point and bootstrap
│   ├── handlers/           # REST endpoint handlers
│   │   ├── users.cpp
│   │   └── products.cpp
│   └── plugins/            # Custom plugins
│       └── auth.cpp
├── include/
│   └── my-project/
│       ├── handlers.h
│       └── plugins.h
└── tests/
    └── test_handlers.cpp
```

## Configuration Files

Zapata uses JSON-based configuration. Place config files in a `config/` directory:

```json
{
    "identity": {
        "id": "my-project-uuid",
        "name": "my-project"
    },
    "log": { "level": 6, "format": 1 },
    "load": [
        { "name": "builtin:http" },
        { "name": "builtin:rest" },
        { "name": "builtin:upnp" }
    ],
    "resources": { "limits": { "max_heap_allocation": 0 } },
    "dispatcher": { "limits": { "max_workers": 4 } },
    "http": { "bind": "0.0.0.0", "port": 8080 },
    "upnp": { "bind": "239.192.1.2", "port": 7979 },
    "transport": { "default": "http", "limits": { "max_workers": 16 } },
    "rest": { "prefix": "/api" }
}
```

Pass the config file at startup:

```bash
./my-project --config config/development.json
```

## CMake Integration

### Using pkg-config

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZAPATA REQUIRED
    zapata-base
    zapata-parser-json
    zapata-engine-rest
)
target_include_directories(myapp PRIVATE ${ZAPATA_INCLUDE_DIRS})
target_link_libraries(myapp ${ZAPATA_LIBRARIES})
```

### Selecting Components

Only link what you need:

| Component | pkg-config name | Purpose |
|-----------|----------------|---------|
| Core | `zapata-base` | Utilities, exceptions, crypto |
| JSON | `zapata-parser-json` | JSON parsing |
| Events | `zapata-events` | Event dispatcher |
| REST | `zapata-engine-rest` | REST API engine |
| HTTP | `zapata-net-http` | HTTP transport |
| WebSocket | `zapata-net-websocket` | WebSocket transport |
| SQLite | `zapata-storage-sqlite` | SQLite database |
| MySQL | `zapata-storage-mysqlx` | MySQL database |
| Lua | `zapata-bridge-lua` | Lua scripting |

## Plugin Architecture

As your project grows, organize functionality into plugins (shared libraries):

```
my-project/
├── plugins/
│   ├── CMakeLists.txt
│   ├── auth-plugin/
│   │   ├── CMakeLists.txt
│   │   └── src/auth.cpp
│   └── cache-plugin/
│       ├── CMakeLists.txt
│       └── src/cache.cpp
└── src/
    └── main.cpp
```

Each plugin is built as a shared library and loaded at startup based on configuration:

```json
{
    "identity": { "id": "my-project-uuid", "name": "my-project" },
    "log": { "level": 6, "format": 1 },
    "load": [
        { "name": "builtin:http" },
        { "name": "builtin:rest" },
        { "name": "builtin:upnp" },
        { "name": "auth-plugin", "source": "./plugins/libauth-plugin.so",
          "requires": [ "builtin:rest" ] },
        { "name": "cache-plugin", "source": "./plugins/libcache-plugin.so",
          "requires": [ "builtin:rest" ] }
    ],
    "resources": { "limits": { "max_heap_allocation": 0 } },
    "dispatcher": { "limits": { "max_workers": 4 } },
    "http": { "bind": "0.0.0.0", "port": 8080 },
    "upnp": { "bind": "239.192.1.2", "port": 7979 },
    "transport": { "default": "http", "limits": { "max_workers": 16 } },
    "rest": { "prefix": "/api" }
}
```

Plugins are loaded in dependency order. The `requires` field ensures dependencies are loaded first. Built-in plugins use the `builtin:` prefix (e.g., `builtin:rest`, `builtin:http`) and don't need a `source` path.

## Module Organization

For larger projects, mirror Zapata's own modular structure:

```
my-project/
├── common/           # Shared types and utilities
├── handlers/         # HTTP endpoint handlers
├── services/         # Business logic
├── storage/          # Data access layer
└── transport/        # Custom protocol implementations
```

Each module can have its own `CMakeLists.txt` and produce a separate library, keeping compilation units small and build times manageable.

## See Also

- [Installation](installation.md) - Build requirements and setup
- [Quickstart](quickstart.md) - Build your first API
- [Architecture Overview](../architecture/overview.md) - System design
- [Plugin System](../architecture/plugins.md) - Plugin architecture details
