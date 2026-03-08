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
 * Provides a thread-safe, bounded queue backed by a fixed-size ring buffer.
 * Head and tail indices are packed into a single 128-bit atomic, with the
 * two most-significant bits used as a mutation guard to serialise concurrent
 * index updates via compare-and-swap. No per-thread state or cleanup is
 * required.
 *
 * @par Thread Safety
 * - `push()`: Multiple threads can push concurrently; spins when the queue is full.
 * - `pop()`: Multiple threads can pop concurrently; throws when the queue is empty.
 * - `size()`: Approximate count (may be transiently stale).
 */

#pragma once

#include <zapata/allocator.h>
#include <zapata/atomics/padded_atomic.h>
#include <zapata/base/sentry.h>
#include <zapata/exceptions/exceptions.h>

namespace zpt {
namespace lf {

/** @brief Bitmask that isolates the lower 126 bits (the packed head/tail indices). */
constexpr __uint128_t UNMASK = (static_cast<__uint128_t>(1) << 126) - 1;
/** @brief Mutation-guard bits (bits 126–127) set during an index update CAS. */
constexpr __uint128_t MASK =
  (static_cast<__uint128_t>(1) << 127) + (static_cast<__uint128_t>(1) << 126);

/**
 * @brief Bounded lock-free FIFO queue for concurrent producer/consumer patterns.
 *
 * Backed by a fixed-capacity ring buffer allocated at construction time.
 * Head and tail positions are packed into a single 128-bit atomic value;
 * the two most-significant bits serve as a mutation guard so that only one
 * CAS winner at a time may advance the index, eliminating the need for
 * per-thread hazard pointers or a linked-node allocator.
 *
 * @tparam T Value type stored in the queue.
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
 * @note `push()` spins until a slot is available when the queue is full.
 * @note `size()` may be transiently stale because the size counter is updated
 *       separately from the index CAS.
 */
template<typename T>
class queue {
  public:
    using size_type = size_t;
    using ptr = zpt::allocator<T>::unique_pointer;

    /**
     * @brief Constructs a bounded queue with the given fixed capacity.
     * @param _max_queue_size Maximum number of elements the queue can hold simultaneously.
     */
    queue(size_t _max_queue_size);
    /** @brief Not copyable — the ring buffer cannot be shared. */
    queue(zpt::lf::queue<T> const& _rhs) = delete;
    /** @brief Not movable — the atomic state cannot be transferred safely. */
    queue(zpt::lf::queue<T>&& _rhs) = delete;
    /** @brief Destructor. */
    ~queue() = default;

    /** @brief Not copyable — the ring buffer cannot be shared. */
    auto operator=(zpt::lf::queue<T> const& _rhs) -> zpt::lf::queue<T>& = delete;
    /** @brief Not movable — the atomic state cannot be transferred safely. */
    auto operator=(zpt::lf::queue<T>&& _rhs) -> zpt::lf::queue<T>& = delete;

    /**
     * @brief Adds an element to the back of the queue.
     * @param value Value to add (will be copied).
     * @return Reference to this queue.
     */
    auto push(T value) -> zpt::lf::queue<T>&;
    /**
     * @brief Adds an element to the back of the queue, transferring ownership.
     * @param value Unique pointer whose ownership is transferred to the queue.
     * @return Reference to this queue.
     */
    auto push(ptr&& value) -> zpt::lf::queue<T>&;
    /**
     * @brief Removes and returns the front element.
     * @return Unique pointer owning the dequeued element.
     * @throws zpt::NoMoreElementsException If queue is empty.
     */
    auto pop() -> ptr;

    /** @brief Returns approximate element count. */
    auto size() const -> size_t;

    /** @brief Returns a debug string representation of the queue. */
    __attribute__((noinline)) auto to_string() const -> std::string;
    /** @brief Converts to string (calls to_string()). */
    operator std::string() const;

    friend auto operator<<(std::ostream& _out, zpt::lf::queue<T>& _in) -> std::ostream& {
        _out << "queue(" << std::hex << &_in << "):" << std::dec << "\n  #items ->\n     [ ";
        try {
            size_t _count{ 0 };
            auto [_lower, _upper] = _in.deserialize(_in.__boundaries->load());
            for (size_t _idx = _lower; _idx != _upper; ++_idx, ++_count) {
                _out << (_count == 0 ? "" : (_count % 5 == 0 ? "\n       " : ", "))
                     << *_in.__elements[_idx % _in.__capacity];
            }
        }
        catch (zpt::NoMoreElementsException const& e) {
        }
        _out << (_in.size() != 0 ? " " : "") << "]\n"
             << "   (" << _in.size() << " elements) ";
        return _out;
    }

  private:
    zpt::allocator<ptr>::array_pointer __elements{ nullptr };
    zpt::padded_atomic<__uint128_t> __boundaries{ 0 };
    zpt::padded_atomic<std::uint64_t> __size{ 0 };
    size_t __capacity{ 0 };

    auto serialize(std::uint64_t _lower, std::uint64_t _upper) const -> __uint128_t;
    auto deserialize(__uint128_t _value) const -> std::tuple<std::uint64_t, std::uint64_t>;
};
} // namespace lf
} // namespace zpt

template<typename T>
zpt::lf::queue<T>::queue(size_t _max_queue_size)
  : __elements{ zpt::allocate_array<ptr>(_max_queue_size) }
  , __boundaries{ 0 }
  , __capacity{ _max_queue_size } {}

template<typename T>
auto zpt::lf::queue<T>::push(T _value) -> zpt::lf::queue<T>& {
    return this->push(zpt::allocate_unique<T>(_value));
}

template<typename T>
auto zpt::lf::queue<T>::push(ptr&& _value) -> zpt::lf::queue<T>& {
    while (true) {
        auto _boundaries = this->__boundaries->load(std::memory_order_acquire) & UNMASK;
        auto [_lower, _upper] = this->deserialize(_boundaries);
        if ((_upper - _lower) < this->__capacity) {
            auto _new_upper = _upper + 1;
            auto _new_boundaries = this->serialize(_lower, _new_upper) | MASK;
            if (this->__boundaries->compare_exchange_strong(
                  _boundaries, _new_boundaries, std::memory_order_release)) {
                this->__elements[_upper % this->__capacity] = std::move(_value);
                this->__size->fetch_add(1);
                this->__boundaries->store(_new_boundaries & UNMASK);
                return (*this);
            }
        }
        std::this_thread::yield();
    }
    return (*this);
}

template<typename T>
auto zpt::lf::queue<T>::pop() -> ptr {
    while (true) {
        auto _boundaries = this->__boundaries->load(std::memory_order_acquire) & UNMASK;
        auto [_lower, _upper] = this->deserialize(_boundaries);
        if (_lower != _upper) {
            auto _new_lower = _lower + 1;
            auto _new_boundaries = this->serialize(_new_lower, _upper) | MASK;
            if (this->__boundaries->compare_exchange_strong(
                  _boundaries, _new_boundaries, std::memory_order_release)) {
                ptr _to_return;
                this->__elements[_lower % this->__capacity].swap(_to_return);
                this->__size->fetch_sub(1);
                this->__boundaries->store(_new_boundaries & UNMASK);
                return _to_return;
            }
        }
        else { break; }
        std::this_thread::yield();
    }
    throw NoMoreElementsException("no element to pop");
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

template<typename T>
auto zpt::lf::queue<T>::serialize(std::uint64_t _lower, std::uint64_t _upper) const -> __uint128_t {
    return (static_cast<__uint128_t>(_upper) << 64) + static_cast<__uint128_t>(_lower);
}

template<typename T>
auto zpt::lf::queue<T>::deserialize(__uint128_t _value) const
  -> std::tuple<std::uint64_t, std::uint64_t> {
    return { static_cast<std::uint64_t>(_value), static_cast<std::uint64_t>(_value >> 64) };
}
