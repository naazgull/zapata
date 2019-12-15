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
class list {
    static_assert(std::is_copy_constructible<T>::value,
                  "Type `T` in `zpt::lf::list<T>` must be of copy constuctible.");

  public:
    class node {
      public:
        T __value;
        std::atomic<bool> __is_null{ true };
        std::atomic<bool> __is_processing{ false },
          std::atomic<zpt::lf::list<T>::node*> __next{ nullptr };
        std::atomic<zpt::lf::list<T>::node*> __prev{ nullptr };

        node() = default;
        node(T _value);
        virtual ~node() = default;

        friend auto operator<<(std::ostream& _out, zpt::lf::list<T>::node& _in) -> std::ostream& {
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

        explicit iterator(zpt::lf::list<T>::node* _current);
        virtual ~iterator() = default;

        // BASIC ITERATOR METHODS //
        auto operator=(const iterator& _rhs) -> iterator&;
        auto operator++() -> iterator&;
        auto operator*() const -> reference;
        // END / BASIC ITERATOR METHODS //

        // INPUT ITERATOR METHODS //
        auto operator++(int) -> iterator;
        auto operator-> () const -> pointer;
        auto operator==(iterator _rhs) const -> bool;
        auto operator!=(iterator _rhs) const -> bool;
        // END / INPUT ITERATOR METHODS //

        // OUTPUT ITERATOR METHODS //
        // reference operator*() const; <- already defined
        // iterator operator++(int); <- already defined
        // END / OUTPUT ITERATOR METHODS //

        // FORWARD ITERATOR METHODS //
        // Enable support for both input and output iterator <- already enabled
        // END / FORWARD ITERATOR METHODS //

        auto node() const -> zpt::lf::list<T>::node*;

      private:
        zpt::lf::list<T>::node const* __initial{ nullptr };
        zpt::lf::list<T>::node* __current{ nullptr };
    };

    list(long _max_threads, long _ptr_per_thread, long _spin_sleep_millis = 0);
    list(const zpt::lf::list<T>& _rhs);
    list(zpt::lf::list<T>&& _rhs);
    virtual ~list();

    auto operator=(const zpt::lf::list<T>& _rhs) -> zpt::lf::list<T>&;
    auto operator=(zpt::lf::list<T>&& _rhs) -> zpt::lf::list<T>&;

    auto front() const -> T;
    auto back() const -> T;

    auto head() const -> zpt::lf::list<T>::node*;
    auto tail() const -> zpt::lf::list<T>::node*;

    auto push(T value) -> zpt::lf::list<T>&;
    auto pop() -> T;
    auto erase(T _to_remove) -> zpt::lf::list<T>::iterator;
    auto erase(zpt::lf::list<T>::iterator _to_remove) -> zpt::lf::list<T>::iterator;

    auto find(T value) const -> zpt::lf::list<T>::iterator;
    auto begin() const -> zpt::lf::list<T>::iterator;
    auto end() const -> zpt::lf::list<T>::iterator;

    auto clear() -> zpt::lf::list<T>&;
    auto get_thread_dangling_count() const -> size_t;

    std::string to_string() const __attribute__((noinline));
    operator std::string();

    friend auto operator<<(std::ostream& _out, zpt::lf::list<T>& _in) -> std::ostream& {
        _out << "* zpt::lf::list(" << std::hex << &_in << "):" << std::dec << std::endl
             << std::flush;
        size_t _count = 0;
        _out << "[" << std::flush;
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
        _out << "]" << std::flush;
        _out << std::endl << "   (" << _count << " elements)" << std::flush;
        _out << std::endl << std::endl << _in.__hptr << std::flush;
        return _out;
    }

  private:
    std::atomic<zpt::lf::list<T>::node*> __head{ nullptr };
    std::atomic<zpt::lf::list<T>::node*> __tail{ nullptr };
    zpt::lf::hptr_domain<zpt::lf::list<T>::node> __hptr;
    long __spin_sleep;
};

template<typename T>
zpt::lf::list<T>::list(long _max_threads, long _ptr_per_thread, long _spin_sleep_millis)
  : __hptr{ _max_threads, _ptr_per_thread }
  , __spin_sleep{ _spin_sleep_millis } {
    auto _initial = new zpt::lf::list<T>::node();
    this->__head.store(_initial);
    this->__tail.store(_initial);
}

template<typename T>
zpt::lf::list<T>::~list() {}

template<typename T>
auto
zpt::lf::list<T>::front() const -> T {
    return this->head()->__value;
}

template<typename T>
auto
zpt::lf::list<T>::back() const -> T {
    return this->tail()->__value;
}

template<typename T>
auto
zpt::lf::list<T>::head() const -> zpt::lf::list<T>::node* {
    zpt::lf::list<T>::node* _front = this->__head.load();
    if (_front != nullptr && !_front->__is_null.load()) {
        return _front;
    }
    throw zpt::NoMoreElementsException("there is no element in the back");
}

template<typename T>
auto
zpt::lf::list<T>::tail() const -> zpt::lf::list<T>::node* {
    zpt::lf::list<T>::node* _back = this->__tail.load();
    if (_back != nullptr && !_back->__is_null.load()) {
        return _back;
    }
    throw zpt::NoMoreElementsException("there is no element in the back");
}

template<typename T>
auto
zpt::lf::list<T>::push(T _value) -> zpt::lf::list<T>& {
    zpt::lf::list<T>::node* _new{ new zpt::lf::list<T>::node{} };
    typename zpt::lf::hptr_domain<node>::guard _new_sentry{ _new, this->__hptr };

    do {
        zpt::lf::list<T>::node* _tail = this->__tail.load();
        zpt::lf::list<T>::node* _null{ nullptr };
        typename zpt::lf::hptr_domain<node>::guard _tail_sentry{ _tail, this->__hptr };

        if (_tail->__next.compare_exchange_strong(_null, _new)) {
            _new->__prev.store(_tail);
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
zpt::lf::list<T>::pop() -> T {
    do {
        zpt::lf::list<T>::node* _head = this->__head.load();
        typename zpt::lf::hptr_domain<node>::guard _head_sentry{ _head, this->__hptr };

        if (_head->__is_null.load()) {
            break;
        }
        else {
            zpt::lf::list<T>::node* _next = _head->__next.load();
            zpt::lf::list<T>::node* _null{ nullptr };
            typename zpt::lf::hptr_domain<node>::guard _next_sentry{ _next, this->__hptr };

            if (this->__head.compare_exchange_strong(_head, _next)) {
                _next->__prev.store(_null);
                T _value = _head->__value;
                _head->__is_null.store(true);
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
zpt::lf::list<T>::erase(T _to_remove) -> zpt::lf::list<T>::iterator {
    auto _it = this->find(_to_remove);
    if (_it != this->end()) {
        _it = this->erase(_it);
    }
    return _it;
}

template<typename T>
auto
zpt::lf::list<T>::erase(zpt::lf::list<T>::iterator _to_remove) -> zpt::lf::list<T>::iterator {
    zpt::lf::list<T>::node* _node = _to_remove.node();
    typename zpt::lf::hptr_domain<node>::guard _node_sentry{ _node, this->__hptr };

    if (_to_remove == this->end() || _node->__is_null.load()) {
        return this->end();
    }
    _node->__is_null.store(true);

    do {
        bool _replaced{ false };
        zpt::lf::list<T>::node* _prev = _node->__prev.load();
        zpt::lf::list<T>::node* _next = _node->__next.load();
        typename zpt::lf::hptr_domain<node>::guard _next_sentry{ _next, this->__hptr };
        typename zpt::lf::hptr_domain<node>::guard _prev_sentry{ _prev, this->__hptr };

        if (!_prev->__is_processing.compare_exchange_strong(false, true) ||
            !_next->__is_processing.compare_exchange_strong(false, true)) {
            continue;
        }

        if (_prev == nullptr) {
            if (this->__head.compare_exchange_strong(_node, _next)) {
                _replaced = true;
            }
        }
        else {
            if (_prev->__next.compare_exchange_strong(_node, _next)) {
                _replaced = true;
            }
        }

        if (_replaced) {
            if (_next != nullptr) {
                _next->__prev.store(_prev);
            }
            _node->__is_null.store(true);
            _node_sentry.retire();
            return zpt::lf::list<T>::iterator{ _next };
        }
    } while (true);

    return this->end(); // never reached
}

template<typename T>
auto
zpt::lf::list<T>::find(T _to_find) const -> zpt::lf::list<T>::iterator {
    for (auto _it = this->begin(); _it != this->end(); ++_it) {
        if (_it.node()->__is_null.load()) {
            continue;
        }
        if (*_it == _to_find) {
            return _it;
        }
    }
    return this->end();
}

template<typename T>
auto
zpt::lf::list<T>::begin() const -> zpt::lf::list<T>::iterator {
    return zpt::lf::list<T>::iterator{ this->__head.load() };
}

template<typename T>
auto
zpt::lf::list<T>::end() const -> zpt::lf::list<T>::iterator {
    return zpt::lf::list<T>::iterator{ nullptr };
}

template<typename T>
auto
zpt::lf::list<T>::clear() -> zpt::lf::list<T>& {
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
zpt::lf::list<T>::get_thread_dangling_count() const -> size_t {
    return this->__hptr.get_thread_dangling_count();
}

template<typename T>
auto
zpt::lf::list<T>::to_string() const -> std::string {
    return static_cast<std::string>(*this);
}

template<typename T>
zpt::lf::list<T>::operator std::string() {
    std::ostringstream _oss;
    _oss << (*this) << std::flush;
    return _oss.str();
}

template<typename T>
zpt::lf::list<T>::node::node(T _value)
  : __value{ _value } {}

template<typename T>
zpt::lf::list<T>::iterator::iterator(zpt::lf::list<T>::node* _current)
  : __initial(_current)
  , __current(_current) {}

template<typename T>
typename zpt::lf::list<T>::iterator&
zpt::lf::list<T>::iterator::operator=(const iterator& _rhs) {
    this->__initial = _rhs.__initial;
    this->__current = _rhs.__current;
    return (*this);
}

template<typename T>
auto
zpt::lf::list<T>::iterator::operator++() -> zpt::lf::list<T>::iterator& {
    if (this->__current != nullptr) {
        this->__current = this->__current->__next.load();
    }
    return (*this);
}

template<typename T>
auto zpt::lf::list<T>::iterator::operator*() const -> zpt::lf::list<T>::iterator::reference {
    return this->__current->__value;
}

template<typename T>
typename zpt::lf::list<T>::iterator
zpt::lf::list<T>::iterator::operator++(int) {
    zpt::lf::list<T>::iterator _to_return = (*this);
    ++(*this);
    return _to_return;
}

template<typename T>
auto zpt::lf::list<T>::iterator::operator-> () const -> zpt::lf::list<T>::iterator::pointer {
    return this->__current->__value;
}

template<typename T>
auto
zpt::lf::list<T>::iterator::operator==(iterator _rhs) const -> bool {
    return this->__current == _rhs.__current;
}

template<typename T>
auto
zpt::lf::list<T>::iterator::operator!=(iterator _rhs) const -> bool {
    return !((*this) == _rhs);
}

template<typename T>
auto
zpt::lf::list<T>::iterator::node() const -> zpt::lf::list<T>::node* {
    return this->__current;
}

} // namespace lf
} // namespace zpt
