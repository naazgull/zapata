#pragma once

#include <memory>
#include <vector>
#include <iostream>

namespace zpt {
namespace graph {
template<typename T, typename P, typename C>
class node {
  public:
    node() = default;
    virtual ~node() = default;

    template<typename I, typename M, typename... Types>
    auto eval(I _sequence, I _end, M _value_to_match, Types... _callback_args) -> bool;
    template<typename I>
    auto merge(I _sequence, I _end, P _path, C _callback) -> bool;

    auto to_string(uint _n_tabs = 0) -> std::string;

    friend auto operator<<(std::ostream& _out, zpt::graph::node<T, P, C>& _in) -> std::ostream& {
        _out << _in.to_string();
        return _out;
    }

  private:
    std::vector<node> __children;
    T __value;
    P __path;
    std::vector<C> __callbacks;
};
} // namespace graph
} // namespace zpt

template<typename T, typename P, typename C>
template<typename I, typename M, typename... Types>
auto
zpt::graph::node<T, P, C>::eval(I _sequence, I _end, M _value_to_match, Types... _callback_args)
  -> bool {
    if (_sequence == _end) {
        return false;
    }
    bool _return{ false };
    if (this->__value == (*_sequence) && this->__path == _value_to_match) {
        for (auto _call : this->__callbacks) {
            _call(_callback_args...);
        }
        _return = true;
    }

    auto _next = _sequence;
    ++_next;
    for (auto _child : this->__children) {
        _return = _child.eval(_next, _end, _value_to_match, _callback_args...) || _return;
    }
    return _return;
}

template<typename T, typename P, typename C>
template<typename I>
auto
zpt::graph::node<T, P, C>::merge(I _sequence, I _end, P _path, C _callback) -> bool {
    bool _is_null{ this->__value == nullptr };
    if (_is_null) {
        this->__value = (*_sequence);
    }

    if (_is_null || this->__value == (*_sequence)) {
        auto _next = _sequence;
        ++_next;
        if (_next == _end) {
            this->__callbacks.push_back(_callback);
            this->__path = _path;
            return true;
        }

        for (auto& _child : this->__children) {
            if (_child.merge(_next, _end, _path, _callback)) {
                return true;
            }
        }

        this->__children.emplace_back();
        this->__children.back().merge(_next, _end, _path, _callback);
        return true;
    }
    return false;
}

template<typename T, typename P, typename C>
auto
zpt::graph::node<T, P, C>::to_string(uint _n_tabs) -> std::string {
    std::string _to_return = std::string(_n_tabs, '\t');
    if (_n_tabs != 0) {
        _to_return.insert(_to_return.length(), "|");
    }
    _to_return.insert(_to_return.length(), "_ ");
    _to_return.insert(_to_return.length(), this->__value);
    _to_return.insert(_to_return.length(), "\n");
    for (auto _child : this->__children) {
        std::string _child_str = _child.to_string(_n_tabs + 1);
        _to_return.insert(_to_return.length(), _child_str);
    }
    return _to_return;
}
