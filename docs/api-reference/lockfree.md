# Lock-Free API Reference

This document provides the API reference for the Zapata lock-free data structures module.

## Headers

```cpp
#include <zapata/lockfree.h>              // Main aggregate header
#include <zapata/lockfree/hazard_ptr.h>   // Hazard pointers only
#include <zapata/lockfree/queue.h>        // Lock-free queue only
```

---

## Namespace: `zpt::lf`

Lock-free data structures and utilities.

---

## Class Template: `zpt::lf::hazard_ptr<T>`

Hazard pointer domain for safe memory reclamation in lock-free structures.

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `T` | Type of pointers being protected |

### Type Aliases

```cpp
using size_type = size_t;
using hp_type = zpt::padded_atomic<T*>;
using thr_slot_type = zpt::padded_atomic<bool>;
using pending_list = std::map<T*, T*>;
```

### Configuration Parameters

| Parameter | Description |
|-----------|-------------|
| P | Maximum number of threads |
| K | Hazard pointers per thread (default: 2) |
| N | Total hazard pointers (P × K) |
| R | Reclamation threshold (N × 2) |

### Constructor

```cpp
hazard_ptr(long _max_threads, long _ptr_per_thread = 2);
```

Creates a hazard pointer domain.

**Parameters:**
- `_max_threads` - Maximum concurrent threads accessing the domain
- `_ptr_per_thread` - Hazard pointer slots per thread (minimum 2)

**Note:** Copy and move constructors are deleted.

### Methods

#### `acquire`

```cpp
auto acquire(T* _ptr) -> long;
```

Acquires a hazard pointer slot and publishes the pointer.

**Parameters:**
- `_ptr` - Pointer to protect

**Returns:** Slot index to use with `release()`

**Throws:** `zpt::ExpectationException` if no slots available

---

#### `release`

```cpp
auto release(long _idx) -> hazard_ptr<T>&;
```

Releases a hazard pointer slot.

**Parameters:**
- `_idx` - Slot index from `acquire()`

---

#### `retire`

```cpp
auto retire(T* _ptr) -> hazard_ptr<T>&;
```

Marks a pointer for deferred deletion. The pointer will be deleted when no hazard pointer references it.

**Parameters:**
- `_ptr` - Pointer to retire

**Note:** Automatically triggers `clean()` when retired list reaches threshold R.

---

#### `clean`

```cpp
auto clean() -> hazard_ptr<T>&;
```

Scans all hazard pointers and deletes retired pointers that are no longer referenced.

---

#### `clear_thread_context`

```cpp
auto clear_thread_context() -> hazard_ptr<T>&;
```

Cleans up thread-local state. **Must be called before thread exit.**

---

#### `get_thread_dangling_count`

```cpp
auto get_thread_dangling_count() -> size_t;
```

Returns the number of retired pointers not yet deleted for the current thread.

---

#### `get_thread_held_count`

```cpp
auto get_thread_held_count() -> size_t;
```

Returns the number of hazard pointer slots currently held by this thread.

---

### Nested Class: `guard`

RAII wrapper for hazard pointer acquisition/release.

#### Constructor

```cpp
guard(T* _target, zpt::lf::hazard_ptr<T>& _parent);
```

Acquires a hazard pointer slot for the target pointer.

#### Destructor

Releases the hazard pointer slot. If `retire()` was called, also retires the target pointer.

#### Methods

| Method | Description |
|--------|-------------|
| `retire() -> guard&` | Marks target for retirement on destruction |
| `target() -> T*` | Returns the protected pointer |

#### Example

```cpp
zpt::lf::hazard_ptr<Node> hp(16);

void safe_access(std::atomic<Node*>& shared) {
    Node* ptr = shared.load();
    zpt::lf::hazard_ptr<Node>::guard guard(ptr, hp);

    // Safe to use guard.target() here
    process(guard.target());

    // If removing the node:
    if (try_remove(shared, ptr)) {
        guard.retire();  // Will be deleted when safe
    }
}
```

---

## Class Template: `zpt::lf::queue<T>`

Lock-free FIFO queue for concurrent producer/consumer patterns.

### Template Parameters

| Parameter | Constraint | Description |
|-----------|------------|-------------|
| `T` | Copy-constructible | Element type |

### Type Aliases

```cpp
using size_type = size_t;
using hazard_domain = zpt::lf::hazard_ptr<zpt::lf::forward_node<T>>;
```

### Constructor

```cpp
queue(long _max_threads);
```

Creates a lock-free queue.

**Parameters:**
- `_max_threads` - Maximum concurrent threads accessing the queue

**Note:** Copy and move constructors are deleted.

### Methods

#### `push`

```cpp
auto push(T value) -> zpt::lf::queue<T>&;
```

Adds an element to the back of the queue. Thread-safe for concurrent calls.

**Parameters:**
- `value` - Value to add (will be copied)

**Returns:** Reference to the queue for chaining

---

#### `pop`

```cpp
auto pop() -> T;
```

Removes and returns the front element. Thread-safe for concurrent calls.

**Returns:** The front element (moved)

**Throws:** `zpt::NoMoreElementsException` if queue is empty

---

#### `front`

```cpp
auto front() const -> T;
```

Returns the front element without removing it.

**Throws:** `zpt::NoMoreElementsException` if queue is empty

---

#### `back`

```cpp
auto back() const -> T;
```

Returns the back element without removing it.

**Throws:** `zpt::NoMoreElementsException` if queue is empty

---

#### `size`

```cpp
auto size() const -> size_t;
```

Returns the approximate element count.

**Note:** May be stale due to concurrent modifications.

---

#### `begin` / `end`

```cpp
auto begin() const -> iterator;
auto end() const -> iterator;
```

Returns iterators for range-based traversal.

**Warning:** Iteration is not thread-safe during concurrent modifications.

---

#### `clear_thread_context`

```cpp
auto clear_thread_context() -> zpt::lf::queue<T>&;
```

Cleans up thread-local hazard pointer state. **Must be called before thread exit.**

---

#### `get_thread_dangling_count`

```cpp
auto get_thread_dangling_count() const -> size_t;
```

Returns count of retired nodes pending deletion for the current thread.

---

### Nested Class: `iterator`

Forward iterator for queue traversal.

```cpp
using difference_type = std::ptrdiff_t;
using value_type = T;
using iterator_category = std::forward_iterator_tag;
```

---

## Class Template: `zpt::lf::forward_node<T>`

Internal node type for lock-free linked structures.

### Members

| Member | Type | Description |
|--------|------|-------------|
| `__value` | `T` | Stored value |
| `__is_null` | `zpt::padded_atomic<bool>` | True if node is sentinel |
| `__next` | `zpt::padded_atomic<forward_node*>` | Next node pointer |

---

## Usage Patterns

### Producer-Consumer Queue

```cpp
#include <zapata/lockfree.h>
#include <thread>
#include <vector>

zpt::lf::queue<int> work_queue(8);  // 8 threads max
std::atomic<bool> done{false};

void producer() {
    for (int i = 0; i < 1000; ++i) {
        work_queue.push(i);
    }
    done = true;
    work_queue.clear_thread_context();
}

void consumer() {
    while (!done || work_queue.size() > 0) {
        try {
            int item = work_queue.pop();
            process(item);
        } catch (zpt::NoMoreElementsException&) {
            std::this_thread::yield();
        }
    }
    work_queue.clear_thread_context();
}
```

### Custom Lock-Free Structure

```cpp
#include <zapata/lockfree/hazard_ptr.h>

template<typename T>
class LockFreeStack {
    struct Node {
        T value;
        std::atomic<Node*> next;
    };

    std::atomic<Node*> head{nullptr};
    zpt::lf::hazard_ptr<Node> hp;

public:
    LockFreeStack(int max_threads) : hp(max_threads, 2) {}

    void push(T value) {
        Node* node = new Node{value, head.load()};
        while (!head.compare_exchange_weak(node->next, node));
    }

    T pop() {
        while (true) {
            Node* old_head = head.load();
            if (!old_head) throw std::runtime_error("empty");

            zpt::lf::hazard_ptr<Node>::guard guard(old_head, hp);
            if (head.compare_exchange_strong(old_head, old_head->next.load())) {
                T value = std::move(old_head->value);
                guard.retire();
                return value;
            }
        }
    }

    void clear_thread() { hp.clear_thread_context(); }
};
```

---

## Thread Safety Notes

1. **Thread Exit**: Always call `clear_thread_context()` before a thread exits to prevent resource leaks.

2. **Maximum Threads**: The maximum thread count is fixed at construction. Exceeding it causes an exception.

3. **Hazard Pointer Slots**: Each thread has K slots (default 2). Acquiring more than K pointers simultaneously will fail.

4. **Memory Reclamation**: Retired pointers are batch-deleted when the retired list reaches threshold R (= 2NK).

5. **Iteration**: Queue iteration is not safe during concurrent `push`/`pop` operations.

---

## See Also

- [Base Utilities](base.md) - `zpt::padded_atomic`
- [Architecture: Concurrency](../architecture/concurrency.md)
