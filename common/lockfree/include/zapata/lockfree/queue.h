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
 * algorithm. Every slot carries a position-tagged `sequence` token. A
 * thread reads the token for its own position first and only attempts the
 * CAS on the shared head_/tail_ counter when the token confirms the slot
 * is ready, so claiming a slot and verifying its readiness form a single
 * atomic handshake — a thread never owns a slot that is not ready, and a
 * consumer can never claim a position the producer has not published.
 *
 * @par Thread Safety
 * - `push()`: Multiple threads can push concurrently.
 * - `pop()`: Multiple threads can pop concurrently; returns nullptr
 *    when the queue is empty.
 * - `size()`: Approximate count (may be transiently stale).
 */

#pragma once

#include <bit>
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
 * Each slot carries a position-tagged `sequence` token; producers and
 * consumers share two counters (`head_` for enqueue, `tail_` for dequeue).
 * A thread checks the token for its position and only claims the slot via
 * a CAS on the matching counter when the token says the slot is ready; it
 * then advances the token to hand the slot to the other side. Because the
 * readiness check and the claim are tied to the same absolute position,
 * `tail_` can never advance past `head_`.
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
 * while (true) {
 *     auto msg = q.pop();
 *     if (msg != nullptr) {
 *         process(*msg);
 *     }
 *     else { // Queue is empty }
 * }
 * @endcode
 *
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
     *   hold simultaneously (rounded up to the next power of two).
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
     */
    auto push(T _value) -> zpt::lf::queue<T>&;
    /**
     * @brief Adds an element to the back of the queue, transferring ownership.
     * @param _value Unique pointer whose ownership is transferred to the queue.
     * @return Reference to this queue.
     */
    auto push(ptr&& _value) -> zpt::lf::queue<T>&;
    /**
     * @brief Removes and returns the front element (non-blocking).
     * @return Unique pointer owning the dequeued element, or nullptr if the
     *   queue is empty or has been shut down.
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
     * @brief Clears the queue's content.
     * @return Reference to this queue.
     */
    auto clear() -> zpt::lf::queue<T>&;
    /**
     * @brief Instructs all threads spinning in `push` or `pop` to exit.
     * @return Reference to this queue.
     */
    auto shutdown() -> zpt::lf::queue<T>&;

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
            for (size_t _idx = _lower; _idx < _upper; ++_idx, ++_count) {
                size_t _pos = _idx % _in.__capacity;
                if (_in.__slots[_pos].value != nullptr) {
                    _out << (_count == 0 ? "" : (_count % 5 == 0 ? "\n       " : ", "))
                         << *_in.__slots[_pos].value;
                }
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
     * The `sequence` field is a position-tagged ownership token (Vyukov).
     * For an absolute position `pos` mapping to this slot:
     *   - `pos`         → slot is free; a producer may claim it to enqueue at `pos`
     *   - `pos + 1`     → slot is full; a consumer may claim it to dequeue at `pos`
     *   - `pos + C`     → slot was freed by that dequeue, ready for the next
     *                     lap's enqueue at `pos + C` (C = queue capacity)
     * The token grows with the absolute position, so it distinguishes laps and
     * a thread only ever claims a slot whose token matches its own position.
     */
    struct alignas(64) slot_t {
        zpt::padded_atomic<std::uint64_t> sequence{ 0 };
        ptr value;
    };

    /** @brief Maximum number of elements the queue can hold (power of two). */
    size_t __capacity{ 0 };
    /** @brief Fixed-size ring buffer of slots allocated at construction. */
    zpt::allocator<slot_t>::array_pointer __slots{ nullptr };
    /** @brief Head index: producers claim the next slot via CAS on head_. */
    zpt::padded_atomic<std::uint64_t> __head{ 0 };
    /** @brief Tail index: consumers claim the next slot via CAS on tail_. */
    zpt::padded_atomic<std::uint64_t> __tail{ 0 };
    /** @brief Approximate element count, updated independently of the slot CAS. */
    zpt::padded_atomic<std::uint64_t> __size{ 0 };
    /** @brief Stop all spinning and exit. */
    zpt::padded_atomic<bool> __shutdown{ false };
    /** @brief Bit mask for index wrapping: `__capacity - 1`. */
    size_t __mask{ 0 };
};
} // namespace lf
} // namespace zpt

template<typename T>
zpt::lf::queue<T>::queue(size_t _max_queue_size)
  : __capacity{ std::bit_ceil(_max_queue_size) }
  , __slots{ zpt::allocate_array<slot_t>(__capacity) }
  , __mask{ __capacity - 1 } {
    assert((this->__capacity & (this->__capacity - 1)) == 0);
    for (size_t _i = 0; _i != this->__capacity; ++_i) {
        this->__slots[_i].sequence->store(_i, std::memory_order_relaxed);
    }
}

template<typename T>
auto zpt::lf::queue<T>::push(T _value) -> zpt::lf::queue<T>& {
    return push(zpt::allocate_unique<T>(_value));
}

template<typename T>
auto zpt::lf::queue<T>::push(ptr&& _value) -> zpt::lf::queue<T>& {
    zpt::this_thread::timer<float> _timer{ 0.01f };
    std::uint64_t _pos = this->__head->load(std::memory_order_relaxed);
    for (;;) {
        if (this->__shutdown->load(std::memory_order_relaxed)) { return (*this); }

        auto& _cell = this->__slots[_pos & this->__mask];
        auto _seq = _cell.sequence->load(std::memory_order_acquire);
        auto _dif = static_cast<std::int64_t>(_seq) - static_cast<std::int64_t>(_pos);

        if (_dif == 0) {
            // Slot is free for this position: claim it by advancing head_.
            // The readiness check and the claim share `_pos`, so we never own
            // a slot we have not verified.
            if (this->__head->compare_exchange_weak(_pos, _pos + 1, std::memory_order_relaxed)) {
                _cell.value = std::move(_value);
                _cell.sequence->store(_pos + 1, std::memory_order_release);
                this->__size->fetch_add(1, std::memory_order_relaxed);
                return (*this);
            }
            // CAS failed: _pos was reloaded; re-evaluate against the new position.
        }
        else if (_dif < 0) {
            // Queue full: the slot still holds an item from the previous lap.
            // Back off until a consumer frees it (or shutdown breaks us out).
            _timer.sleep_for(0.1f);
        }
        else {
            // Slot already advanced by another producer: our _pos is stale.
            _pos = this->__head->load(std::memory_order_relaxed);
        }
    }
}

template<typename T>
auto zpt::lf::queue<T>::pop() -> ptr {
    std::uint64_t _pos = this->__tail->load(std::memory_order_relaxed);
    for (;;) {
        if (this->__shutdown->load(std::memory_order_relaxed)) { return nullptr; }

        auto& _cell = this->__slots[_pos & this->__mask];
        auto _seq = _cell.sequence->load(std::memory_order_acquire);
        auto _dif = static_cast<std::int64_t>(_seq) - static_cast<std::int64_t>(_pos + 1);

        if (_dif == 0) {
            // Slot is full for this position: claim it by advancing tail_.
            // Readiness is verified for `_pos` before the CAS.
            if (this->__tail->compare_exchange_weak(_pos, _pos + 1, std::memory_order_relaxed)) {
                ptr _to_return;
                _cell.value.swap(_to_return);
                _cell.sequence->store(_pos + this->__mask + 1, std::memory_order_release);
                this->__size->fetch_sub(1, std::memory_order_relaxed);
                return _to_return;
            }
            // CAS failed: _pos was reloaded; re-evaluate against the new position.
        }
        else if (_dif < 0) {
            // Queue empty: nothing published at this position.
            return nullptr;
        }
        else {
            // Slot already advanced by another consumer: our _pos is stale.
            _pos = this->__tail->load(std::memory_order_relaxed);
        }
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
auto zpt::lf::queue<T>::clear() -> zpt::lf::queue<T>& {
    while (this->pop() != nullptr);
    return (*this);
}

template<typename T>
auto zpt::lf::queue<T>::shutdown() -> zpt::lf::queue<T>& {
    this->__shutdown->store(true);
    return (*this);
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
