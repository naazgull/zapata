![zapata](https://media.zgul.me/logo_zapata_color_white_1000x600.png)
================================

# Zapata

A RESTful API development framework for C++20.

Zapata provides a complete ecosystem for building high-performance HTTP services with asynchronous programming, lambda functions, and promises. It features a modular plugin architecture with built-in support for multiple databases, authentication mechanisms, and network protocols.

## Features

- **REST API Development** - Build RESTful endpoints with minimal boilerplate
- **Modern C++20** - Lambda functions, concepts, async/await patterns
- **JSON Support** - Native JSON parsing and serialization
- **Database Connectivity** - SQLite, MySQL with abstract connector interface
- **Multiple Transports** - HTTP, WebSocket, TCP, Unix sockets, UPnP
- **Scripting Bridges** - Lua and Prolog integration
- **Lock-Free Concurrency** - Thread-safe data structures using hazard pointers
- **Plugin Architecture** - Extensible protocols and storage backends

## Requirements

- C++20 compiler (GCC 10+, Clang 12+)
- CMake 3.18+
- OpenSSL

Optional dependencies:
- SQLite3
- MySQL client library
- Lua 5.4
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

## Quick Example

```cpp
#include <zapata/rest.h>

int main(int argc, char* argv[]) {
    // Initialize the framework
    zpt::json _config = zpt::load_config(argc, argv);

    // Create REST engine
    auto _engine = zpt::make_engine(_config);

    // Register endpoint
    _engine->on("GET", "/hello/{name}",
        [](zpt::performative _performative,
           zpt::json _envelope,
           zpt::rest::engine::ptr _engine) -> zpt::json {
            auto _name = _envelope["params"]["name"];
            return { "message", std::string("Hello, ") + _name->string() };
        });

    // Start server
    _engine->start();
    return 0;
}
```

## Documentation

Full documentation is available in the [docs/](docs/index.md) directory:

- [Getting Started](docs/getting-started/installation.md) - Installation and quickstart
- [Guides](docs/guides/rest-api.md) - In-depth usage guides
- [Architecture](docs/architecture/overview.md) - System design and concepts
- [API Reference](docs/api-reference/base.md) - Detailed API documentation
- [Examples](docs/examples/) - Working code examples

## Project Structure

```
zapata/
├── bridges/          # Language integration (Lua, Prolog)
├── common/           # Core utilities, events, lock-free structures
├── engines/          # REST, transport, and event engines
├── generators/       # Code generation tools
├── io/               # Stream, socket, pipe abstractions
├── network/          # Protocol implementations (HTTP, WebSocket, TCP)
├── parsers/          # JSON, HTTP, URI parsers
├── storage/          # Database connectors (SQLite, MySQL)
└── docs/             # Documentation
```

## License

This is free and unencumbered software released into the public domain.

See [UNLICENSE](https://unlicense.org/) for details.
