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
 * @brief Lock-free FIFO queue implementation.
 *
 * Provides a thread-safe, lock-free queue using Michael & Scott's algorithm
 * with hazard pointer-based memory reclamation. Multiple threads can safely
 * push and pop concurrently without locks.
 *
 * @par Algorithm
 * Based on "Simple, Fast, and Practical Non-Blocking and Blocking Concurrent
 * Queue Algorithms" by Michael and Scott (1996), with hazard pointers for
 * safe memory reclamation (Maged Michael, 2004).
 *
 * @par Thread Safety
 * - `push()`: Multiple threads can push concurrently
 * - `pop()`: Multiple threads can pop concurrently
 * - `size()`: Approximate count (may be stale)
 * - Threads must call `clear_thread_context()` before exiting
 *
 * @see zpt::lf::hazard_ptr
 */

#pragma once

#include <zapata/atomics/padded_atomic.h>
#include <zapata/base/sentry.h>
#include <zapata/exceptions/exceptions.h>
#include <zapata/lockfree/hazard_ptr.h>

namespace zpt {
namespace lf {

/**
 * @brief Lock-free FIFO queue for concurrent producer/consumer patterns.
 *
 * A thread-safe queue that allows multiple threads to push and pop elements
 * concurrently without using locks. Uses compare-and-swap operations and
 * hazard pointers for memory safety.
 *
 * @tparam T Value type (must be copy-constructible).
 *
 * @par Example
 * @code
 * // Create queue supporting up to 10000 elements in the queue
 * zpt::lf::queue<std::string> queue(10000);
 *
 * // Producer thread
 * queue.push("message 1");
 * queue.push("message 2");
 *
 * // Consumer thread
 * try {
 *     while (true) {
 *         std::string msg = queue.pop();
 *         process(msg);
 *     }
 * } catch (zpt::NoMoreElementsException&) {
 *     // Queue is empty
 * }
 *
 * @endcode
 *
 * @note The `size()` method returns an approximate count that may be stale
 *       due to concurrent modifications.
 */
template<typename T>
class queue {
    static_assert(std::is_copy_constructible<T>::value,
                  "Type `T` in `zpt::lf::queue<T>` must be copy constuctible.");

  public:
    using size_type = size_t;
    using ptr = std::shared_ptr<T>;
    using const_ptr = std::shared_ptr<T const>;

    /**
     * @brief Constructs a queue with the specified thread capacity.
     * @param _max_queue_size Maximum number elements in the queue.
     */
    queue(long _max_queue_size);
    queue(zpt::lf::queue<T> const& _rhs) = delete;
    queue(zpt::lf::queue<T>&& _rhs) = delete;
    virtual ~queue() = default;

    auto operator=(zpt::lf::queue<T> const& _rhs) -> zpt::lf::queue<T>& = delete;
    auto operator=(zpt::lf::queue<T>&& _rhs) -> zpt::lf::queue<T>& = delete;

    /**
     * @brief Returns the front element without removing it.
     * @return Copy of the front element.
     * @throws zpt::NoMoreElementsException If queue is empty.
     */
    auto front() const -> ptr;
    /**
     * @brief Returns the back element without removing it.
     * @return Copy of the back element.
     * @throws zpt::NoMoreElementsException If queue is empty.
     */
    auto back() const -> ptr;

    /**
     * @brief Adds an element to the back of the queue.
     * @param value Value to add (will be copied).
     * @return Reference to this queue.
     */
    auto push(T value) -> zpt::lf::queue<T>&;
    /**
     * @brief Adds an element to the back of the queue.
     * @param value Shared-pointer to the value to add.
     * @return Reference to this queue.
     */
    auto push(ptr value) -> zpt::lf::queue<T>&;
    /**
     * @brief Removes and returns the front element.
     * @return The front element (moved).
     * @throws zpt::NoMoreElementsException If queue is empty.
     */
    auto pop() -> ptr;

    /** @brief Returns approximate element count. */
    auto size() const -> size_t;

    /** @brief Returns a debug string representation of the queue. */
    __attribute__((noinline)) auto to_string() const -> std::string;
    /** @brief Converts to string (calls to_string()). */
    operator std::string();

    friend auto operator<<(std::ostream& _out, zpt::lf::queue<T>& _in) -> std::ostream& {
        _out << "queue(" << std::hex << &_in << "):" << std::dec << std::endl
             << "  #head -> " << std::hex << _in.head() << std::dec << " is_null(" << std::boolalpha
             << _in.head()->__is_null->load(std::memory_order_relaxed) << ")" << std::endl
             << "  #tail -> " << std::hex << _in.tail() << std::dec << " is_null(" << std::boolalpha
             << _in.tail()->__is_null->load(std::memory_order_relaxed) << ")" << std::endl
             << std::endl;

        _out << "  #items ->\n     [ " << std::flush;
        try {
            size_t _count{ 0 };
            for (auto _it = _in.begin(); _it != _in.end(); ++_it, ++_count) {
                _out << (_count == 0 ? "" : (_count % 5 == 0 ? "\n       " : ", ")) << *_it.node()
                     << std::flush;
            }
        }
        catch (zpt::NoMoreElementsException const& e) {
        }
        _out << (_in.size() != 0 ? " " : "") << "]" << std::endl
             << "   (" << _in.size() << " elements) " << _in.__hazard_domain << std::flush;
        return _out;
    }

  private:
    std::unique_ptr<ptr[]> __elements{ nullptr };
    zpt::padded_atomic<__uint128_t> __boundaries{ -1 };
    zpt::padded_atomic<std::uint64_t> __size{ 0 };
    std::uint64_t __capacity{ 0 };

    auto serialize() -> __uint128_t;
    auto deserialize() -> std::tuple<std::uint64_t, std::uint64_t>;
};
} // namespace lf
} // namespace zpt

template<typename T>
zpt::lf::queue<T>::queue(long _max_queue_size)
  : __elements{ std::make_unique<ptr[]>(_max_queue_size) }
  , __capacity{ _max_queue_size } {}

template<typename T>
auto zpt::lf::queue<T>::front() const -> ptr {
    auto _front = this->__elements[this->__max->load() % this->__capacity];
    if (_front != nullptr) { return _front; }
    throw zpt::NoMoreElementsException("there is no element in the front");
}

template<typename T>
auto zpt::lf::queue<T>::back() const -> ptr {
    auto _tail = this->__elements[this->__cur->load() % this->__capacity];
    if (_tail != nullptr) { return _tail; }
    throw zpt::NoMoreElementsException("there is no element in the front");
}

template<typename T>
auto zpt::lf::queue<T>::push(T _value) -> zpt::lf::queue<T>& {
    return this->push(std::make_shared<T>(_value));
}

template<typename T>
auto zpt::lf::queue<T>::push(ptr _value) -> zpt::lf::queue<T>& {
    while (true) {
        auto _boundaries = this->__boundaries->load(std::memory_order_acquire);
        auto [_lower, _upper] = this->deserialize(_boundaries);
        auto _new_upper = 
        auto _new_boundaries = this->serialize(_lower, _new_upper);
        if ((_new_upper - this->__cur->load()) < this->__capacity &&
            this->__upper->compare_exchange_strong(_upper, _new_upper, std::memory_order_release)) {
            this->__elements[_new_upper] = _value;
            return (*this);
        }
        break;
    }
    throw zpt::NoSpaceAvailableException("maximum queue size reached");
}

template<typename T>
auto zpt::lf::queue<T>::pop() -> ptr {
    auto _cur = this->__cur->load(std::memory_order_acquire);
    auto _next = _cur + 1;
    if (_next < this->__max->load() &&
        this->__cur->compare_exchange_strong(_cur, _next, std::memory_order_release)) {}
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
zpt::lf::queue<T>::operator std::string() {
    std::ostringstream _oss;
    _oss << (*this) << std::flush;
    return _oss.str();
}

template<typename T>
auto zpt::lf::queue<T>::serialize(std::uint64_t _lower, std::uint64_t _upper) -> __uint128_t {
    return (static_cast<__uint128_t>(_upper) << 64) + static_cast<__uint128_t>(_lower);
}

template<typename T>
auto zpt::lf::queue::deserialize(__uint128_t _value) -> std::tuple<std::uint64_t, std::uint64_t> {
    return { static_cast<std::uint64_t>(_value), static_cast<std::uint64_t>(_value >> 64) };
}
