![zapata](https://media.zgul.me/logo_zapata_color_white_1000x600.png)
================================

# Zapata

A RESTful API development framework for C++20.

Zapata provides a complete ecosystem for building high-performance HTTP services with an event-driven architecture and a modular plugin system. It features built-in support for multiple database backends, authentication mechanisms, network protocols, and scripting bridges.

## Features

- **REST API Development** - Build RESTful endpoints using typed handler classes that extend `zpt::events::process`
- **Modern C++20** - Concepts, ranges, coroutines-ready design (callback-based event dispatch, no runtime coroutine support)
- **JSON Support** - Native JSON parsing and serialization via the built-in JSON parser
- **Database Connectivity** - SQLite, MySQL, PostgreSQL, MongoDB with a unified abstract connector interface
- **Multiple Transports** - HTTP, WebSocket, TCP, UPnP, Unix pipes, local, AMQP, MQTT, identity-aware
- **Scripting Bridges** - Lua and Prolog integration for dynamic logic
- **Lock-Free Concurrency** - Thread-safe data structures using a 128-bit atomic mutation guard (CAS-based)
- **Plugin Architecture** - Extensible protocols and storage backends with load-time dependency resolution
- **Ontology & Graph** - Graph processing and ontology management utilities

## Requirements

- C++20 compiler (GCC 10+, Clang 12+)
- CMake 3.18+
- OpenSSL

Optional dependencies:
- SQLite3
- MySQL client library
- PostgreSQL client library
- MongoDB C++ driver
- Lua
- libmagic

## Building

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Debug` | Build type (Debug, Release) |
| `WITH_ASAN` | `OFF` | Enable Address Sanitizer |
| `WITH_TSAN` | `OFF` | Enable Thread Sanitizer |
| `WITH_UBSAN` | `OFF` | Enable UndefinedBehavior Sanitizer |
| `WITH_EXCEPTION_PROPAGATION` | `OFF` | Propagate exceptions instead of catching them |
| `WITH_ALLOCATOR_DEBUGGING` | `OFF` | Keep allocation list for debugging |

## Quick Example

A minimal Zapata handler is a class that extends `zpt::events::process` and implements the `Operation` concept:

```cpp
#include <zapata/rest.h>
#include <zapata/startup.h>

class hello_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;

    auto blocked() const -> bool { return false; }

    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto _name = this->received()->params()["name"];
        this->to_send()
            ->status(200)
            .body() = { "message", "Hello, " + _name->string() };
        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    auto _resolver = zpt::REST_RESOLVER();
    _resolver->add<hello_handler>("/hello/{name}");
}
```

Register this handler class with the REST resolver — the framework handles dispatch, threading, and lifecycle automatically.

## Documentation

Full documentation is available in the [docs/](docs/index.md) directory:

- [Getting Started](docs/getting-started/quickstart.md) - Installation and quickstart
- [Guides](docs/guides/rest-api.md) - In-depth usage guides
- [Architecture](docs/architecture/overview.md) - System design and concepts
- [API Reference](docs/api-reference/rest.md) - Detailed API documentation

## Project Structure

```
zapata/
├── bridges/          # Language integration (Lua, Prolog)
├── common/           # Core utilities, events, lock-free structures, ontology
├── engines/          # REST, transport, runtime, and startup engines
├── generators/       # Code generation tools (AST, REST)
├── io/               # Stream, socket, pipe abstractions
├── network/          # Protocol implementations (HTTP, WebSocket, TCP, AMQP, MQTT, etc.)
├── parsers/          # JSON, HTTP, URI, functional parsers
├── security/         # Security utilities (OAuth2, Redis - disabled)
├── storage/          # Database connectors (SQLite, MySQL, PostgreSQL, MongoDB)
└── docs/             # Documentation
```

## License

This is free and unencumbered software released into the public domain.

See [UNLICENSE](https://unlicense.org/) for details.
