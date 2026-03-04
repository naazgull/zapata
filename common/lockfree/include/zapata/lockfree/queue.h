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
 * @brief Internal node for lock-free singly-linked structures.
 *
 * Stores a value and an atomic pointer to the next node. Used internally
 * by lock-free queue and other linked structures.
 *
 * @tparam T Value type (must be copy-constructible).
 */
template<typename T>
class forward_node {
  public:
    /** @brief Atomic pointer type for linking nodes. */
    using ptr = zpt::padded_atomic<zpt::lf::forward_node<T>*>;

    T __value;                                       ///< Stored value
    zpt::padded_atomic<bool> __is_null{ true };      ///< True if node is sentinel/empty
    zpt::lf::forward_node<T>::ptr __next{ nullptr }; ///< Pointer to next node

    /** @brief Default constructor (sentinel/empty node). */
    forward_node() = default;
    /** @brief Constructs a node with the given value. */
    forward_node(T _value);
    forward_node(forward_node const&) = delete;
    forward_node(forward_node&&) = delete;
    /** @brief Destructor. */
    virtual ~forward_node() = default;

    auto operator=(forward_node const&) -> forward_node& = delete;
    auto operator=(forward_node&&) -> forward_node& = delete;

    friend auto operator<<(std::ostream& _out, zpt::lf::forward_node<T>& _in) -> std::ostream& {
        if constexpr (std::is_pointer<T>::value) { _out << *(_in.__value) << std::flush; }
        else { _out << _in.__value << std::flush; }
        _out << std::flush;
        return _out;
    }
};

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
    zpt::padded_atomic<std::uint64_t> __max{ 0 };
    zpt::padded_atomic<std::uint64_t> __next{ 0 };
    zpt::padded_atomic<std::uint64_t> __size{ 0 };
    std::uint64_t __capacity{ 0 };
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
    auto _tail = this->__elements[this->__next->load() % this->__capacity];
    if (_tail != nullptr) { return _tail; }
    throw zpt::NoMoreElementsException("there is no element in the front");
}

template<typename T>
auto zpt::lf::queue<T>::push(T _value) -> zpt::lf::queue<T>& {    
    return this->push(std::make_shared<T>(_value));
}

template<typename T>
auto zpt::lf::queue<T>::push(T _value) -> zpt::lf::queue<T>& {
    
    return (*this); // never reached
}

template<typename T>
auto zpt::lf::queue<T>::pop() -> T {
    do {
        typename zpt::lf::queue<T>::hazard_domain::guard _head_sentry{ *this->__head,
                                                                       this->__hazard_domain };
        auto _head = _head_sentry.target();
        auto _next = _head->__next->load(std::memory_order_acquire);
        if (_next == nullptr) { break; }

        if (this->__head->compare_exchange_strong(_head, _next, std::memory_order_release)) {
            while (_head->__is_null);
            --(*this->__size);
            _head->__is_null = true;
            _head_sentry.retire();
            return std::move(_head->__value);
        }
    } while (true);
    throw NoMoreElementsException("no element to pop");
}

template<typename T>
auto zpt::lf::queue<T>::begin() const -> zpt::lf::queue<T>::iterator {
    return zpt::lf::queue<T>::iterator{ (*this->__head).load() };
}

template<typename T>
auto zpt::lf::queue<T>::end() const -> zpt::lf::queue<T>::iterator {
    return zpt::lf::queue<T>::iterator{ (*this->__tail).load() };
}

template<typename T>
auto zpt::lf::queue<T>::size() const -> size_t {
    return this->__size->load(std::memory_order_relaxed);
}

template<typename T>
auto zpt::lf::queue<T>::get_thread_dangling_count() const -> size_t {
    return this->__hazard_domain.get_thread_dangling_count();
}

template<typename T>
auto zpt::lf::queue<T>::clear_thread_context() -> queue<T>& {
    this->__hazard_domain.clear_thread_context();
    return (*this);
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
zpt::lf::queue<T>::iterator::iterator(zpt::lf::forward_node<T>* _current)
  : __initial{ _current }
  , __current{ _current } {}

template<typename T>
zpt::lf::queue<T>::iterator::iterator(const iterator& _rhs)
  : __initial{ _rhs.__initial }
  , __current{ _rhs.__current } {}

template<typename T>
zpt::lf::queue<T>::iterator::iterator(iterator&& _rhs)
  : __initial{ _rhs.__initial }
  , __current{ _rhs.__current } {
    _rhs.__initial = nullptr;
    _rhs.__current = nullptr;
}

template<typename T>
typename zpt::lf::queue<T>::iterator& zpt::lf::queue<T>::iterator::operator=(const iterator& _rhs) {
    this->__initial = _rhs.__initial;
    this->__current = _rhs.__current;
    return (*this);
}

template<typename T>
typename zpt::lf::queue<T>::iterator& zpt::lf::queue<T>::iterator::operator=(iterator&& _rhs) {
    this->__initial = _rhs.__initial;
    this->__current = _rhs.__current;
    _rhs.__initial = nullptr;
    _rhs.__current = nullptr;
    return (*this);
}

template<typename T>
auto zpt::lf::queue<T>::iterator::operator++() -> zpt::lf::queue<T>::iterator& {
    if (this->__current != nullptr) { this->__current = this->__current->__next->load(); }
    return (*this);
}

template<typename T>
auto zpt::lf::queue<T>::iterator::operator*() -> zpt::lf::queue<T>::iterator::reference {
    return this->__current->__value;
}

template<typename T>
typename zpt::lf::queue<T>::iterator zpt::lf::queue<T>::iterator::operator++(int) {
    auto _to_return = (*this);
    ++(*this);
    return _to_return;
}

template<typename T>
auto zpt::lf::queue<T>::iterator::operator->() -> zpt::lf::queue<T>::iterator::pointer {
    return this->__current->__value;
}

template<typename T>
auto zpt::lf::queue<T>::iterator::operator==(iterator const& _rhs) const -> bool {
    return this->__current == _rhs.__current;
}

template<typename T>
auto zpt::lf::queue<T>::iterator::operator!=(iterator const& _rhs) const -> bool {
    return !((*this) == _rhs);
}

template<typename T>
auto zpt::lf::queue<T>::iterator::node() const -> zpt::lf::forward_node<T>* {
    return this->__current;
}
