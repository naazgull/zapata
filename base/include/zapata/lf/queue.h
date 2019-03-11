#pragma once

#include <memory>
#include <iterator>
#include <atomic>
#include <math.h>
#include <unistd.h>
#include <iostream>
#include <thread>

#include <zapata/lf/hptr.h>

namespace zpt {
namespace lf {
template <typename T> class queue {
  public:
    using size_type = size_t;

    queue(int _n_threads, int _n_hp_per_thread);
    queue(const queue<T>& _rhs);
    queue(queue<T>&& _rhs);
    virtual ~queue();

    auto operator=(const queue<T>& _rhs) -> queue<T>&;
    auto operator=(queue<T>&& _rhs) -> queue<T>&;

    auto front() -> T;
    auto back() -> T;

    auto push(T value) -> void;
    auto pop(bool _blocking = true) -> T;

  protected:
    class pointer;

    class node {
      public:
        T __value;
        std::atomic<pointer*> __next{nullptr};

        node() = default;
        node(T _value, pointer* _next);
        virtual ~node();
    };

    class pointer {
      public:
        node* __ptr{nullptr};

        pointer() = default;
        pointer(node* _ptr);
        virtual ~pointer() = default;
    };

  public:
    class iterator {
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;

        explicit iterator(zpt::lf::queue<T>::pointer* _current);
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

      private:
        zpt::lf::queue<T>::pointer const* __initial;
        zpt::lf::queue<T>::pointer* __current;
    };

    auto begin() const -> iterator;
    auto end() const -> iterator;

    friend auto operator<<(std::ostream& _out, zpt::lf::queue<T>& _in) -> std::ostream& {
        size_t _count = 0;
        for (auto _it : _in) {
            _out << "[" << &(*_it) << ", " << *_it << "]" << std::endl << std::flush;
            _count++;
        }
        _out << " (" << _count << " elements)" << std::flush;
        return _out;
    }

  private:
    std::atomic<pointer*> __head{nullptr};
    std::atomic<pointer*> __tail{nullptr};
    zpt::lf::hptr_domain<node>* __hptr{nullptr};
};

template <typename T>
zpt::lf::queue<T>::queue(int _n_threads, int _n_hp_per_thread)
    : __head{nullptr}, __tail{nullptr},
      __hptr{&zpt::lf::hptr_domain<zpt::lf::queue<T>::node>::get_instance(_n_threads,
                                                                          _n_hp_per_thread)} {
    auto _first = new zpt::lf::queue<T>::pointer{};
    this->__head.store(_first, std::memory_order_release);
    this->__tail.store(_first, std::memory_order_release);
    zpt::lf::queue<T>::node* _new_node = new zpt::lf::queue<T>::node{};
    this->__head.load(std::memory_order_relaxed)->__ptr =
        this->__tail.load(std::memory_order_relaxed)->__ptr = _new_node;
}

template <typename T> zpt::lf::queue<T>::~queue() {
    delete this->__head.load(std::memory_order_acquire);
    this->__head.store(nullptr, std::memory_order_release);
    this->__tail.store(nullptr, std::memory_order_release);
}

template <typename T> auto zpt::lf::queue<T>::front() -> T {
    node* _front = this->__head.load()->__next.load();
    return (_front != nullptr ? _front->__value : nullptr);
}

template <typename T> auto zpt::lf::queue<T>::back() -> T {
    node* _back = this->__tail.load()->__next.load();
    return (_back != nullptr ? _back->__value : nullptr);
}

template <typename T> auto zpt::lf::queue<T>::push(T _value) -> void {
    zpt::lf::queue<T>::node* _new_node = new zpt::lf::queue<T>::node{};
    _new_node->__value = _value;

    do {
        auto _tail = this->__tail.load(std::memory_order_acquire);
        auto _next = _tail->__ptr->__next.load(std::memory_order_acquire);
        if (this->__tail.compare_exchange_strong(_tail, _tail, std::memory_order_acq_rel)) {
            if (_next == nullptr || _next->__ptr == nullptr) {
                zpt::lf::queue<T>::pointer* _new_ptr = new zpt::lf::queue<T>::pointer{_new_node};

                if (_tail->__ptr->__next.compare_exchange_strong(
                        _next, _new_ptr, std::memory_order_acq_rel)) {
                    this->__tail.compare_exchange_strong(
                        _tail, _new_ptr, std::memory_order_acq_rel);
                    break;
                } else {
                    delete _new_ptr;
                }
            } else {
                zpt::lf::queue<T>::pointer* _new_ptr = new zpt::lf::queue<T>::pointer{_next->__ptr};
                this->__tail.compare_exchange_strong(_tail, _new_ptr, std::memory_order_acq_rel);
            }
        }
    } while (true);
}

template <typename T> auto zpt::lf::queue<T>::pop(bool _blocking) -> T {
    do {
        auto _tail = this->__tail.load(std::memory_order_acquire);
        auto _head = this->__head.load(std::memory_order_acquire);

        auto _next = _head->__ptr->__next.load(std::memory_order_acquire);
        if (_next == nullptr || _next->__ptr == nullptr) {
            return static_cast<T>(0);
        }

        if (this->__head.compare_exchange_strong(_head, _head, std::memory_order_acq_rel)) {
            if (_next->__ptr == _tail->__ptr) {
                return static_cast<T>(0);
            } else {
                T _to_return = _next->__ptr->__value;
                if (_head->__ptr->__next.compare_exchange_strong(
                        _next, _next->__ptr->__next.load(), std::memory_order_acq_rel))
                    return _to_return;
            }
        }
    } while (true);

    return static_cast<T>(0);
}

template <typename T> auto zpt::lf::queue<T>::begin() const -> zpt::lf::queue<T>::iterator {
    return zpt::lf::queue<T>::iterator{this->__head.load()->__ptr->__next.load()};
}

template <typename T> auto zpt::lf::queue<T>::end() const -> zpt::lf::queue<T>::iterator {
    return zpt::lf::queue<T>::iterator{nullptr};
}

template <typename T> zpt::lf::queue<T>::node::~node() { delete this->__next.load(); }

template <typename T>
zpt::lf::queue<T>::pointer::pointer(zpt::lf::queue<T>::node* _ptr) : __ptr{_ptr} {}

template <typename T>
zpt::lf::queue<T>::iterator::iterator(zpt::lf::queue<T>::pointer* _current)
    : __initial(_current), __current(_current) {}

template <typename T>
typename zpt::lf::queue<T>::iterator& zpt::lf::queue<T>::iterator::operator=(const iterator& _rhs) {
    this->__initial = _rhs.__initial;
    this->__current = _rhs.__current;
    return (*this);
}

template <typename T>
auto zpt::lf::queue<T>::iterator::operator++() -> zpt::lf::queue<T>::iterator& {
    if (this->__current != nullptr) {
        auto _next = this->__current->__ptr->__next.load(std::memory_order_release);
        this->__current = _next != nullptr ? _next : nullptr;
    }
    return (*this);
}

template <typename T> auto zpt::lf::queue<T>::iterator::operator*() const -> T& {
    return this->__current->__ptr->__value;
}

template <typename T>
typename zpt::lf::queue<T>::iterator zpt::lf::queue<T>::iterator::operator++(int) {
    zpt::lf::queue<T>::iterator _to_return = (*this);
    ++(*this);
    return _to_return;
}

template <typename T> auto zpt::lf::queue<T>::iterator::operator-> () const -> T* {
    return &this->__current->__ptr->__value;
}

template <typename T> auto zpt::lf::queue<T>::iterator::operator==(iterator _rhs) const -> bool {
    return this->__current == _rhs.__current;
}

template <typename T> auto zpt::lf::queue<T>::iterator::operator!=(iterator _rhs) const -> bool {
    return !((*this) == _rhs);
}

} // namespace lf
} // namespaace zpt
