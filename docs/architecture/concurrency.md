# Concurrency Model

How Zapata handles concurrent operations using event-driven I/O and lock-free data structures.

## Threading Model

Zapata uses a multi-threaded event-driven architecture:

- **Main thread** - Configuration loading, plugin initialization, and shutdown coordination
- **I/O threads** - Epoll-based event loops handling socket accept/read/write
- **Worker threads** - Request handler execution and event processing

Threads communicate via lock-free queues, avoiding mutex contention.

## Event-Driven I/O

### Epoll Integration

Network I/O uses Linux `epoll` for efficient multiplexing:

```
┌─────────────────────────────────┐
│         Epoll Event Loop         │
│                                  │
│  ┌─────┐ ┌─────┐ ┌─────┐       │
│  │ fd1 │ │ fd2 │ │ fd3 │ ...   │
│  └──┬──┘ └──┬──┘ └──┬──┘       │
│     └───────┼───────┘           │
│             ▼                    │
│     Event Dispatcher             │
│             │                    │
│     ┌───────┼───────┐           │
│     ▼       ▼       ▼           │
│  Handler Handler Handler        │
└─────────────────────────────────┘
```

The I/O layer uses non-blocking sockets with `EPOLLIN`/`EPOLLOUT` events, dispatching incoming data to protocol parsers which produce messages for the transport layer.

## Lock-Free Data Structures

### Lock-Free Queue

`zpt::lf::queue<T>` provides a bounded, multi-producer, multi-consumer FIFO queue using Dmitry Vyukov's MPMC algorithm:

- **Enqueue** (`push`): Atomically claims a slot via the tail counter; throws `zpt::NoSpaceAvailableException` when the buffer is full.
- **Dequeue** (`pop`): Atomically claims a slot via the head counter; throws `zpt::NoMoreElementsException` when empty.
- **Memory**: A flat array of slots (each containing a value and a sequence token) is allocated once at construction — no per-element allocation at runtime.
- **Sequence tokens**: Each slot has a monotonically increasing sequence number that ensures producers and consumers don't overwrite each other, avoiding ABA issues without hazard pointers.
- **Capacity**: Must be a power of two (enforced via bitmask indexing).
- **No thread limit**: Any number of threads may push or pop concurrently without registration or cleanup.

```cpp
// Size the queue to the expected peak depth (must be power of two)
zpt::lf::queue<zpt::message> channel(4096);

// Producer
try {
    channel.push(make_message());
}
catch (zpt::NoSpaceAvailableException&) { /* queue full */ }

// Consumer
try {
    auto msg = channel.pop();   // returns T by value
    handle(msg);
}
catch (zpt::NoMoreElementsException&) { /* queue empty */ }
```

Used for inter-thread message passing between I/O and worker threads.

### Padded Atomics

`zpt::padded_atomic<T>` aligns atomic variables to cache line boundaries (typically 64 bytes), preventing false sharing between cores:

```cpp
// Without padding: two atomics on same cache line cause contention
std::atomic<int> a, b;  // May share cache line

// With padding: each atomic gets its own cache line
zpt::padded_atomic<int> a, b;  // Guaranteed separate cache lines
```

## Spin Mutex

For short critical sections where contention is low, `zpt::spin_mutex` provides a lightweight alternative to `std::mutex`:

- Uses atomic test-and-set (no kernel involvement)
- Spins in userspace waiting for the lock
- Suitable for very short hold times (nanoseconds)
- Not suitable for long or contended critical sections

## Thread Safety Guidelines

### JSON Values

`zpt::json` uses shared pointers internally. Reference counting is thread-safe, but concurrent mutations to the same value are not:

```cpp
// Safe: read-only access from multiple threads
auto config = load_config();  // Shared across threads
auto value = config("key");   // OK: read-only

// Unsafe: concurrent modification
// Use clone() or external synchronization
auto copy = config->clone();  // Independent copy for mutation
```

### Event Dispatcher

The event dispatcher is thread-safe for:
- Subscribing handlers (from any thread)
- Publishing events (from any thread)

Handler execution is serialized per-event but concurrent across different events.

## See Also

- [Architecture Overview](overview.md) - High-level design
- [Lock-Free API Reference](../api-reference/lockfree.md) - Bounded queue API
- [Component Architecture](components.md) - Module relationships
