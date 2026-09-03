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

## Class Template: `zpt::lf::queue<T>`

Bounded MPMC (multi-producer, multi-consumer) lock-free FIFO queue based on [Dmitry Vyukov's algorithm](http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue).

Backed by a fixed-capacity ring buffer allocated at construction time. Each slot carries a `sequence` token that acts as an ownership gate — a CAS on the shared `head_`/`tail_` counters is the ownership transfer (it prevents two threads from operating on the same slot), and the sequence check is a guard that tells us whether the slot is ready or not.

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `T` | Element type stored in the queue (must be trivially copyable) |

### Type Aliases

```cpp
using size_type = size_t;
using ptr       = zpt::allocator<T>::unique_pointer;
```

### Constructor

```cpp
explicit queue(size_t _max_queue_size);
```

Creates a bounded queue.

**Parameters:**
- `_max_queue_size` — Maximum number of elements the queue can hold simultaneously. **Must be a power of two.**

**Note:** Copy and move constructors (and assignment operators) are deleted — the ring buffer and atomic state cannot be shared or transferred.

### Methods

#### `push` (by value)

```cpp
auto push(T _value) -> zpt::lf::queue<T>&;
```

Copies the value into a new heap-allocated node and enqueues it.

**Parameters:**
- `_value` — Value to copy into the queue.

**Returns:** Reference to the queue for chaining.

**Throws:** `zpt::NoSpaceAvailableException` if the queue is full.

---

#### `push` (by unique_ptr)

```cpp
auto push(ptr&& _value) -> zpt::lf::queue<T>&;
```

Transfers ownership of an already-allocated node into the queue.

**Parameters:**
- `_value` — Owning pointer to the element. Ownership is transferred to the queue.

**Returns:** Reference to the queue for chaining.

**Throws:** `zpt::NoSpaceAvailableException` if the queue is full.

---

#### `pop`

```cpp
auto pop() -> ptr;
```

Removes and returns the front element.

**Returns:** `std::unique_ptr<T>` owning the dequeued element.

**Throws:** `zpt::NoMoreElementsException` if the queue is empty.

---

#### `capacity`

```cpp
auto capacity() const -> size_t;
```

Returns the maximum number of elements the queue can hold.

**Returns:** Queue capacity (the value passed to the constructor).

---

#### `size`

```cpp
auto size() const -> size_t;
```

Returns the approximate current element count.

**Note:** The size counter is updated independently of the slot CAS and may be transiently stale. Use it for monitoring, not synchronisation.

---

#### `shutdown`

```cpp
auto shutdown() -> zpt::lf::queue<T>&;
```

Sets the internal shutdown flag, causing all threads spinning in `push()` or `pop()` to throw `NoMoreElementsException` and exit.

**Returns:** Reference to the queue for chaining.

---

#### `to_string`

```cpp
auto to_string() const -> std::string;
```

Returns a debug string listing the queue's current contents.

---

#### `operator std::string`

```cpp
operator std::string() const;
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

// Capacity of 1000 elements
zpt::lf::queue<int> work_queue(1000);
std::atomic<bool> done{ false };

void producer() {
    for (int i = 0; i < 500; ++i) {
        try {
            work_queue.push(i);
        } catch (zpt::NoSpaceAvailableException const&) {
            // Queue is full — either back-pressure, retry with sleep, or log
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            --i;  // Retry this iteration
        }
    }
    done = true;
}

void consumer() {
    while (!done.load() || work_queue.size() > 0) {
        try {
            auto item = work_queue.pop();
            process(*item);
        } catch (zpt::NoMoreElementsException const&) {
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

2. **Bounded capacity** — `push()` throws `NoSpaceAvailableException` when the queue is full. Size the queue to the peak concurrent load to avoid exceptions.

3. **Approximate size** — `size()` is updated outside the CAS critical section and may be transiently stale; use it for monitoring, not synchronisation.

4. **No iteration** — The queue does not provide iterators; elements must be consumed via `pop()`.

5. **Shutdown** — Call `shutdown()` to wake threads blocked in `push()`/`pop()`. They will throw `NoMoreElementsException`.

6. **Power-of-two capacity** — The queue capacity must be a power of two. This is enforced at construction via the bitmask used for index wrapping (`index & mask`).

---

## See Also

- [Base Utilities](base.md) - `zpt::padded_atomic`
- [Architecture: Concurrency](../architecture/concurrency.md)
