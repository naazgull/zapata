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

#pragma once

#include <atomic>
#include <iostream>
#include <iterator>
#include <math.h>
#include <memory>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <type_traits>

#include <zapata/base/sentry.h>
#include <zapata/lockfree/atomics.h>
#include <zapata/lockfree/hazard_ptr.h>
#include <zapata/exceptions/exceptions.h>

namespace zpt {
namespace lf {

template<typename T>
class forward_node {
  public:
    using ptr = zpt::padded_atomic<zpt::lf::forward_node<T>*>;

    T __value;
    zpt::padded_atomic<bool> __is_null{ true };
    zpt::lf::forward_node<T>::ptr __next{ nullptr };

    forward_node() = default;
    forward_node(T _value);
    forward_node(forward_node const&) = delete;
    forward_node(forward_node&&) = delete;
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

template<typename T>
class queue {
    static_assert(std::is_copy_constructible<T>::value,
                  "Type `T` in `zpt::lf::queue<T>` must be copy constuctible.");

  public:
    using size_type = size_t;
    using hazard_domain = zpt::lf::hazard_ptr<zpt::lf::forward_node<T>>;

    class iterator {
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T;
        using reference = T;
        using iterator_category = std::forward_iterator_tag;

        explicit iterator(zpt::lf::forward_node<T>* _current);
        iterator(const iterator& _rhs);
        iterator(iterator&& _rhs);
        virtual ~iterator() = default;

        // BASIC ITERATOR METHODS //
        auto operator=(const iterator& _rhs) -> iterator&;
        auto operator=(iterator&& _rhs) -> iterator&;
        auto operator++() -> iterator&;
        auto operator*() -> reference;
        // END / BASIC ITERATOR METHODS //

        // INPUT ITERATOR METHODS //
        auto operator++(int) -> iterator;
        auto operator->() -> pointer;
        auto operator==(iterator const& _rhs) const -> bool;
        auto operator!=(iterator const& _rhs) const -> bool;
        // END / INPUT ITERATOR METHODS //

        // OUTPUT ITERATOR METHODS //
        // reference operator*(); <- already defined
        // iterator operator++(int); <- already defined
        // END / OUTPUT ITERATOR METHODS //

        // FORWARD ITERATOR METHODS //
        // Enable support for both input and output iterator <- already enabled
        // END / FORWARD ITERATOR METHODS //

        auto node() const -> zpt::lf::forward_node<T>*;

      private:
        zpt::lf::forward_node<T> const* __initial{ nullptr };
        zpt::lf::forward_node<T>* __current{ nullptr };
    };

    queue(zpt::lf::queue<T>::hazard_domain& _hazard_domain);
    queue(zpt::lf::queue<T> const& _rhs) = delete;
    queue(zpt::lf::queue<T>&& _rhs) = delete;
    virtual ~queue();

    auto operator=(zpt::lf::queue<T> const& _rhs) -> zpt::lf::queue<T>& = delete;
    auto operator=(zpt::lf::queue<T>&& _rhs) -> zpt::lf::queue<T>& = delete;

    auto front() -> T;
    auto back() -> T;

    auto head() -> zpt::lf::forward_node<T>*;
    auto tail() -> zpt::lf::forward_node<T>*;

    auto push(T value) -> zpt::lf::queue<T>&;
    auto pop() -> T;

    auto begin() -> zpt::lf::queue<T>::iterator;
    auto end() -> zpt::lf::queue<T>::iterator;

    auto clear() -> zpt::lf::queue<T>&;
    auto get_thread_dangling_count() const -> size_t;

    std::string to_string() const __attribute__((noinline));
    operator std::string();

    friend auto operator<<(std::ostream& _out, zpt::lf::queue<T>& _in) -> std::ostream& {
        _out << "* zpt::lf::queue(" << std::hex << &_in << "):" << std::dec << std::endl
             << std::flush;
        _out << "  #head -> " << std::flush;
        try {
            auto _head = _in.head();
            _out << *_head << std::flush;
        }
        catch (zpt::NoMoreElementsException const& e) {
            _out << "0x0" << std::flush;
        }
        _out << std::endl << "  #tail -> " << std::flush;
        try {
            auto _tail = _in.tail();
            _out << *_tail << std::flush;
        }
        catch (zpt::NoMoreElementsException const& e) {
            _out << "0x0" << std::flush;
        }
        auto _count = 0;
        _out << std::endl << std::endl << "  #items -> [" << std::flush;
        try {
            for (auto _it = _in.begin(); _it != _in.end(); ++_it) {
                if (_it.node()->__is_null->load()) continue;
                _out << (_count % 5 == 0 ? "\n\t" : ", ") << *_it.node() << std::flush;
                _count++;
            }
        }
        catch (zpt::NoMoreElementsException const& e) {
            _count = 0;
        }
        _out << (_count != 0 ? "  " : "") << "]" << std::flush;
        _out << std::endl << "   (" << _count << " elements)" << std::flush;
        _out << std::endl << std::endl << _in.__hazard_domain << std::flush;
        return _out;
    }

  private:
    zpt::lf::forward_node<T>::ptr __head{ nullptr };
    zpt::lf::forward_node<T>::ptr __tail{ nullptr };
    zpt::lf::queue<T>::hazard_domain& __hazard_domain;
};

template<typename T>
zpt::lf::forward_node<T>::forward_node(T _value)
  : __value{ _value } {}

template<typename T>
zpt::lf::queue<T>::queue(zpt::lf::queue<T>::hazard_domain& _hazard_domain)
  : __hazard_domain{ _hazard_domain } {
    auto _initial = new zpt::lf::forward_node<T>();
    this->__head->store(_initial);
    this->__tail->store(_initial);
}

template<typename T>
zpt::lf::queue<T>::~queue() {}

template<typename T>
auto
zpt::lf::queue<T>::front() -> T {
    return this->head()->__value;
}

template<typename T>
auto
zpt::lf::queue<T>::back() -> T {
    return this->tail()->__value;
}

template<typename T>
auto
zpt::lf::queue<T>::head() -> zpt::lf::forward_node<T>* {
    typename zpt::lf::queue<T>::hazard_domain::guard _front_sentry{ *this->__head,
                                                                    this->__hazard_domain };
    if (_front_sentry.is_acquired()) {
        auto _front = _front_sentry.target();
        if (_front != nullptr && !_front->__is_null->load()) { return _front; }
    }
    throw zpt::NoMoreElementsException("there is no element in the front");
}

template<typename T>
auto
zpt::lf::queue<T>::tail() -> zpt::lf::forward_node<T>* {
    typename zpt::lf::queue<T>::hazard_domain::guard _back_sentry{ *this->__tail,
                                                                   this->__hazard_domain };
    if (_back_sentry.is_acquired()) {
        auto _back = _back_sentry.target();
        if (_back != nullptr && !_back->__is_null->load()) { return _back; }
    }
    throw zpt::NoMoreElementsException("there is no element in the back");
}

template<typename T>
auto
zpt::lf::queue<T>::push(T _value) -> zpt::lf::queue<T>& {
    zpt::lf::forward_node<T>* _new{ new zpt::lf::forward_node<T>{} };
    typename zpt::lf::queue<T>::hazard_domain::guard _new_sentry{ _new, this->__hazard_domain };

    do {
        typename zpt::lf::queue<T>::hazard_domain::guard _tail_sentry{ *this->__tail,
                                                                       this->__hazard_domain };
        zpt::lf::forward_node<T>* _null{ nullptr };
        if (_tail_sentry.is_acquired()) {
            auto _tail = _tail_sentry.target();
            if (_tail->__next->compare_exchange_strong(_null, _new)) {
                _tail->__value = _value;
                _tail->__is_null->store(false);
                this->__tail->store(_new, std::memory_order_release);
                return (*this);
            }
        }
        std::this_thread::yield();
    } while (true);

    return (*this); // never reached
}

template<typename T>
auto
zpt::lf::queue<T>::pop() -> T {
    do {
        typename zpt::lf::queue<T>::hazard_domain::guard _head_sentry{ *this->__head,
                                                                       this->__hazard_domain };
        if (_head_sentry.is_acquired()) {
            auto _head = _head_sentry.target();
            if (_head->__is_null->load()) { break; }
            else {
                try {
                    auto _next = _head->__next->load(std::memory_order_acquire);
                    if (this->__head->compare_exchange_strong(
                          _head, _next, std::memory_order_release)) {
                        _head->__is_null->store(true);
                        T _value = _head->__value;
                        _head_sentry.retire();
                        return _value;
                    }
                }
                catch (zpt::failed_expectation const& _e) {
                    std::cout << std::hex << _head << std::endl << std::flush;
                    exit(0);
                }
            }
        }
        std::this_thread::yield();
    } while (true);
    throw NoMoreElementsException("no element to pop");
}

template<typename T>
auto
zpt::lf::queue<T>::begin() -> zpt::lf::queue<T>::iterator {
    return zpt::lf::queue<T>::iterator{ (*this->__head).load() };
}

template<typename T>
auto
zpt::lf::queue<T>::end() -> zpt::lf::queue<T>::iterator {
    return zpt::lf::queue<T>::iterator{ nullptr };
}

template<typename T>
auto
zpt::lf::queue<T>::clear() -> zpt::lf::queue<T>& {
    try {
        while (true) { this->pop(); }
    }
    catch (zpt::NoMoreElementsException const& e) {
    }
    this->__hazard_domain.clear();
    return (*this);
}

template<typename T>
auto
zpt::lf::queue<T>::get_thread_dangling_count() const -> size_t {
    return this->__hazard_domain.get_thread_dangling_count();
}

template<typename T>
auto
zpt::lf::queue<T>::to_string() const -> std::string {
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
typename zpt::lf::queue<T>::iterator&
zpt::lf::queue<T>::iterator::operator=(const iterator& _rhs) {
    this->__initial = _rhs.__initial;
    this->__current = _rhs.__current;
    return (*this);
}

template<typename T>
typename zpt::lf::queue<T>::iterator&
zpt::lf::queue<T>::iterator::operator=(iterator&& _rhs) {
    this->__initial = _rhs.__initial;
    this->__current = _rhs.__current;
    _rhs.__initial = nullptr;
    _rhs.__current = nullptr;
    return (*this);
}

template<typename T>
auto
zpt::lf::queue<T>::iterator::operator++() -> zpt::lf::queue<T>::iterator& {
    if (this->__current != nullptr) { this->__current = this->__current->__next->load(); }
    return (*this);
}

template<typename T>
auto
zpt::lf::queue<T>::iterator::operator*() -> zpt::lf::queue<T>::iterator::reference {
    return this->__current->__value;
}

template<typename T>
typename zpt::lf::queue<T>::iterator
zpt::lf::queue<T>::iterator::operator++(int) {
    auto _to_return = (*this);
    ++(*this);
    return _to_return;
}

template<typename T>
auto
zpt::lf::queue<T>::iterator::operator->() -> zpt::lf::queue<T>::iterator::pointer {
    return this->__current->__value;
}

template<typename T>
auto
zpt::lf::queue<T>::iterator::operator==(iterator const& _rhs) const -> bool {
    return this->__current == _rhs.__current;
}

template<typename T>
auto
zpt::lf::queue<T>::iterator::operator!=(iterator const& _rhs) const -> bool {
    return !((*this) == _rhs);
}

template<typename T>
auto
zpt::lf::queue<T>::iterator::node() const -> zpt::lf::forward_node<T>* {
    return this->__current;
}

} // namespace lf
} // namespace zpt
