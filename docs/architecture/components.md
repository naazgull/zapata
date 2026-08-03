# Component Architecture

Detailed module relationships and dependency structure in Zapata.

## Module Categories

Zapata organizes its modules into categories. Each produces one or more shared libraries following the naming convention `libzapata-{category}-{name}`.

### Foundation (`common/`)

| Module | Library | Dependencies | Purpose |
|--------|---------|-------------|---------|
| base | `libzapata-base` | OpenSSL | Utilities, exceptions, crypto, logging |
| catalog | `libzapata-catalog` | base | Key-value catalog with pattern matching |
| events | `libzapata-events` | base, lockfree | Event dispatcher and routing |
| globals | `libzapata-globals` | base | Global and thread-local variable storage |
| graph | `libzapata-graph` | base | Graph data structures |
| lockfree | `libzapata-lockfree` | base | Lock-free data structures |
| ontology | `libzapata-ontology` | base, parser-json | Messages and performatives |
| testing | `libzapata-testing` | base, parser-json | Testing utilities |

### Parsers (`parsers/`)

| Module | Library | Dependencies | Purpose |
|--------|---------|-------------|---------|
| functional | `libzapata-parser-functional` | base | Functional parser combinators |
| json | `libzapata-parser-json` | base | JSON parsing and serialization |
| http | `libzapata-parser-http` | base, parser-json | HTTP message parsing |
| uri | `libzapata-parser-uri` | base | URI parsing |

### I/O (`io/`)

| Module | Library | Dependencies | Purpose |
|--------|---------|-------------|---------|
| stream | `libzapata-io-stream` | base | Stream abstraction, systemd journal |
| socket | `libzapata-io-socket` | base, io-stream, OpenSSL | TCP/UDP/Unix sockets with SSL |
| pipe | `libzapata-io-pipe` | base, io-stream | Named pipe IPC |

### Network (`network/`)

| Module | Library | Dependencies | Purpose |
|--------|---------|-------------|---------|
| transport | `libzapata-net-transport` | base, events, ontology | Transport abstraction layer |
| http | `libzapata-net-http` | transport, parser-http, io-socket | HTTP transport |
| tcp | `libzapata-net-tcp` | transport, io-socket | Raw TCP transport |
| websocket | `libzapata-net-websocket` | transport, parser-http, io-socket | WebSocket transport |
| local | `libzapata-net-local` | transport, io-socket | Unix domain socket transport |
| pipe | `libzapata-net-pipe` | transport, io-pipe | Named pipe transport |
| self | `libzapata-net-self` | transport | In-process callback transport |
| upnp | `libzapata-net-upnp` | transport, io-socket | UPnP/SSDP discovery |
| identity | `libzapata-net-identity-plugin` | base, transport, startup | Network identity management |
| amqp | `libzapata-net-amqp` | transport, io-socket | AMQP protocol |
| mqtt | `libzapata-net-mqtt` | transport, io-socket | MQTT protocol |

### Storage (`storage/`)

| Module | Library | Dependencies | Purpose |
|--------|---------|-------------|---------|
| connector | `libzapata-storage-connector` | base, parser-json | Abstract connector interface |
| sqlite | `libzapata-storage-sqlite` | connector, SQLite3 | SQLite backend |
| mysqlx | `libzapata-storage-mysqlx` | connector, MySQL client | MySQL backend |
| pgsql | `libzapata-storage-pgsql` | connector, PostgreSQL client | PostgreSQL backend |
| mongodb | `libzapata-storage-mongodb` | connector, MongoDB client | MongoDB backend |

### Engines (`engines/`)

| Module | Library | Dependencies | Purpose |
|--------|---------|-------------|---------|
| startup | `libzapata-engine-startup` | base, events | Configuration and bootstrap |
| rest | `libzapata-engine-rest` | startup, transport, ontology | REST API engine |
| transport | `libzapata-engine-transport` | startup, transport | Transport lifecycle |
| runtime | `libzapata-engine-runtime` | startup, transport | Runtime execution |

### Bridges (`bridges/`)

| Module | Library | Dependencies | Purpose |
|--------|---------|-------------|---------|
| base | `libzapata-bridge-base` | base, parser-json | CRTP bridge template |
| lua | `libzapata-bridge-lua` | bridge-base, Lua | Lua scripting |
| prolog | `libzapata-bridge-prolog` | bridge-base, Pl | Prolog scripting |

### Generators (`generators/`)

| Module | Library | Dependencies | Purpose |
|--------|---------|-------------|---------|
| ast | `libzapata-generator-ast` | base | AST code generation |
| rest | `libzapata-generator-rest` | ast | REST API code generator |

## Dependency Graph

```
                        Application
                            │
                    ┌───────┼───────┐
                    ▼       ▼       ▼
              engine-rest  engine-  engine-
                    │      transport runtime
                    │       │       │
                    └───┬───┘       │
                        ▼           │
                    transport ◄─────┘
                    │       │
              ┌─────┤       ├─────┐
              ▼     ▼       ▼     ▼
           net-http net-ws net-tcp net-local ...
              │     │       │     │
              └──┬──┘       └──┬──┘
                 ▼             ▼
            parser-http    io-socket
                 │             │
                 ▼             ▼
            parser-json    io-stream
                 │             │
                 └──────┬──────┘
                        ▼
                       base
```

## Disabled Modules

The following modules exist in the repository but are currently disabled (commented out in CMake) and should not be built or documented:

- `network/smtp` - SMTP email
- `security/oauth2` - OAuth 2.0 authentication
- `storage/redis` - Redis key-value store
- `storage/couchdb` - CouchDB database

## See Also

- [Architecture Overview](overview.md) - High-level design
- [Concurrency Model](concurrency.md) - Threading and lock-free patterns
- [Plugin System](plugins.md) - Plugin loading and lifecycle
