/*
  Zapata project <https://github.com/naazgull/zapata>
  Author: n@zgul <n@zgul.me>

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
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

#include <zapata/lf/hptr.h>
#include <zapata/exceptions/NoMoreElementsException.h>

namespace zpt {
namespace lf {
template<typename T, int M, int P>
class queue {
    static_assert(std::is_scalar<T>::value,
                  "Type `T` in `zpt::lf::queue<T, M, P>` must be of scalar type");

  public:
    class node {
      public:
        T __value;
        std::atomic<bool> __is_null{ true };
        std::atomic<zpt::lf::queue<T, M, P>::node*> __next{ nullptr };

        node() = default;
        node(T _value);
        virtual ~node() = default;

        friend auto operator<<(std::ostream& _out, zpt::lf::queue<T, M, P>::node& _in)
          -> std::ostream& {
            if constexpr (std::is_pointer<T>::value) {
                _out << "(" << std::hex << &_in << " -> " << *_in.__value << ")" << std::flush;
            }
            else {
                _out << "(" << std::hex << &_in << " -> " << std::dec << _in.__value << ")"
                     << std::flush;
            }
            _out << std::dec << std::flush;
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

        explicit iterator(zpt::lf::queue<T, M, P>::node* _current);
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

        auto node() const -> zpt::lf::queue<T, M, P>::node*;

      private:
        zpt::lf::queue<T, M, P>::node const* __initial{ nullptr };
        zpt::lf::queue<T, M, P>::node* __current{ nullptr };
    };

    queue();
    queue(const zpt::lf::queue<T, M, P>& _rhs);
    queue(zpt::lf::queue<T, M, P>&& _rhs);
    virtual ~queue();

    auto operator=(const zpt::lf::queue<T, M, P>& _rhs) -> zpt::lf::queue<T, M, P>&;
    auto operator=(zpt::lf::queue<T, M, P>&& _rhs) -> zpt::lf::queue<T, M, P>&;

    auto front() -> T;
    auto back() -> T;

    auto head() -> zpt::lf::queue<T, M, P>::node*;
    auto tail() -> zpt::lf::queue<T, M, P>::node*;

    auto push(T value) -> void;
    auto pop() -> T;

    auto begin() const -> zpt::lf::queue<T, M, P>::iterator;
    auto end() const -> zpt::lf::queue<T, M, P>::iterator;

    std::string to_string() __attribute__((noinline));
    operator std::string();

    friend auto operator<<(std::ostream& _out, zpt::lf::queue<T, M, P>& _in) -> std::ostream& {
        size_t _count = 0;
        _out << "#items -> [" << std::flush;
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
        _out << "]" << std::endl << "#head -> " << std::flush;
        try {
            auto _head = _in.head();
            _out << *_head << std::flush;
        }
        catch (zpt::NoMoreElementsException& e) {
            _out << "0x0" << std::flush;
        }
        _out << std::endl << "#tail -> " << std::flush;
        try {
            auto _tail = _in.tail();
            _out << *_tail << std::flush;
        }
        catch (zpt::NoMoreElementsException& e) {
            _out << "0x0" << std::flush;
        }
        _out << std::endl << " (" << _count << " elements)" << std::flush;
        return _out;
    }

  private:
    std::atomic<zpt::lf::queue<T, M, P>::node*> __head{ nullptr };
    std::atomic<zpt::lf::queue<T, M, P>::node*> __tail{ nullptr };
    // zpt::lf::hptr_domain<zpt::lf::queue<T, M, P>::node, M, P>& __hptr{
    //     zpt::lf::hptr_domain<zpt::lf::queue<T, M, P>::node, M, P>::get_instance()
    // };
};

template<typename T, int M, int P>
zpt::lf::queue<T, M, P>::queue() {
    auto _initial = new zpt::lf::queue<T, M, P>::node();
    this->__head.store(_initial);
    this->__tail.store(_initial);
}

template<typename T, int M, int P>
zpt::lf::queue<T, M, P>::~queue() {}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::front() -> T {
    return this->head()->__value;
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::back() -> T {
    return this->tail()->__value;
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::head() -> zpt::lf::queue<T, M, P>::node* {
    zpt::lf::queue<T, M, P>::node* _front = this->__head.load();
    if (_front != nullptr && !_front->__is_null.load()) {
        return _front;
    }
    throw NoMoreElementsException("there is no element in the back");
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::tail() -> zpt::lf::queue<T, M, P>::node* {
    zpt::lf::queue<T, M, P>::node* _back = this->__tail.load();
    if (_back != nullptr && !_back->__is_null.load()) {
        return _back;
    }
    throw NoMoreElementsException("there is no element in the back");
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::push(T _value) -> void {
    zpt::lf::queue<T, M, P>::node* _new{ new zpt::lf::queue<T, M, P>::node{} };

    do {
        zpt::lf::queue<T, M, P>::node* _tail = this->__tail.load();
        zpt::lf::queue<T, M, P>::node* _null{ nullptr };
        typename zpt::lf::hptr_domain<node, M, P>::guard _tail_sentry{ _tail };

        if (_tail->__next.compare_exchange_strong(_null, _new)) {
            _tail->__value = _value;
            _tail->__is_null.store(false);
            this->__tail.store(_new);
            return;
        }
    } while (true);
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::pop() -> T {
    do {
        zpt::lf::queue<T, M, P>::node* _head = this->__head.load();
        typename zpt::lf::hptr_domain<node, M, P>::guard _head_sentry{ _head };

        if (_head->__is_null.load()) {
            break;
        }
        else {
            zpt::lf::queue<T, M, P>::node* _next = _head->__next.load();
            typename zpt::lf::hptr_domain<node, M, P>::guard _next_sentry{ _next };

            if (this->__head.compare_exchange_strong(_head, _next)) {
                T _value = _head->__value;
                _head_sentry.retire();
                return _value;
            }
        }
    } while (true);
    throw NoMoreElementsException("no element to pop");
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::begin() const -> zpt::lf::queue<T, M, P>::iterator {
    return zpt::lf::queue<T, M, P>::iterator{ this->__head.load() };
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::end() const -> zpt::lf::queue<T, M, P>::iterator {
    return zpt::lf::queue<T, M, P>::iterator{ nullptr };
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::to_string() -> std::string {
    return static_cast<std::string>(*this);
}

template<typename T, int M, int P>
zpt::lf::queue<T, M, P>::operator std::string() {
    std::ostringstream _oss;
    _oss << (*this) << std::flush;
    return _oss.str();
}

template<typename T, int M, int P>
zpt::lf::queue<T, M, P>::node::node(T _value)
  : __value{ _value } {}

template<typename T, int M, int P>
zpt::lf::queue<T, M, P>::iterator::iterator(zpt::lf::queue<T, M, P>::node* _current)
  : __initial(_current)
  , __current(_current) {}

template<typename T, int M, int P>
typename zpt::lf::queue<T, M, P>::iterator&
zpt::lf::queue<T, M, P>::iterator::operator=(const iterator& _rhs) {
    this->__initial = _rhs.__initial;
    this->__current = _rhs.__current;
    return (*this);
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::iterator::operator++() -> zpt::lf::queue<T, M, P>::iterator& {
    if (this->__current != nullptr) {
        this->__current = this->__current->__next.load();
    }
    return (*this);
}

template<typename T, int M, int P>
auto zpt::lf::queue<T, M, P>::iterator::operator*() const
  -> zpt::lf::queue<T, M, P>::iterator::reference {
    return this->__current->__value;
}

template<typename T, int M, int P>
typename zpt::lf::queue<T, M, P>::iterator
zpt::lf::queue<T, M, P>::iterator::operator++(int) {
    zpt::lf::queue<T, M, P>::iterator _to_return = (*this);
    ++(*this);
    return _to_return;
}

template<typename T, int M, int P>
auto zpt::lf::queue<T, M, P>::iterator::operator-> () const
  -> zpt::lf::queue<T, M, P>::iterator::pointer {
    return this->__current->__value;
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::iterator::operator==(iterator _rhs) const -> bool {
    return this->__current == _rhs.__current;
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::iterator::operator!=(iterator _rhs) const -> bool {
    return !((*this) == _rhs);
}

template<typename T, int M, int P>
auto
zpt::lf::queue<T, M, P>::iterator::node() const -> zpt::lf::queue<T, M, P>::node* {
    return this->__current;
}

} // namespace lf
} // namespace zpt
