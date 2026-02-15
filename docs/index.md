# Zapata Framework Documentation

Zapata is a RESTful API development framework for C++20. It provides a complete ecosystem for building high-performance HTTP services with asynchronous programming, promises, and a modular plugin architecture.

## Key Features

- **RESTful API Development** - Build REST endpoints with minimal boilerplate
- **Async/Await Programming** - Promise-based APIs with C++20 coroutine support
- **JSON Support** - Native JSON parsing and serialization
- **Database Connectivity** - SQLite and MySQL with abstract connector interface
- **Multiple Transports** - HTTP, WebSocket, TCP, Unix sockets, named pipes
- **Scripting Integration** - Lua and Prolog bridges for extensibility
- **Lock-Free Concurrency** - Hazard pointer-based thread-safe data structures
- **Plugin Architecture** - Extensible protocol and storage backends

## Documentation

### Getting Started

- [Installation](getting-started/installation.md) - Build requirements and setup
- [Quickstart](getting-started/quickstart.md) - Build your first REST API
- [Project Structure](getting-started/project-structure.md) - How to organize a Zapata project

### Guides

- [Building REST APIs](guides/rest-api.md) - Endpoints, routing, and request handling
- [Configuration](guides/configuration.md) - Configuration options reference
- [Working with JSON](guides/json.md) - Parsing, serialization, and manipulation
- [Events](guides/events.md) - Pub/sub event system

**Database Integration**
- [Database Overview](guides/database/overview.md) - Storage abstraction concepts
- [SQLite](guides/database/sqlite.md) - SQLite integration
- [MySQL](guides/database/mysql.md) - MySQL integration

**Networking**
- [Transports](guides/networking/transports.md) - Transport layer concepts
- [HTTP](guides/networking/http.md) - HTTP protocol usage
- [WebSocket](guides/networking/websocket.md) - Real-time communication

**Scripting**
- [Lua Integration](guides/scripting/lua.md) - Embedding Lua scripts

### Architecture

- [Overview](architecture/overview.md) - High-level architecture
- [Components](architecture/components.md) - Module relationships
- [Concurrency Model](architecture/concurrency.md) - Async patterns and lock-free structures
- [Plugin System](architecture/plugins.md) - Extensibility architecture

### API Reference

- [Base Utilities](api-reference/base.md) - Core utilities, exceptions, crypto
- [JSON](api-reference/json.md) - JSON types and operations
- [URI](api-reference/uri.md) - URI parsing and serialization
- [HTTP](api-reference/http.md) - HTTP message parsing
- [Events](api-reference/events.md) - Event dispatcher and resolver
- [Ontology](api-reference/ontology.md) - Messages and performatives
- [Transport](api-reference/transport.md) - Transport interfaces
- [Storage](api-reference/storage.md) - Database connector API
- [REST Engine](api-reference/rest.md) - REST API engine
- [Lock-Free](api-reference/lockfree.md) - Concurrent data structures
- [I/O](api-reference/io.md) - Stream and socket abstractions
- [Bridges & Generators](api-reference/bridges-generators.md) - Language bridges and code generation

### Extending Zapata

- [Custom Transports](extending/custom-transport.md) - Writing transport plugins
- [Custom Storage](extending/custom-storage.md) - Writing database connectors
- [Language Bridges](extending/bridges.md) - Creating scripting bridges

### Examples

- [Hello World](examples/hello-world/) - Minimal REST API
- [CRUD API](examples/crud-api/) - Full CRUD with SQLite
- [WebSocket Chat](examples/websocket-chat/) - Real-time WebSocket example

## Requirements

- C++20 compatible compiler (GCC 10+, Clang 12+)
- CMake 3.18+
- OpenSSL
- SQLite3 (optional)
- MySQL client library (optional)
- Lua 5.4 (optional)

## License

Zapata is released into the public domain under the [Unlicense](https://unlicense.org/).
