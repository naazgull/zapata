# Architecture Overview

Zapata is a modular, plugin-based RESTful API framework for C++20. This document describes its high-level architecture and design principles.

## Design Principles

1. **Modularity** - Components are loosely coupled and can be loaded/unloaded at runtime
2. **Asynchronous by Default** - Event-driven architecture with non-blocking I/O
3. **Lock-Free Where Possible** - Concurrent data structures avoid traditional locks
4. **Transport Agnostic** - Same API works over HTTP, WebSocket, TCP, etc.
5. **Database Agnostic** - Unified storage interface for multiple backends

## Layer Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                        │
│              (Your REST endpoints and business logic)        │
├─────────────────────────────────────────────────────────────┤
│                      Engine Layer                            │
│         ┌─────────────┬─────────────┬─────────────┐         │
│         │ REST Engine │  Transport  │   Events    │         │
│         │             │   Engine    │   Engine    │         │
│         └─────────────┴─────────────┴─────────────┘         │
├─────────────────────────────────────────────────────────────┤
│                     Protocol Layer                           │
│    ┌──────┬───────────┬─────┬───────┬────────┬──────┐      │
│    │ HTTP │ WebSocket │ TCP │ Local │  Pipe  │ UPnP │      │
│    └──────┴───────────┴─────┴───────┴────────┴──────┘      │
├─────────────────────────────────────────────────────────────┤
│                      I/O Layer                               │
│         ┌─────────────┬─────────────┬─────────────┐         │
│         │   Stream    │   Socket    │    Pipe     │         │
│         │  (systemd)  │  (SSL/TLS)  │   (IPC)     │         │
│         └─────────────┴─────────────┴─────────────┘         │
├─────────────────────────────────────────────────────────────┤
│                     Storage Layer                            │
│              ┌──────────┬──────────┐                        │
│              │  SQLite  │  MySQL   │                        │
│              └──────────┴──────────┘                        │
├─────────────────────────────────────────────────────────────┤
│                    Foundation Layer                          │
│   ┌────────┬────────┬────────┬─────────┬────────┬───────┐  │
│   │  Base  │ Events │ Ontol- │ Lock-   │ JSON   │ Globals│  │
│   │ Utils  │        │  ogy   │  free   │ Parser │       │  │
│   └────────┴────────┴────────┴─────────┴────────┴───────┘  │
└─────────────────────────────────────────────────────────────┘
```

## Component Overview

### Foundation Layer

**zapata-base**
Core utilities used throughout the framework:
- Exception hierarchy
- Cryptographic functions (SHA-1/256/512)
- Text encoding (Base64, UTF-8, URL encoding)
- Logging system
- Synchronization primitives (spin mutex)

**zapata-parser-json**
Complete JSON parser with lexer/tokenizer:
- Streaming parser for large documents
- Lambda function support in JSON
- Type-safe value access

**zapata-events**
Event dispatcher for pub/sub messaging:
- Pattern-based event routing
- Synchronous and asynchronous delivery
- System event broadcasting

**zapata-ontology**
Message semantics and conversation tracking:
- Performative types (request, reply, inform, etc.)
- Conversation ID propagation
- Message envelope structure

**zapata-lockfree**
Wait-free concurrent data structures:
- Hazard pointer-based memory reclamation
- Lock-free FIFO queue
- Cache-line aligned atomics

### I/O Layer

**zapata-io-stream**
Abstract stream interfaces with systemd journal integration.

**zapata-io-socket**
Socket operations with SSL/TLS support via OpenSSL.

**zapata-io-pipe**
Named pipe support for IPC.

### Protocol Layer

Each protocol implements the transport interface:

| Protocol | Library | Description |
|----------|---------|-------------|
| HTTP | `zapata-net-http` | HTTP/1.1 protocol |
| WebSocket | `zapata-net-websocket` | RFC 6455 WebSockets |
| TCP | `zapata-net-tcp` | Raw TCP sockets |
| Local | `zapata-net-local` | Unix domain sockets |
| Pipe | `zapata-net-pipe` | Named pipes |
| Self | `zapata-net-self` | In-process callbacks |

### Storage Layer

**zapata-storage-connector**
Abstract database interface defining:
- Connection management
- Query execution
- Result iteration
- Transaction support

Implementations:
- `zapata-storage-sqlite` - SQLite3 via C API
- `zapata-storage-mysqlx` - MySQL via C API

### Engine Layer

**zapata-engine-startup**
Configuration loading and service bootstrap.

**zapata-engine-rest**
REST API engine:
- Route registration and matching
- Request/response handling
- Middleware support

**zapata-engine-transport**
Transport management:
- Protocol plugin loading
- Connection lifecycle
- Message routing

**zapata-engine-events**
Event processing coordination:
- Event loop management
- Dispatcher orchestration

## Data Flow

### Incoming Request

```
1. Network socket receives data
2. Protocol parser (HTTP/WS) decodes message
3. Transport layer creates request envelope
4. REST engine matches route
5. Handler lambda executes
6. Response flows back through layers
```

### Event Publication

```
1. Component publishes event
2. Dispatcher matches subscribers
3. Each subscriber receives copy
4. Handlers execute (sync or async)
```

## Threading Model

Zapata uses an event-driven model with:

- **Main thread**: Configuration, startup, shutdown
- **I/O threads**: Socket accept/read/write
- **Worker threads**: Request handlers, event processing

Lock-free queues enable safe communication between threads without traditional mutexes.

## Plugin Architecture

Protocols and storage backends are loaded as shared libraries:

```cpp
// Protocol plugins implement transport interface
class my_transport : public zpt::transport {
    // ...
};

// Storage plugins implement connector interface
class my_connector : public zpt::storage::connector {
    // ...
};
```

Plugins are discovered and loaded at startup based on configuration.

## Memory Management

- **Automatic**: Shared pointers for most objects
- **Pool Allocator**: Optional bounded memory pools
- **Hazard Pointers**: Safe reclamation in lock-free structures

## Configuration

Configuration is JSON-based:

```json
{
    "transport": {
        "bind": "tcp://0.0.0.0:8080"
    },
    "storage": {
        "sqlite": {
            "path": "/var/lib/myapp/data.db"
        }
    },
    "log": {
        "level": "info"
    }
}
```

## See Also

- [Components](components.md) - Detailed component relationships
- [Concurrency](concurrency.md) - Threading and lock-free patterns
- [Plugins](plugins.md) - Plugin development guide
