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

    T __value;                                      ///< Stored value
    zpt::padded_atomic<bool> __is_null{ true };     ///< True if node is sentinel/empty
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
 * // Create queue supporting up to 8 threads
 * zpt::lf::queue<std::string> queue(8);
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
 * // Before thread exits
 * queue.clear_thread_context();
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
    /** @brief Hazard pointer domain type for this queue. */
    using hazard_domain = zpt::lf::hazard_ptr<zpt::lf::forward_node<T>>;

    class iterator {
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T;
        using reference = T;
        using iterator_category = std::forward_iterator_tag;

        /** @brief Constructs an iterator at the given node. */
        explicit iterator(zpt::lf::forward_node<T>* _current);
        /** @brief Copy constructor. */
        iterator(const iterator& _rhs);
        /** @brief Move constructor. */
        iterator(iterator&& _rhs);
        /** @brief Destructor. */
        virtual ~iterator() = default;

        /** @brief Copy assignment. */
        auto operator=(const iterator& _rhs) -> iterator&;
        /** @brief Move assignment. */
        auto operator=(iterator&& _rhs) -> iterator&;
        /** @brief Pre-increment: advances to next node. */
        auto operator++() -> iterator&;
        /** @brief Dereference: returns the stored value. */
        auto operator*() -> reference;

        /** @brief Post-increment: advances and returns previous. */
        auto operator++(int) -> iterator;
        /** @brief Arrow operator: returns the stored value. */
        auto operator->() -> pointer;
        /** @brief Equality comparison. */
        auto operator==(iterator const& _rhs) const -> bool;
        /** @brief Inequality comparison. */
        auto operator!=(iterator const& _rhs) const -> bool;

        /** @brief Returns the underlying node pointer. */
        auto node() const -> zpt::lf::forward_node<T>*;

      private:
        zpt::lf::forward_node<T> const* __initial{ nullptr };
        zpt::lf::forward_node<T>* __current{ nullptr };
    };

    /**
     * @brief Constructs a queue with the specified thread capacity.
     * @param _max_threads Maximum number of threads that will access the queue.
     */
    queue(long _max_threads);
    queue(zpt::lf::queue<T> const& _rhs) = delete;
    queue(zpt::lf::queue<T>&& _rhs) = delete;
    virtual ~queue();

    auto operator=(zpt::lf::queue<T> const& _rhs) -> zpt::lf::queue<T>& = delete;
    auto operator=(zpt::lf::queue<T>&& _rhs) -> zpt::lf::queue<T>& = delete;

    /**
     * @brief Returns the front element without removing it.
     * @return Copy of the front element.
     * @throws zpt::NoMoreElementsException If queue is empty.
     */
    auto front() const -> T;
    /**
     * @brief Returns the back element without removing it.
     * @return Copy of the back element.
     * @throws zpt::NoMoreElementsException If queue is empty.
     */
    auto back() const -> T;

    /** @brief Returns pointer to head node (internal use). */
    auto head() const -> zpt::lf::forward_node<T>*;
    /** @brief Returns pointer to tail node (internal use). */
    auto tail() const -> zpt::lf::forward_node<T>*;

    /**
     * @brief Adds an element to the back of the queue.
     * @param value Value to add (will be copied).
     * @return Reference to this queue.
     */
    auto push(T value) -> zpt::lf::queue<T>&;
    /**
     * @brief Removes and returns the front element.
     * @return The front element (moved).
     * @throws zpt::NoMoreElementsException If queue is empty.
     */
    auto pop() -> T;

    /** @brief Returns iterator to front element. */
    auto begin() const -> zpt::lf::queue<T>::iterator;
    /** @brief Returns iterator past back element. */
    auto end() const -> zpt::lf::queue<T>::iterator;

    /** @brief Returns approximate element count. */
    auto size() const -> size_t;

    /** @brief Cleans up thread-local state (call before thread exit). */
    auto clear_thread_context() -> zpt::lf::queue<T>&;
    /** @brief Returns count of retired nodes pending deletion. */
    auto get_thread_dangling_count() const -> size_t;

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
    zpt::lf::forward_node<T>::ptr __head{ nullptr };
    zpt::lf::forward_node<T>::ptr __tail{ nullptr };
    zpt::lf::queue<T>::hazard_domain __hazard_domain;
    zpt::padded_atomic<std::uint64_t> __size;
};
} // namespace lf
} // namespace zpt

template<typename T>
zpt::lf::forward_node<T>::forward_node(T _value)
  : __value{ _value } {}

template<typename T>
zpt::lf::queue<T>::queue(long _max_threads)
  : __hazard_domain{ _max_threads, 2 } {
    auto _initial = new zpt::lf::forward_node<T>();
    this->__head->store(_initial);
    this->__tail->store(_initial);
}

template<typename T>
zpt::lf::queue<T>::~queue() {
    for (auto _it = this->__head->load(); _it != this->__tail->load();) {
        auto _current = _it;
        _it = _it->__next;
        delete _current;
    }
    delete this->__tail->load();
}

template<typename T>
auto zpt::lf::queue<T>::front() const -> T {
    auto _front = this->head();
    if (_front != nullptr && _front->__next->load() != nullptr) { return _front->__value; }
    throw zpt::NoMoreElementsException("there is no element in the front");
}

template<typename T>
auto zpt::lf::queue<T>::back() const -> T {
    auto _tail = this->tail();
    if (_tail != nullptr && !_tail->__is_null->load(std::memory_order_relaxed)) {
        return _tail->__value;
    }
    throw zpt::NoMoreElementsException("there is no element in the back");
}

template<typename T>
auto zpt::lf::queue<T>::head() const -> zpt::lf::forward_node<T>* {
    return this->__head->load(std::memory_order_relaxed);
}

template<typename T>
auto zpt::lf::queue<T>::tail() const -> zpt::lf::forward_node<T>* {
    return this->__tail->load(std::memory_order_relaxed);
}

template<typename T>
auto zpt::lf::queue<T>::push(T _value) -> zpt::lf::queue<T>& {
    zpt::lf::forward_node<T>* _new{ new zpt::lf::forward_node<T>{} };
    typename zpt::lf::queue<T>::hazard_domain::guard _new_sentry{ _new, this->__hazard_domain };

    do {
        typename zpt::lf::queue<T>::hazard_domain::guard _tail_sentry{ *this->__tail,
                                                                       this->__hazard_domain };
        auto _tail = _tail_sentry.target();
        zpt::lf::forward_node<T>* _null{ nullptr };
        if (_tail->__next->compare_exchange_strong(_null, _new)) {
            ++(*this->__size);
            _tail->__value = _value;
            _tail->__is_null = false;
            this->__tail->store(_new, std::memory_order_release);
            return (*this);
        }
    } while (true);

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
