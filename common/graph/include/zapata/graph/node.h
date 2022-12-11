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

#include <zapata/locks/spin_lock.h>
#include <zapata/log/log.h>

#include <memory>
#include <vector>
#include <iostream>

namespace zpt {
namespace tree {
template<typename T, typename P, typename C>
class node {
  public:
    node() = default;
    node(node const& _rhs);
    node(node&& _rhs);
    virtual ~node() = default;

    auto operator=(node const& _rhs) -> node&;
    auto operator=(node&& _rhs) -> node&;

    auto clear() -> node&;

    template<typename I, typename M, typename... Types>
    auto eval(I _sequence, I _end, M _value_to_match, Types... _callback_args) -> bool;
    template<typename I>
    auto merge(I _sequence, I _end, P _path, C _callback) -> bool;

    auto to_string(uint _n_tabs = 0) const -> std::string;

    friend auto operator<<(std::ostream& _out, zpt::tree::node<T, P, C> const& _in) -> std::ostream& {
        _out << _in.to_string();
        return _out;
    }

  private:
    std::vector<node> __children;
    T __value;
    P __path;
    std::vector<C> __callbacks;
};
} // namespace tree
} // namespace zpt

template<typename T, typename P, typename C>
zpt::tree::node<T, P, C>::node(zpt::tree::node<T, P, C> const& _rhs)
  : __children{ _rhs.__children }
  , __value{ _rhs.__value }
  , __path{ _rhs.__path }
  , __callbacks{ _rhs.__callbacks } {}

template<typename T, typename P, typename C>
zpt::tree::node<T, P, C>::node(zpt::tree::node<T, P, C>&& _rhs)
  : __children{ std::move(_rhs.__children) }
  , __value{ std::move(_rhs.__value) }
  , __path{ std::move(_rhs.__path) }
  , __callbacks{ std::move(_rhs.__callbacks) } {
    _rhs.clear();
}

template<typename T, typename P, typename C>
auto
zpt::tree::node<T, P, C>::operator=(zpt::tree::node<T, P, C> const& _rhs) -> zpt::tree::node<T, P, C>& {
    this->__children = _rhs.__children;
    this->__value = _rhs.__value;
    this->__path = _rhs.__path;
    this->__callbacks = _rhs.__callbacks;
    return (*this);
}

template<typename T, typename P, typename C>
auto
zpt::tree::node<T, P, C>::operator=(zpt::tree::node<T, P, C>&& _rhs) -> zpt::tree::node<T, P, C>& {
    this->__children = std::move(_rhs.__children);
    this->__value = std::move(_rhs.__value);
    this->__path = std::move(_rhs.__path);
    this->__callbacks = std::move(_rhs.__callbacks);
    _rhs.clear();
    return (*this);
}

template<typename T, typename P, typename C>
auto
zpt::tree::node<T, P, C>::clear() -> zpt::tree::node<T, P, C>& {
    this->__children.clear();
    this->__callbacks.clear();
    return (*this);
}

template<typename T, typename P, typename C>
template<typename I, typename M, typename... Types>
auto
zpt::tree::node<T, P, C>::eval(I _sequence, I _end, M _value_to_match, Types... _callback_args) -> bool {
    if (_sequence == _end) { return false; }

    auto _return{ false };
    if (this->__value != (*_sequence)) { return false; }

    if (this->__path == _value_to_match) {
        for (auto& _call : this->__callbacks) { _call(_callback_args...); }
        _return = true;
    }

    auto _next = _sequence;
    ++_next;
    for (auto& _child : this->__children) {
        _return = _child.eval(_next, _end, _value_to_match, _callback_args...) || _return;
    }
    return _return;
}

template<typename T, typename P, typename C>
template<typename I>
auto
zpt::tree::node<T, P, C>::merge(I _sequence, I _end, P _path, C _callback) -> bool {
    auto _is_null{ this->__value == nullptr };
    if (_is_null) { this->__value = (*_sequence); }

    if (_is_null || this->__value == (*_sequence)) {
        auto _next = _sequence;
        ++_next;
        if (_next == _end) {
            this->__callbacks.push_back(_callback);
            this->__path = _path;
            return true;
        }

        for (auto& _child : this->__children) {
            if (_child.merge(_next, _end, _path, _callback)) { return true; }
        }

        this->__children.emplace_back();
        this->__children.back().merge(_next, _end, _path, _callback);
        return true;
    }
    return false;
}

template<typename T, typename P, typename C>
auto
zpt::tree::node<T, P, C>::to_string(uint _n_tabs) const -> std::string {
    std::ostringstream _oss;
    _oss << std::string(static_cast<size_t>(_n_tabs), '\t');
    if (_n_tabs != 0) { _oss << "|"; }
    _oss << "_ " << this->__value << std::endl << std::flush;
    for (auto& _child : this->__children) { _oss << _child.to_string(_n_tabs + 1); }
    return _oss.str();
}
