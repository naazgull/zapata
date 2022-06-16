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

#include <zapata/lockfree/hazard_ptr.h>
#include <zapata/locks/spin_lock.h>
#include <zapata/exceptions/NoMoreElementsException.h>

namespace zpt {
namespace lf {

template<typename T>
class bidirectional_node {
  public:
    T __value;
    std::atomic<bool> __is_null{ true };
    std::atomic<zpt::lf::bidirectional_node<T>*> __next{ nullptr };
    std::atomic<zpt::lf::bidirectional_node<T>*> __prev{ nullptr };

    bidirectional_node() = default;
    bidirectional_node(T _value);
    virtual ~bidirectional_node() = default;

    friend auto operator<<(std::ostream& _out, zpt::lf::bidirectional_node<T>& _in)
      -> std::ostream& {
        if constexpr (std::is_pointer<T>::value) { _out << *(_in.__value) << std::flush; }
        else { _out << _in.__value << std::flush; }
        _out << std::flush;
        return _out;
    }
};

template<typename T>
class list {
    static_assert(std::is_copy_constructible<T>::value,
                  "Type `T` in `zpt::lf::list<T>` must be of copy constuctible.");

  public:
    using size_type = size_t;
    using hazard_domain = zpt::lf::hazard_ptr<zpt::lf::bidirectional_node<T>>;

    class iterator {
      public:
        friend class zpt::lf::list<T>;

        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T;
        using reference = T;

        explicit iterator(zpt::lf::bidirectional_node<T>* _current, zpt::lf::list<T>& _target);
        iterator(iterator const& _rhs);
        iterator(iterator&& _rhs);
        virtual ~iterator();

        // BASIC ITERATOR METHODS //
        auto operator=(iterator const& _rhs) -> iterator&;
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

        auto node() const -> zpt::lf::bidirectional_node<T>*;
        auto release() -> iterator&;
        auto exclusivity() -> iterator&;
        auto shareability() -> iterator&;

      private:
        zpt::lf::bidirectional_node<T> const* __initial{ nullptr };
        zpt::lf::bidirectional_node<T>* __current{ nullptr };
        zpt::lf::list<T>& __target;
        std::unique_ptr<zpt::locks::spin_lock::guard> __shared_sentry;
    };
    friend class zpt::lf::list<T>::iterator;

    list(zpt::lf::list<T>::hazard_domain& _hazard_domain, long _spin_sleep_micros = -1);
    list(zpt::lf::list<T> const& _rhs) = delete;
    list(zpt::lf::list<T>&& _rhs) = delete;
    virtual ~list();

    auto operator=(zpt::lf::list<T> const& _rhs) -> zpt::lf::list<T>& = delete;
    auto operator=(zpt::lf::list<T>&& _rhs) -> zpt::lf::list<T>& = delete;

    auto front() const -> T;
    auto back() const -> T;

    auto head() const -> zpt::lf::bidirectional_node<T>*;
    auto tail() const -> zpt::lf::bidirectional_node<T>*;

    auto push(T value) -> zpt::lf::list<T>&;
    auto pop() -> T;
    template<typename F>
    auto erase(F _remove_if) -> zpt::lf::list<T>::iterator;
    auto erase(zpt::lf::list<T>::iterator& _to_remove) -> zpt::lf::list<T>::iterator;

    template<typename F>
    auto find_if(F _callback) -> zpt::lf::list<T>::iterator;
    auto at(size_t _idx) -> T;

    auto begin() -> zpt::lf::list<T>::iterator;
    auto end() -> zpt::lf::list<T>::iterator;

    auto clear() -> zpt::lf::list<T>&;
    auto get_thread_dangling_count() const -> size_t;

    auto to_string() const -> std::string;

    operator std::string();
    auto operator[](size_t _idx) -> T;

    friend auto operator<<(std::ostream& _out, zpt::lf::list<T>& _in) -> std::ostream& {
        _out << "* zpt::lf::list(" << std::hex << &_in << "):" << std::dec << std::endl
             << std::flush;
        size_t _count = 0;
        _out << "\t[" << std::flush;
        try {
            for (auto _it = _in.begin(); _it != _in.end(); ++_it) {
                if (_it.node()->__is_null.load()) continue;
                _out << (_count != 0 ? (_count % 5 == 0 ? "\n\t" : ", ") : "") << *_it.node()
                     << std::flush;
                _count++;
            }
        }
        catch (zpt::NoMoreElementsException const& e) {
            _count = 0;
        }
        _out << "]" << std::flush;
        _out << std::endl << "   (" << _count << " elements)" << std::flush;
        _out << std::endl << std::endl << _in.__hazard_domain << std::flush;
        return _out;
    }

  private:
    // alignas(128)
    std::atomic<zpt::lf::bidirectional_node<T>*> __head{ nullptr };
    // alignas(128)
    std::atomic<zpt::lf::bidirectional_node<T>*> __tail{ nullptr };
    zpt::lf::list<T>::hazard_domain& __hazard_domain;
    long __spin_sleep{ 0 };
    zpt::locks::spin_lock __access_lock;
};

template<typename T>
zpt::lf::bidirectional_node<T>::bidirectional_node(T _value)
  : __value{ _value } {}

template<typename T>
zpt::lf::list<T>::iterator::iterator(zpt::lf::bidirectional_node<T>* _current,
                                     zpt::lf::list<T>& _target)
  : __initial{ _current }
  , __current{ _current }
  , __target{ _target }
  , __shared_sentry{ std::make_unique<zpt::locks::spin_lock::guard>(_target.__access_lock, true) } {
    this->__target.__hazard_domain.acquire(this->__current);
}

template<typename T>
zpt::lf::list<T>::iterator::iterator(iterator const& _rhs)
  : __initial{ _rhs.__initial }
  , __current{ _rhs.__current }
  , __target{ _rhs.__target }
  , __shared_sentry{ std::make_unique<zpt::locks::spin_lock::guard>(_rhs.__target.__access_lock,
                                                                    true) } {
    this->__target.__hazard_domain.acquire(this->__current);
}

template<typename T>
zpt::lf::list<T>::iterator::iterator(iterator&& _rhs)
  : __initial{ _rhs.__initial }
  , __current{ _rhs.__current }
  , __target{ _rhs.__target }
  , __shared_sentry{ std::move(_rhs.__shared_sentry) } {
    this->__target.__hazard_domain.acquire(this->__current);
}

template<typename T>
zpt::lf::list<T>::iterator::~iterator() {
    this->__target.__hazard_domain.release(this->__current);
}

template<typename T>
auto
zpt::lf::list<T>::iterator::operator=(iterator const& _rhs) -> zpt::lf::list<T>::iterator& {
    this->__initial = _rhs.__initial;
    this->__current = _rhs.__current;
    this->__target = _rhs.__target;
    this->__shared_sentry =
      std::make_unique<zpt::locks::spin_lock::guard>(_rhs.__target.__access_lock, true);
    this->__target.__hazard_domain.acquire(this->__current);
    return (*this);
}

template<typename T>
auto
zpt::lf::list<T>::iterator::operator=(iterator&& _rhs) -> zpt::lf::list<T>::iterator& {
    this->__initial = _rhs.__initial;
    this->__current = _rhs.__current;
    this->__target = _rhs.__target;
    this->__shared_sentry = std::move(_rhs.__shared_entry);
    _rhs.__initial = nullptr;
    _rhs.__current = nullptr;
    this->__target.__hazard_domain.acquire(this->__current);
    return (*this);
}

template<typename T>
auto
zpt::lf::list<T>::iterator::operator++() -> zpt::lf::list<T>::iterator& {
    if (this->__current != nullptr) {
        this->__target.__hazard_domain.release(this->__current);
        this->__current = this->__current->__next.load();
        this->__target.__hazard_domain.acquire(this->__current);
    }
    return (*this);
}

template<typename T>
auto
zpt::lf::list<T>::iterator::operator*() -> zpt::lf::list<T>::iterator::reference {
    return this->__current->__value;
}

template<typename T>
auto
zpt::lf::list<T>::iterator::operator++(int) -> zpt::lf::list<T>::iterator {
    auto _to_return = (*this);
    ++(*this);
    return _to_return;
}

template<typename T>
auto
zpt::lf::list<T>::iterator::operator->() -> zpt::lf::list<T>::iterator::pointer {
    return this->__current->__value;
}

template<typename T>
auto
zpt::lf::list<T>::iterator::operator==(iterator const& _rhs) const -> bool {
    return this->__current == _rhs.__current;
}

template<typename T>
auto
zpt::lf::list<T>::iterator::operator!=(iterator const& _rhs) const -> bool {
    return !((*this) == _rhs);
}

template<typename T>
auto
zpt::lf::list<T>::iterator::node() const -> zpt::lf::bidirectional_node<T>* {
    return this->__current;
}

template<typename T>
auto
zpt::lf::list<T>::iterator::release() -> zpt::lf::list<T>::iterator& {
    this->__shared_sentry->release();
    return (*this);
}

template<typename T>
auto
zpt::lf::list<T>::iterator::exclusivity() -> zpt::lf::list<T>::iterator& {
    this->__shared_sentry->exclusivity();
    return (*this);
}

template<typename T>
auto
zpt::lf::list<T>::iterator::shareability() -> zpt::lf::list<T>::iterator& {
    this->__shared_sentry->shareability();
    return (*this);
}

template<typename T>
zpt::lf::list<T>::list(zpt::lf::list<T>::hazard_domain& _hazard_domain, long _spin_sleep_micros)
  : __hazard_domain{ _hazard_domain }
  , __spin_sleep{ _spin_sleep_micros } {
    auto _initial = new zpt::lf::bidirectional_node<T>();
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
zpt::lf::list<T>::head() const -> zpt::lf::bidirectional_node<T>* {
    zpt::locks::spin_lock::guard _shared_sentry{ this->__access_lock,
                                                 zpt::locks::spin_lock::shared };
    auto _front = this->__head.load();
    if (_front != nullptr && !_front->__is_null.load()) { return _front; }
    throw zpt::NoMoreElementsException("there is no element in the back");
}

template<typename T>
auto
zpt::lf::list<T>::tail() const -> zpt::lf::bidirectional_node<T>* {
    zpt::locks::spin_lock::guard _shared_sentry{ this->__access_lock,
                                                 zpt::locks::spin_lock::shared };
    auto _back = this->__tail.load();
    if (_back != nullptr && !_back->__is_null.load()) { return _back; }
    throw zpt::NoMoreElementsException("there is no element in the back");
}

template<typename T>
auto
zpt::lf::list<T>::push(T _value) -> zpt::lf::list<T>& {
    zpt::locks::spin_lock::guard _shared_sentry{ this->__access_lock,
                                                 zpt::locks::spin_lock::shared };
    auto _new{ new zpt::lf::bidirectional_node<T>{} };
    typename zpt::lf::list<T>::hazard_domain::guard _new_sentry{ _new, this->__hazard_domain };

    do {
        auto _tail = this->__tail.load(std::memory_order_acquire);
        zpt::lf::bidirectional_node<T>* _null{ nullptr };
        typename zpt::lf::list<T>::hazard_domain::guard _tail_sentry{ _tail,
                                                                      this->__hazard_domain };

        if (_tail->__next.compare_exchange_strong(_null, _new)) {
            _new->__prev.store(_tail);
            _tail->__value = _value;
            _tail->__is_null.store(false);
            this->__tail.store(_new, std::memory_order_release);
            return (*this);
        }
        if (this->__spin_sleep <= 0) { std::this_thread::yield(); }
        else if (this->__spin_sleep != 0) {
            std::this_thread::sleep_for(
              std::chrono::duration<int, std::micro>{ this->__spin_sleep });
        }
    } while (true);

    return (*this); // never reached
}

template<typename T>
auto
zpt::lf::list<T>::pop() -> T {
    zpt::locks::spin_lock::guard _shared_sentry{ this->__access_lock,
                                                 zpt::locks::spin_lock::shared };
    do {
        zpt::lf::bidirectional_node<T>* _head = this->__head.load(std::memory_order_acquire);
        typename zpt::lf::list<T>::hazard_domain::guard _head_sentry{ _head,
                                                                      this->__hazard_domain };

        if (_head->__is_null.load()) { break; }
        else {
            auto _next = _head->__next.load();
            zpt::lf::bidirectional_node<T>* _null{ nullptr };
            typename zpt::lf::list<T>::hazard_domain::guard _next_sentry{ _next,
                                                                          this->__hazard_domain };

            if (this->__head.compare_exchange_strong(_head, _next, std::memory_order_release)) {
                _next->__prev.store(_null);
                T _value = _head->__value;
                _head->__is_null.store(true);
                _head_sentry.retire();
                return _value;
            }
        }
        if (this->__spin_sleep < 0) { std::this_thread::yield(); }
        else if (this->__spin_sleep != 0) {
            std::this_thread::sleep_for(
              std::chrono::duration<int, std::micro>{ this->__spin_sleep });
        }
    } while (true);
    throw zpt::NoMoreElementsException("no element to pop");
}

template<typename T>
template<typename F>
auto
zpt::lf::list<T>::erase(F _remove_if) -> zpt::lf::list<T>::iterator {
    zpt::lf::bidirectional_node<T>* _it{ nullptr };
    {
        zpt::locks::spin_lock::guard _exclusive_sentry{ this->__access_lock,
                                                        zpt::locks::spin_lock::exclusive };
        auto _node = this->__head.load();
        while (_node != nullptr) {
            auto _next = _node->__next.load();
            auto _prev = _node->__prev.load();

            if (!_node->__is_null.load() && _remove_if(_node->__value)) {
                if (_prev == nullptr) { this->__head.store(_next); }
                else { _prev->__next.store(_next); }
                _next->__prev.store(_prev);
                _it = _next;
                this->__hazard_domain.retire(_node);
                break;
            }
            _node = _next;
        }
    }
    return zpt::lf::list<T>::iterator{ _it, *this };
}

template<typename T>
auto
zpt::lf::list<T>::erase(zpt::lf::list<T>::iterator& _to_remove) -> zpt::lf::list<T>::iterator {
    _to_remove.exclusivity();

    auto _node = _to_remove.node();
    auto _next = _node->__next.load();
    if (_node != nullptr && !_node->__is_null.load()) {
        auto _prev = _node->__prev.load();
        if (_prev == nullptr) { this->__head.store(_next); }
        else { _node->__prev.store(_next); }
        _next->__prev.store(_prev);
    }

    _to_remove.shareability();

    return zpt::lf::list<T>::iterator{ _next, *this };
}

template<typename T>
template<typename F>
auto
zpt::lf::list<T>::find_if(F _callback) -> zpt::lf::list<T>::iterator {
    zpt::locks::spin_lock::guard _shared_sentry{ this->__access_lock,
                                                 zpt::locks::spin_lock::shared };
    for (auto _it = this->begin(); _it != this->end(); ++_it) {
        if (!_it.node()->__is_null.load() && _callback(_it.node()->__value)) {
            return zpt::lf::list<T>::iterator{ _it.node(), *this };
        }
    }
    return zpt::lf::list<T>::iterator{ nullptr, *this };
}

template<typename T>
auto
zpt::lf::list<T>::at(size_t _idx) -> T {
    zpt::locks::spin_lock::guard _shared_sentry{ this->__access_lock,
                                                 zpt::locks::spin_lock::shared };
    size_t _k{ 0 };
    auto _node = this->__head.load();
    while (_node != nullptr && _k != _idx) {
        auto _next = _node->__next.load();
        typename zpt::lf::list<T>::hazard_domain::guard _next_sentry{ _next,
                                                                      this->__hazard_domain };
        _node = _next;
        ++_k;
    }
    expect(_k == _idx, "couldn't find the provided index", 500, 0);
    typename zpt::lf::list<T>::hazard_domain::guard _node_sentry{ _node, this->__hazard_domain };
    return _node->__value;
}

template<typename T>
auto
zpt::lf::list<T>::begin() -> zpt::lf::list<T>::iterator {
    return zpt::lf::list<T>::iterator{ this->__head.load(), *this };
}

template<typename T>
auto
zpt::lf::list<T>::end() -> zpt::lf::list<T>::iterator {
    return zpt::lf::list<T>::iterator{ nullptr, *this };
}

template<typename T>
auto
zpt::lf::list<T>::clear() -> zpt::lf::list<T>& {
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
zpt::lf::list<T>::get_thread_dangling_count() const -> size_t {
    return this->__hazard_domain.get_thread_dangling_count();
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
auto
zpt::lf::list<T>::operator[](size_t _idx) -> T {
    return this->at(_idx);
}

} // namespace lf
} // namespace zpt
