/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file queue.h
 * @brief Bounded lock-free FIFO queue implementation.
 *
 * Provides a thread-safe, bounded MPMC queue based on Dmitry Vyukov's
 * algorithm.  Every slot carries a `sequence` token that acts as an
 * ownership gate — a CAS on the shared head_/tail_ counter is the
 * **ownership transfer** (it prevents two threads from operating on the
 * same slot), and the sequence check is a guard that tells us whether
 * the slot is ready.
 *
 * @par Thread Safety
 * - `push()`: Multiple threads can push concurrently; throws
 *   `NoSpaceAvailableException` when the queue is full.
 * - `pop()`: Multiple threads can pop concurrently; throws
 *   `NoMoreElementsException` when the queue is empty.
 * - `size()`: Approximate count (may be transiently stale).
 */

#pragma once

#include <zapata/allocator.h>
#include <zapata/atomics/padded_atomic.h>
#include <zapata/base/sentry.h>
#include <zapata/exceptions/exceptions.h>

namespace zpt {
namespace lf {

/**
 * @brief Bounded MPMC lock-free FIFO queue (Vyukov algorithm).
 *
 * Backed by a fixed-capacity ring buffer allocated at construction time.
 * Each slot carries a `sequence` token; producers and consumers share
 * two counters (`head_` and `tail_`).  A thread claims a slot index via
 * a CAS on one of those counters — only the CAS winner gets to read or
 * write the slot.  The sequence token is then advanced to hand the slot
 * to the other side.
 *
 * @tparam T Value type stored in the queue (must be trivially copyable).
 *
 * @par Example
 * @code
 * // Create a queue with capacity for up to 1000 elements
 * zpt::lf::queue<std::string> q(1000);
 *
 * // Producer thread
 * q.push("message 1");
 * q.push("message 2");
 *
 * // Consumer thread
 * try {
 *     while (true) {
 *         auto msg = q.pop();
 *         process(*msg);
 *     }
 * } catch (zpt::NoMoreElementsException&) {
 *     // Queue is empty
 * }
 * @endcode
 *
 * @note `push()` throws `NoSpaceAvailableException` when the queue is full.
 * @note `size()` may be transiently stale because the size counter is
 *       updated independently of the slot CAS.
 */
template<typename T>
class queue {
  public:
    using size_type = size_t;
    using ptr = zpt::allocator<T>::unique_pointer;

    /**
     * @brief Constructs a bounded queue with the given fixed capacity.
     * @param _max_queue_size Maximum number of elements the queue can
     *   hold simultaneously (must be a power of two).
     */
    explicit queue(size_t _max_queue_size);
    /**
     * @brief Copy constructor (deleted).
     * @return void (queue cannot be copied).
     *
     * The ring buffer cannot be shared between instances.
     */
    queue(zpt::lf::queue<T> const& _rhs) = delete;
    /**
     * @brief Move constructor (deleted).
     * @return void (queue cannot be moved).
     *
     * The atomic state cannot be transferred safely.
     */
    queue(zpt::lf::queue<T>&& _rhs) = delete;
    /** @brief Destructor. */
    ~queue() = default;

    /**
     * @brief Copy assignment (deleted).
     * @return void (queue cannot be copied).
     *
     * The ring buffer cannot be shared between instances.
     */
    auto operator=(zpt::lf::queue<T> const& _rhs) -> zpt::lf::queue<T>& = delete;
    /**
     * @brief Move assignment (deleted).
     * @return void (queue cannot be moved).
     *
     * The atomic state cannot be transferred safely.
     */
    auto operator=(zpt::lf::queue<T>&& _rhs) -> zpt::lf::queue<T>& = delete;

    /**
     * @brief Adds an element to the back of the queue.
     * @param _value Value to add (will be copied).
     * @return Reference to this queue.
     * @throws zpt::NoSpaceAvailableException If queue is full.
     */
    auto push(T _value) -> zpt::lf::queue<T>&;
    /**
     * @brief Adds an element to the back of the queue, transferring ownership.
     * @param _value Unique pointer whose ownership is transferred to the queue.
     * @return Reference to this queue.
     * @throws zpt::NoSpaceAvailableException If queue is full.
     */
    auto push(ptr&& _value) -> zpt::lf::queue<T>&;
    /**
     * @brief Removes and returns the front element.
     * @return Unique pointer owning the dequeued element.
     * @throws zpt::NoMoreElementsException If queue is empty.
     */
    auto pop() -> ptr;

    /**
     * @brief Returns the queue maximum number of elements.
     * @return Maximum capacity of the queue.
     */
    auto capacity() const -> size_t;
    /**
     * @brief Returns approximate element count.
     * @return Current number of elements in the queue.
     */
    auto size() const -> size_t;

    /**
     * @brief Returns a debug string representation of the queue.
     * @return String with queue state.
     */
    __attribute__((noinline)) auto to_string() const -> std::string;
    /**
     * @brief Converts to string (calls to_string()).
     * @return String representation of the queue.
     */
    operator std::string() const;

    friend auto operator<<(std::ostream& _out, zpt::lf::queue<T>& _in) -> std::ostream& {
        _out << "queue(" << std::hex << &_in << "):" << std::dec << "\n  #items ->\n     [ ";
        try {
            size_t _count{ 0 };
            auto _lower = _in.__head->load();
            auto _upper = _in.__tail->load();
            for (size_t _idx = _lower; _idx != _upper; ++_idx, ++_count) {
                _out << (_count == 0 ? "" : (_count % 5 == 0 ? "\n       " : ", "))
                     << *_in.__slots[_idx % _in.__capacity].value;
            }
        }
        catch (zpt::NoMoreElementsException const& e) {
        }
        _out << (_in.size() != 0 ? " " : "") << "]\n"
             << "   (" << _in.size() << " elements) ";
        return _out;
    }

  private:
    /**
     * @brief One cache-line-aligned slot in the ring buffer.
     *
     * The `sequence` field is the ownership token.  Valid values are:
     *   - `slot_index`        → slot is free (producers may write)
     *   - `slot_index + C`    → slot is full (consumers may read)
     *   where C is the queue capacity.
     */
    struct alignas(64) slot_t {
        zpt::padded_atomic<std::uint64_t> sequence{ 0 };
        ptr value;
    };

    /** @brief Fixed-size ring buffer of slots allocated at construction. */
    zpt::allocator<slot_t>::array_pointer __slots{ nullptr };
    /** @brief Head index: producers claim the next slot via CAS on head_. */
    zpt::padded_atomic<std::uint64_t> __head{ 0 };
    /** @brief Tail index: consumers claim the next slot via CAS on tail_. */
    zpt::padded_atomic<std::uint64_t> __tail{ 0 };
    /** @brief Approximate element count, updated independently of the slot CAS. */
    zpt::padded_atomic<std::uint64_t> __size{ 0 };
    /** @brief Maximum number of elements the queue can hold (power of two). */
    size_t __capacity{ 0 };
    /** @brief Bit mask for index wrapping: `__capacity - 1`. */
    size_t __mask{ 0 };
};
} // namespace lf
} // namespace zpt

template<typename T>
zpt::lf::queue<T>::queue(size_t _max_queue_size)
  : __slots{ zpt::allocate_array<slot_t>(_max_queue_size) }
  , __capacity{ _max_queue_size }
  , __mask{ _max_queue_size - 1 } {
    // Initialise every slot: slot[i].sequence == i at start so that
    // the first producer (claiming index 0) sees sequence[0] == 0.
    for (size_t _i = 0; _i < _max_queue_size; ++_i) {
        this->__slots[_i].sequence->store(_i, std::memory_order_relaxed);
    }
}

template<typename T>
auto zpt::lf::queue<T>::push(T _value) -> zpt::lf::queue<T>& {
    return push(zpt::allocate_unique<T>(_value));
}

template<typename T>
auto zpt::lf::queue<T>::push(ptr&& _value) -> zpt::lf::queue<T>& {
    for (;;) {
        auto _head = this->__head->load(std::memory_order_relaxed);

        // Claim a slot by advancing head_ via CAS.  Only the winner
        // gets ownership of this slot index.
        auto _new_head = _head + 1;
        if (this->__head->compare_exchange_strong(_head, _new_head, std::memory_order_relaxed)) {
            size_t _slot_idx = _head & this->__mask;

            // Spin-wait until the slot is free (sequence == slot_index).
            while (this->__slots[_slot_idx].sequence->load(std::memory_order_acquire) !=
                   _slot_idx) {
                std::this_thread::yield();
            }

            // Write the value and release the slot to consumers.
            this->__slots[_slot_idx].value = std::move(_value);
            this->__slots[_slot_idx].sequence->store(_slot_idx + this->__capacity,
                                                     std::memory_order_release);

            // Update the approximate size counter.
            this->__size->fetch_add(1);
            return (*this);
        }

        // CAS failed — another producer claimed this slot first.
        // _head was updated by CAS; loop and retry.
    }
}

template<typename T>
auto zpt::lf::queue<T>::pop() -> ptr {
    for (;;) {
        if (this->__tail->load() == this->__head->load()) {
            throw zpt::NoMoreElementsException("No elements in the queue");
        }

        auto _tail = this->__tail->load(std::memory_order_relaxed);

        // Claim a slot by advancing tail_ via CAS.  Only the winner
        // gets ownership of this slot index.
        auto _new_tail = _tail + 1;
        if (this->__tail->compare_exchange_strong(_tail, _new_tail, std::memory_order_relaxed)) {
            size_t _slot_idx = _tail & this->__mask;

            // Spin-wait until the slot is full (sequence == slot_index + capacity).
            while (this->__slots[_slot_idx].sequence->load(std::memory_order_acquire) !=
                   (_slot_idx + this->__capacity)) {
                std::this_thread::yield();
            }

            // Read the value and release the slot to producers.
            ptr _to_return;
            this->__slots[_slot_idx].value.swap(_to_return);
            this->__slots[_slot_idx].sequence->store(_slot_idx, std::memory_order_release);

            // Update the approximate size counter.
            this->__size->fetch_sub(1);
            return _to_return;
        }

        // CAS failed — another consumer claimed this slot first.
        // _tail was updated by CAS; loop and retry.
    }
}

template<typename T>
auto zpt::lf::queue<T>::capacity() const -> size_t {
    return this->__capacity;
}

template<typename T>
auto zpt::lf::queue<T>::size() const -> size_t {
    return this->__size->load(std::memory_order_relaxed);
}

template<typename T>
auto zpt::lf::queue<T>::to_string() const -> std::string {
    return static_cast<std::string>(*this);
}

template<typename T>
zpt::lf::queue<T>::operator std::string() const {
    std::ostringstream _oss;
    _oss << (*this) << std::flush;
    return _oss.str();
}
