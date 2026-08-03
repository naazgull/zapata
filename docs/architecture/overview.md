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
Lock-free concurrent data structures:
- Bounded FIFO queue backed by a ring buffer with 128-bit atomic mutation guard (CAS-based, no hazard pointers)
- Lock-free FIFO queue
- Cache-line aligned atomics (`padded_atomic`)

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
- **Atomic CAS**: Lock-free queues use 128-bit compare-and-swap with mutation guard for safe concurrent access

## Configuration

Configuration is JSON-based, with top-level sections for identity, logging, plugin loading, and transport settings:

```json
{
    "identity": {
        "id": "service-uuid",
        "name": "my-service"
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

## See Also

- [Components](components.md) - Detailed component relationships
- [Concurrency](concurrency.md) - Threading and lock-free patterns
- [Plugins](plugins.md) - Plugin development guide
