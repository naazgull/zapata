# Configuration

Zapata uses JSON-based configuration for all framework settings. The configuration is loaded from a file specified via `--config` or from inline JSON in your C++ code.

## Loading Configuration

### From Command Line

```bash
./my-app --config /path/to/config.json
```

### In C++ Code

```cpp
#include <zapata/startup.h>

// Pass config to the boot engine
auto config = zpt::json::object();
config << "identity" << zpt::json{
    "id", "my-uuid",
    "name", "my-app"
} << "log" << zpt::json{ "level", 6, "format", 1 }
   << "load" << zpt::json::array();

zpt::BOOT(config);
```

### Accessing Configuration

```cpp
#include <zapata/startup.h>

// Global configuration accessor
auto config = zpt::GLOBAL_CONFIG();

// Read values from loaded configuration
auto app_name = config("identity")("name");
auto log_level = int(config("log")("level"));
```

## Configuration Structure

A minimal configuration file:

```json
{
    "identity": {
        "id": "my-service-uuid",
        "name": "my-service"
    },
    "log": { "level": 6, "format": 1 },
    "load": [
        { "name": "builtin:http" },
        { "name": "builtin:rest" },
        { "name": "builtin:upnp" },
        { "name": "my-app", "source": "libmy-app.so", "requires": [ "builtin:rest" ] }
    ],
    "resources": {
        "limits": {
            "max_heap_allocation": 0
        }
    },
    "dispatcher": {
        "limits": {
            "max_workers": 4
        }
    },
    "http": { "bind": "0.0.0.0", "port": 8080 },
    "upnp": { "bind": "239.192.1.2", "port": 7979 },
    "transport": {
        "default": "http",
        "limits": {
            "max_workers": 16
        }
    },
    "rest": {
        "prefix": "/api/1.0"
    }
}
```

## Top-Level Keys

### `identity`

Service identity. Each instance needs a unique UUID and a human-readable name.

| Key | Type | Description |
|-----|------|-------------|
| `id` | string | UUID for this service instance |
| `name` | string | Human-readable service name |

### `log`

Logging configuration.

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `level` | int | 6 (info) | Minimum log level (0–9) |
| `format` | int | 1 | Log output format |

Log levels:

| Value | Level |
|-------|-------|
| 0 | emergency |
| 1 | alert |
| 2 | critical |
| 3 | error |
| 4 | warning |
| 5 | notice |
| 6 | info |
| 7 | debug |
| 8 | trace |
| 9 | verbose |

### `load`

Plugin loading list. Each entry specifies a plugin to load with its name, source library, and dependencies.

| Key | Type | Description |
|-----|------|-------------|
| `name` | string | Plugin name (use `builtin:<name>` for built-in plugins) |
| `source` | string | Shared library path (for custom plugins) |
| `requires` | array | Plugin dependencies (names of other plugins) |

Builtin plugins:

| Plugin | Library | Dependencies |
|--------|---------|-------------|
| `builtin:transport` | `libzapata-engine-transport-plugin.so` | — |
| `builtin:self` | `libzapata-net-self-plugin.so` | `builtin:transport` |
| `builtin:http` | `libzapata-net-http-plugin.so` | `builtin:transport`, `builtin:self` |
| `builtin:ws` | `libzapata-net-websocket-plugin.so` | `builtin:transport`, `builtin:self` |
| `builtin:tcp` | `libzapata-net-tcp-plugin.so` | `builtin:transport`, `builtin:self` |
| `builtin:local` | `libzapata-net-local-plugin.so` | `builtin:transport`, `builtin:self` |
| `builtin:pipe` | `libzapata-net-pipe-plugin.so` | `builtin:transport`, `builtin:self` |
| `builtin:upnp` | `libzapata-net-upnp-plugin.so` | `builtin:transport`, `builtin:self` |
| `builtin:identity` | `libzapata-net-identity-plugin.so` | `builtin:transport` |
| `builtin:rest` | `libzapata-engine-rest-plugin.so` | `builtin:transport`, `builtin:self`, `builtin:identity` |
| `builtin:lua` | `libzapata-bridge-lua-plugin.so` | `builtin:rest`, `builtin:transport` |
| `builtin:prolog` | `libzapata-bridge-prolog-plugin.so` | `builtin:rest`, `builtin:transport` |
| `builtin:amqp` | `libzapata-net-amqp-plugin.so` | `builtin:transport`, `builtin:self` |
| `builtin:mqtt` | `libzapata-net-mqtt-plugin.so` | `builtin:transport`, `builtin:self` |
| `builtin:testing` | `libzapata-common-testing-plugin.so` | `builtin:lua`, `builtin:rest`, `builtin:transport` |

### `resources.limits`

Resource limits.

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `max_heap_allocation` | int | 0 (unlimited) | Maximum heap allocation in bytes |

### `dispatcher.limits`

Event dispatcher limits.

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `max_workers` | int | 0 (auto) | Maximum worker threads |

### `transport`

Transport configuration. `default` specifies the primary transport protocol.

| Key | Type | Description |
|-----|------|-------------|
| `default` | string | Default transport protocol (`http`, `tcp`, `ws`, `amqp`, etc.) |
| `limits.max_workers` | int | Max workers for this transport |

### `rest`

REST engine configuration.

| Key | Type | Description |
|-----|------|-------------|
| `prefix` | string | API URL prefix (e.g., `/api/1.0`) |

## Transport-Specific Configuration

Each transport plugin can have its own configuration section. The transport name is used as the key:

```json
{
    "http": { "bind": "0.0.0.0", "port": 8080 },
    "tcp": { "bind": "0.0.0.0", "port": 8081 },
    "amqp": {
        "address": "127.0.0.1",
        "port": 5672,
        "subscribe": [ "/queues/jobs" ]
    },
    "mqtt": { "address": "127.0.0.1", "port": 1883 },
    "upnp": { "bind": "239.192.1.2", "port": 7979 }
}
```

## Lua Configuration

The Lua bridge has additional configuration for module loading and execution:

```json
{
    "lua": {
        "modules": [
            { "module": "example_module", "file": "./scripts/module.lua" }
        ],
        "exec": [
            {
                "module": "example_module",
                "function": "process",
                "args": []
            }
        ]
    }
}
```

## Prolog Configuration

The Prolog bridge:

```json
{
    "prolog": {
        "modules": [
            { "module": "example_consumer", "file": "./consumer.pl" }
        ],
        "exec": [
            "call(consume)"
        ]
    }
}
```

## Configuration Merging

Use the JSON merge operator (`|`) to combine defaults with overrides:

```cpp
zpt::json defaults = {
    "identity", { "id", "default-id", "name", "default" },
    "log", { "level", 6, "format", 1 },
    "http", { "bind", "0.0.0.0", "port", 8080 }
};

// Load overrides from file
std::ifstream file(config_file);
zpt::json overrides;
file >> overrides;

// Merge: overrides win on conflicts
zpt::json config = defaults | overrides;
zpt::BOOT(config);
```

## Environment-Specific Configuration

Organize configs by environment:

```
config/
├── base.json           # Shared defaults (identity, log, plugins)
├── development.json    # Dev overrides (ports, debug levels)
├── production.json     # Production overrides (bind addresses)
```

Load and merge at startup:

```cpp
auto base = load_json("config/base.json");
auto env = load_json("config/" + environment + ".json");
auto config = base | env;
zpt::BOOT(config);
```

## See Also

- [Architecture Overview](../architecture/overview.md) - System design
- [Building REST APIs](rest-api.md) - Using configuration in REST services
- [Plugin System](../architecture/plugins.md) - Plugin configuration
- [Installation](../getting-started/installation.md) - Build setup
