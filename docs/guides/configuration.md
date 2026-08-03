# Configuration

Zapata uses JSON-based configuration for all framework settings.

## Loading Configuration

### From Command Line

```bash
./my-app --config /path/to/config.json
```

### Accessing Configuration

```cpp
#include <zapata/startup.h>

// Global configuration accessor
auto config = zpt::GLOBAL_CONFIG();

// Read values
auto port = int(config["transport"]["port"]);
auto db_path = std::string(config["storage"]["sqlite"]["path"]);
```

## Configuration Structure

A typical configuration file:

```json
{
    "transport": {
        "type": "http",
        "bind": "tcp://0.0.0.0:8080",
        "threads": 4
    },
    "storage": {
        "sqlite": {
            "path": "/var/lib/myapp/data.db"
        },
        "mysqlx": {
            "host": "localhost",
            "port": 3306,
            "database": "myapp",
            "user": "root",
            "password": ""
        }
    },
    "log": {
        "level": "info",
        "file": "/var/log/myapp/app.log"
    },
    "plugins": [
        {
            "name": "http-transport",
            "path": "/usr/local/lib/libzapata-net-http.so"
        }
    ]
}
```

## Common Configuration Keys

### Transport

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `transport.type` | string | `"http"` | Transport protocol |
| `transport.bind` | string | — | Bind address (e.g., `"tcp://0.0.0.0:8080"`) |
| `transport.threads` | int | CPU count | Number of I/O threads |

### Network Interface Placeholders

Bind addresses support network interface placeholders that resolve at startup:

```json
{
    "transport": {
        "bind": "tcp://{eth0}:8080"
    }
}
```

`{eth0}` is replaced with the actual IP address of the `eth0` interface.

### Storage

| Key | Type | Description |
|-----|------|-------------|
| `storage.sqlite.path` | string | SQLite database file path |
| `storage.sqlite.memory` | bool | Use in-memory database |
| `storage.mysqlx.host` | string | MySQL server hostname |
| `storage.mysqlx.port` | int | MySQL server port |
| `storage.mysqlx.database` | string | Database name |
| `storage.mysqlx.user` | string | Authentication user |
| `storage.mysqlx.password` | string | Authentication password |

### Logging

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `log.level` | string | `"info"` | Min level: `"trace"`, `"debug"`, `"info"`, `"warning"`, `"error"` |
| `log.file` | string | stdout | Log file path |

### Plugins

```json
{
    "plugins": [
        {
            "name": "plugin-name",
            "path": "/path/to/libplugin.so",
            "config": { }
        }
    ]
}
```

## Configuration Merging

Use JSON merge operator to combine defaults with overrides:

```cpp
zpt::json defaults = {
    "transport", { "type", "http", "bind", "tcp://0.0.0.0:8080" },
    "log", { "level", "info" }
};

// Load from file
std::ifstream file(config_file);
zpt::json overrides;
file >> overrides;

// Merge: overrides win on conflicts
auto config = defaults | overrides;
```

## Environment-Specific Configuration

Organize configs by environment:

```
config/
├── base.json           # Shared defaults
├── development.json    # Dev overrides
├── staging.json        # Staging overrides
└── production.json     # Production overrides
```

Load and merge at startup:

```cpp
auto base = load_json("config/base.json");
auto env = load_json("config/" + environment + ".json");
auto config = base | env;
```

## See Also

- [Architecture Overview](../architecture/overview.md) - System design
- [Building REST APIs](rest-api.md) - Using configuration in REST services
- [Plugin System](../architecture/plugins.md) - Plugin configuration
