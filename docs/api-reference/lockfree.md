# Lock-Free API Reference

This document provides the API reference for the Zapata lock-free data structures module.

## Headers

```cpp
#include <zapata/lockfree.h>         // Main aggregate header
#include <zapata/lockfree/queue.h>   // Lock-free queue only
```

---

## Namespace: `zpt::lf`

Lock-free data structures and utilities.

---

## Constants

```cpp
constexpr __uint128_t zpt::lf::UNMASK;
constexpr __uint128_t zpt::lf::MASK;
```

Internal bitmasks used by `zpt::lf::queue` to pack and guard head/tail indices inside a single 128-bit atomic. `UNMASK` isolates the lower 126 bits (the actual index values); `MASK` has bits 126–127 set and is used as a mutation guard during CAS operations.

---

## Class Template: `zpt::lf::queue<T>`

Bounded lock-free FIFO queue for concurrent producer/consumer patterns.

Backed by a fixed-capacity ring buffer allocated at construction time. Head and tail positions are packed into a single 128-bit atomic value; the two most-significant bits serve as a mutation guard so that only one CAS winner at a time may advance an index. No per-thread state or cleanup is required.

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `T` | Element type stored in the queue |

### Type Aliases

```cpp
using size_type = size_t;
using ptr       = std::unique_ptr<T>;
using const_ptr = std::shared_ptr<T const>;
```

### Constructor

```cpp
queue(size_t _max_queue_size);
```

Creates a bounded queue.

**Parameters:**
- `_max_queue_size` — Maximum number of elements the queue can hold simultaneously.

**Note:** Copy and move constructors are deleted — the ring buffer and atomic state cannot be shared or transferred.

### Methods

#### `push` (by value)

```cpp
auto push(T value) -> zpt::lf::queue<T>&;
```

Copies the value into a new heap-allocated node and enqueues it.

**Parameters:**
- `value` — Value to copy into the queue.

**Returns:** Reference to the queue for chaining.

**Note:** Spins with `std::this_thread::yield()` until a slot becomes available when the queue is at capacity.

---

#### `push` (by unique_ptr)

```cpp
auto push(ptr&& value) -> zpt::lf::queue<T>&;
```

Transfers ownership of an already-allocated node into the queue.

**Parameters:**
- `value` — Owning pointer to the element. Ownership is transferred to the queue.

**Returns:** Reference to the queue for chaining.

**Note:** Spins with `std::this_thread::yield()` until a slot becomes available when the queue is at capacity.

---

#### `pop`

```cpp
auto pop() -> ptr;
```

Removes and returns the front element.

**Returns:** `std::unique_ptr<T>` owning the dequeued element.

**Throws:** `zpt::NoMoreElementsException` if the queue is empty.

---

#### `size`

```cpp
auto size() const -> size_t;
```

Returns the current element count.

**Note:** May be transiently stale because the size counter is updated separately from the index CAS.

---

#### `to_string`

```cpp
auto to_string() const -> std::string;
```

Returns a debug string listing the queue's current contents.

---

#### `operator std::string`

```cpp
operator std::string();
```

Implicit conversion to string; delegates to `to_string()`.

---

#### `operator<<`

```cpp
friend auto operator<<(std::ostream& _out, zpt::lf::queue<T>& _in) -> std::ostream&;
```

Streams a human-readable representation of the queue to `_out`.

---

## Usage Patterns

### Producer-Consumer Queue

```cpp
#include <zapata/lockfree.h>
#include <thread>

// Capacity of 1000 elements; no thread-count limit
zpt::lf::queue<int> work_queue(1000);
std::atomic<bool> done{ false };

void producer() {
    for (int i = 0; i < 500; ++i) {
        work_queue.push(i);
    }
    done = true;
}

void consumer() {
    while (!done || work_queue.size() > 0) {
        try {
            auto item = work_queue.pop();
            process(*item);
        }
        catch (zpt::NoMoreElementsException&) {
            std::this_thread::yield();
        }
    }
}
```

### Passing Ownership Without Copying

```cpp
zpt::lf::queue<std::vector<char>> buffer_queue(256);

// Producer: build and enqueue without an extra copy
auto buf = zpt::allocate_unique<std::vector<char>>(4096);
fill_buffer(*buf);
buffer_queue.push(std::move(buf));

// Consumer: take ownership directly
auto received = buffer_queue.pop();
process(*received);
```

---

## Thread Safety Notes

1. **No thread registration** — Any thread may call `push()` or `pop()` without prior registration.

2. **Bounded capacity** — `push()` spins when the queue is full. Size the queue to the peak concurrent load to avoid starvation.

3. **Approximate size** — `size()` is updated outside the CAS critical section and may be transiently stale; use it for monitoring, not synchronisation.

4. **No iteration** — The queue does not provide iterators; elements must be consumed via `pop()`.

---

## See Also

- [Base Utilities](base.md) - `zpt::padded_atomic`
- [Architecture: Concurrency](../architecture/concurrency.md)
