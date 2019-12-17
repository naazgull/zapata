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

#include <zapata/lockfree/hptr.h>
#include <zapata/exceptions/NoMoreElementsException.h>

namespace zpt {
namespace lf {
template<typename T>
class queue {
    static_assert(std::is_copy_constructible<T>::value,
                  "Type `T` in `zpt::lf::queue<T>` must be of copy constuctible.");

  public:
    class node {
      public:
        T __value;
        std::atomic<bool> __is_null{ true };
        std::atomic<zpt::lf::queue<T>::node*> __next{ nullptr };

        node() = default;
        node(T _value);
        virtual ~node() = default;

        friend auto operator<<(std::ostream& _out, zpt::lf::queue<T>::node& _in) -> std::ostream& {
            if constexpr (std::is_pointer<T>::value) {
                _out << *(_in.__value) << std::flush;
            }
            else {
                _out << _in.__value << std::flush;
            }
            _out << std::flush;
            return _out;
        }
    };

    using size_type = size_t;

    class iterator {
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T;
        using reference = T;

        explicit iterator(zpt::lf::queue<T>::node* _current);
        iterator(const iterator& _rhs);
        iterator(iterator&& _rhs);
        virtual ~iterator() = default;

        // BASIC ITERATOR METHODS //
        auto operator=(const iterator& _rhs) -> iterator&;
        auto operator=(iterator&& _rhs) -> iterator&;
        auto operator++() -> iterator&;
        auto operator*() const -> reference;
        // END / BASIC ITERATOR METHODS //

        // INPUT ITERATOR METHODS //
        auto operator++(int) -> iterator;
        auto operator-> () const -> pointer;
        auto operator==(iterator const& _rhs) const -> bool;
        auto operator!=(iterator const& _rhs) const -> bool;
        // END / INPUT ITERATOR METHODS //

        // OUTPUT ITERATOR METHODS //
        // reference operator*() const; <- already defined
        // iterator operator++(int); <- already defined
        // END / OUTPUT ITERATOR METHODS //

        // FORWARD ITERATOR METHODS //
        // Enable support for both input and output iterator <- already enabled
        // END / FORWARD ITERATOR METHODS //

        auto node() const -> zpt::lf::queue<T>::node*;

      private:
        zpt::lf::queue<T>::node const* __initial{ nullptr };
        zpt::lf::queue<T>::node* __current{ nullptr };
    };

    queue(long _max_threads, long _ptr_per_thread, long _spin_sleep_millis = 0);
    queue(const zpt::lf::queue<T>& _rhs);
    queue(zpt::lf::queue<T>&& _rhs);
    virtual ~queue();

    auto operator=(const zpt::lf::queue<T>& _rhs) -> zpt::lf::queue<T>&;
    auto operator=(zpt::lf::queue<T>&& _rhs) -> zpt::lf::queue<T>&;

    auto front() const -> T;
    auto back() const -> T;

    auto head() const -> zpt::lf::queue<T>::node*;
    auto tail() const -> zpt::lf::queue<T>::node*;

    auto push(T value) -> zpt::lf::queue<T>&;
    auto pop() -> T;

    auto begin() const -> zpt::lf::queue<T>::iterator;
    auto end() const -> zpt::lf::queue<T>::iterator;

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
        catch (zpt::NoMoreElementsException& e) {
            _out << "0x0" << std::flush;
        }
        _out << std::endl << "  #tail -> " << std::flush;
        try {
            auto _tail = _in.tail();
            _out << *_tail << std::flush;
        }
        catch (zpt::NoMoreElementsException& e) {
            _out << "0x0" << std::flush;
        }
        size_t _count = 0;
        _out << std::endl << std::endl << "  #items -> [" << std::flush;
        try {
            for (auto _it = _in.begin(); _it != _in.end(); ++_it) {
                if (_it.node()->__is_null.load())
                    continue;
                _out << (_count % 5 == 0 ? "\n\t" : ", ") << *_it.node() << std::flush;
                _count++;
            }
        }
        catch (zpt::NoMoreElementsException& e) {
            _count = 0;
        }
        _out << (_count != 0 ? "  " : "") << "]" << std::flush;
        _out << std::endl << "   (" << _count << " elements)" << std::flush;
        _out << std::endl << std::endl << _in.__hptr << std::flush;
        return _out;
    }

  private:
    std::atomic<zpt::lf::queue<T>::node*> __head{ nullptr };
    std::atomic<zpt::lf::queue<T>::node*> __tail{ nullptr };
    zpt::lf::hptr_domain<zpt::lf::queue<T>::node> __hptr;
    long __spin_sleep;
};

template<typename T>
zpt::lf::queue<T>::queue(long _max_threads, long _ptr_per_thread, long _spin_sleep_millis)
  : __hptr{ _max_threads, _ptr_per_thread }
  , __spin_sleep{ _spin_sleep_millis } {
    auto _initial = new zpt::lf::queue<T>::node();
    this->__head.store(_initial);
    this->__tail.store(_initial);
}

template<typename T>
zpt::lf::queue<T>::~queue() {}

template<typename T>
auto
zpt::lf::queue<T>::front() const -> T {
    return this->head()->__value;
}

template<typename T>
auto
zpt::lf::queue<T>::back() const -> T {
    return this->tail()->__value;
}

template<typename T>
auto
zpt::lf::queue<T>::head() const -> zpt::lf::queue<T>::node* {
    zpt::lf::queue<T>::node* _front = this->__head.load();
    if (_front != nullptr && !_front->__is_null.load()) {
        return _front;
    }
    throw zpt::NoMoreElementsException("there is no element in the back");
}

template<typename T>
auto
zpt::lf::queue<T>::tail() const -> zpt::lf::queue<T>::node* {
    zpt::lf::queue<T>::node* _back = this->__tail.load();
    if (_back != nullptr && !_back->__is_null.load()) {
        return _back;
    }
    throw zpt::NoMoreElementsException("there is no element in the back");
}

template<typename T>
auto
zpt::lf::queue<T>::push(T _value) -> zpt::lf::queue<T>& {
    zpt::lf::queue<T>::node* _new{ new zpt::lf::queue<T>::node{} };
    typename zpt::lf::hptr_domain<node>::guard _new_sentry{ _new, this->__hptr };

    do {
        zpt::lf::queue<T>::node* _tail = this->__tail.load();
        zpt::lf::queue<T>::node* _null{ nullptr };
        typename zpt::lf::hptr_domain<node>::guard _tail_sentry{ _tail, this->__hptr };

        if (_tail->__next.compare_exchange_strong(_null, _new)) {
            _tail->__value = _value;
            _tail->__is_null.store(false);
            this->__tail.store(_new);
            return (*this);
        }
        if (this->__spin_sleep < 0) {
            std::this_thread::yield();
        }
        else if (this->__spin_sleep != 0) {
            std::this_thread::sleep_for(
              std::chrono::duration<int, std::milli>{ this->__spin_sleep });
        }
    } while (true);

    return (*this); // never reached
}

template<typename T>
auto
zpt::lf::queue<T>::pop() -> T {
    do {
        zpt::lf::queue<T>::node* _head = this->__head.load();
        typename zpt::lf::hptr_domain<node>::guard _head_sentry{ _head, this->__hptr };

        if (_head->__is_null.load()) {
            break;
        }
        else {
            zpt::lf::queue<T>::node* _next = _head->__next.load();
            typename zpt::lf::hptr_domain<node>::guard _next_sentry{ _next, this->__hptr };

            if (this->__head.compare_exchange_strong(_head, _next)) {
                _head->__is_null.store(true);
                T _value = _head->__value;
                _head_sentry.retire();
                return _value;
            }
        }
        if (this->__spin_sleep < 0) {
            std::this_thread::yield();
        }
        else if (this->__spin_sleep != 0) {
            std::this_thread::sleep_for(
              std::chrono::duration<int, std::milli>{ this->__spin_sleep });
        }
    } while (true);
    throw NoMoreElementsException("no element to pop");
}

template<typename T>
auto
zpt::lf::queue<T>::begin() const -> zpt::lf::queue<T>::iterator {
    return zpt::lf::queue<T>::iterator{ this->__head.load() };
}

template<typename T>
auto
zpt::lf::queue<T>::end() const -> zpt::lf::queue<T>::iterator {
    return zpt::lf::queue<T>::iterator{ nullptr };
}

template<typename T>
auto
zpt::lf::queue<T>::clear() -> zpt::lf::queue<T>& {
    try {
        while (true) {
            this->pop();
        }
    }
    catch (zpt::NoMoreElementsException& e) {
    }
    this->__hptr.clear();
    return (*this);
}

template<typename T>
auto
zpt::lf::queue<T>::get_thread_dangling_count() const -> size_t {
    return this->__hptr.get_thread_dangling_count();
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
zpt::lf::queue<T>::node::node(T _value)
  : __value{ _value } {}

template<typename T>
zpt::lf::queue<T>::iterator::iterator(zpt::lf::queue<T>::node* _current)
  : __initial(_current)
  , __current(_current) {}

template<typename T>
zpt::lf::queue<T>::iterator::iterator(const iterator& _rhs)
  : __initial(_rhs.__initial)
  , __current(_rhs.__current) {}

template<typename T>
zpt::lf::queue<T>::iterator::iterator(iterator&& _rhs)
  : __initial(_rhs.__initial)
  , __current(_rhs.__current) {}

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
    return (*this);
}

template<typename T>
auto
zpt::lf::queue<T>::iterator::operator++() -> zpt::lf::queue<T>::iterator& {
    if (this->__current != nullptr) {
        this->__current = this->__current->__next.load();
    }
    return (*this);
}

template<typename T>
auto zpt::lf::queue<T>::iterator::operator*() const -> zpt::lf::queue<T>::iterator::reference {
    return this->__current->__value;
}

template<typename T>
typename zpt::lf::queue<T>::iterator
zpt::lf::queue<T>::iterator::operator++(int) {
    zpt::lf::queue<T>::iterator _to_return = (*this);
    ++(*this);
    return _to_return;
}

template<typename T>
auto zpt::lf::queue<T>::iterator::operator-> () const -> zpt::lf::queue<T>::iterator::pointer {
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
zpt::lf::queue<T>::iterator::node() const -> zpt::lf::queue<T>::node* {
    return this->__current;
}

} // namespace lf
} // namespace zpt
