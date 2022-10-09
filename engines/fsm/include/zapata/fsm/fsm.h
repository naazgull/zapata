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

#include <zapata/events.h>
#include <zapata/json.h>

namespace zpt {
namespace fsm {

#if __GNUC__ >= 10
template<typename C, typename S>
concept machine_override = requires(C _c, S _s) {
    { _c.verify_allowed_transition(_s, _s) };
    { _c.verify_transition(_s) };
};
#endif

template<typename D, typename I>
using payload = std::tuple<D, I>;

template<typename C, typename S, typename D, typename I>
class machine;

template<typename C, typename S, typename D, typename I>
class machine : public zpt::events::dispatcher<zpt::fsm::machine<C, S, D, I>, S, zpt::fsm::payload<D, I>> {
  public:
    using callback = std::function<S(S&, D&, I const&)>;
    using callback_list = std::vector<callback>;
    using state_list = std::map<S, bool>;
    using error_callback = std::function<bool(S const& _event,
                                              D& _content,
                                              I const& _id,
                                              const char* _what,
                                              const char* _description,
                                              const char* _backtrace,
                                              int _error,
                                              int status)>;

    machine(long _processor_threads, long _max_threads, zpt::json _config);
    virtual ~machine();

    auto add_allowed_transitions(zpt::json _states) -> machine&;

    auto begin(D _content, I _id) -> machine&;
    auto resume(I _id, S _state) -> machine&;
    auto add_transition(S _event, callback _callback) -> machine&;
    auto set_error_callback(error_callback _error_callback) -> machine&;

    auto trapped(S _current_state, zpt::fsm::payload<D, I> _content) -> void;
    auto listen_to(S _event, callback _callback) -> void;
    auto report_error(S const& _event,
                      zpt::fsm::payload<D, I>& _content,
                      const char* _what,
                      const char* _description = nullptr,
                      const char* _backtrace = nullptr,
                      int _error = -1,
                      int _status = 500) -> bool;

    auto start_threads() -> machine&;

    auto to_string() const -> std::string;
    friend auto operator<<(std::ostream& _out, machine& _in) -> std::ostream& {
        _out << _in.to_string() << std::flush;
        return _out;
    }

  protected:
    auto set_states(zpt::json _states) -> machine&;

  private:
    zpt::locks::spin_lock __stalled_lock;
    std::map<S, callback_list> __callbacks;
    std::map<S, state_list> __transitions;
    std::map<I, std::tuple<S, D>> __stalled;
    S __undefined;
    S __begin;
    S __end;
    S __pause;
    error_callback __error_callback{ nullptr };
    long __processor_threads{ 0 };

    auto coalesce(S _current, S _potential) const -> S;
    auto pause(S _current, zpt::fsm::payload<D, I> _content) -> void;
};

template<typename C, typename S, typename D, typename I>
zpt::fsm::machine<C, S, D, I>::machine(long _processor_threads, long _max_threads, zpt::json _config)
  : zpt::events::dispatcher<zpt::fsm::machine<C, S, D, I>, S, zpt::fsm::payload<D, I>>{ _max_threads }
  , __processor_threads{ _processor_threads } {
    this->set_states(_config);
    if (_config["transitions"]->ok()) { this->add_allowed_transitions(_config["transitions"]); }
}

template<typename C, typename S, typename D, typename I>
zpt::fsm::machine<C, S, D, I>::~machine() {
    this->shutdown();
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::set_states(zpt::json _states) -> machine& {
    this->__undefined = static_cast<S>(_states["undefined"]);
    this->__begin = static_cast<S>(_states["begin"]);
    this->__end = static_cast<S>(_states["end"]);
    this->__pause = static_cast<S>(_states["pause"]);
    return (*this);
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::add_allowed_transitions(zpt::json _states) -> machine& {
    expect(_states->ok() && _states->is_array(), "states JSON configuration must be a JSON array");

    for (auto [_, __, _potential] : _states) {
        expect(_potential[1]->is_array(), "each state allowed states member value must be an array");
        for (auto [_, __, _target] : _potential[1]) {
            auto _from = static_cast<S>(_potential[0]);
            auto _to = static_cast<S>(_target);
            static_cast<C*>(this)->verify_allowed_transition(_from, _to);
            this->__transitions[_from][_to] = true;
        }
    }
    return (*this);
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::begin(D _content, I _id) -> machine& {
    this->trigger(this->__begin, zpt::fsm::payload<D, I>{ std::make_tuple(_content, _id) });
    return (*this);
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::resume(I _id, S _state) -> machine& {
    S _stalled_state;
    S _next_state;
    D _data;
    {
        zpt::locks::spin_lock::guard _shared_sentry{ this->__stalled_lock, zpt::locks::spin_lock::exclusive };
        auto _found = this->__stalled.find(_id);
        expect(_found != this->__stalled.end(), "no such payload identified by " << _id);
        _stalled_state = std::get<0>(_found->second);
        _data = std::get<1>(_found->second);
        _next_state = this->coalesce(_stalled_state, _state);
        this->__stalled.erase(_found);
    }
    this->trigger(_next_state, zpt::fsm::payload<D, I>{ _data, _id });
    return (*this);
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::add_transition(S _source, callback _callback) -> machine& {
    static_cast<C*>(this)->verify_transition(_source);
    this->listen(_source, _callback);
    return (*this);
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::set_error_callback(error_callback _error_callback) -> machine& {
    this->__error_callback = _error_callback;
    return (*this);
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::trapped(S _current_state, zpt::fsm::payload<D, I> _content) -> void {
    auto _it = this->__callbacks.find(_current_state);
    if (_it != this->__callbacks.end()) {
        S _next_state = this->__undefined;

        for (auto _callback : _it->second) {
            try {
                S _potential = _callback(_current_state, std::get<0>(_content), std::get<1>(_content));
                expect(_next_state == this->__undefined || _potential == this->__undefined || _next_state == _potential,
                       "state '" << _potential
                                 << "' returned from transition callback isn't consistent with "
                                    "previously returned state '"
                                 << _next_state << "'");
                if (_potential != this->__undefined) { _next_state = this->coalesce(_current_state, _potential); }
            }
            catch (zpt::events::unregister const& _e) {
            }
        }
        if (_next_state == this->__pause) { return; }
        if (_next_state != this->__undefined) { this->trigger(_next_state, _content); }
        else if (_current_state != this->__end) { this->pause(_current_state, _content); }
    }
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::listen_to(S _event, callback _callback) -> void {
    auto _found = this->__callbacks.find(_event);
    if (_found == this->__callbacks.end()) {
        auto [_insert, _was_inserted] = this->__callbacks.insert(std::make_pair(_event, callback_list()));
        expect(_was_inserted, "something wrong with the map key logic around typename 'S'");
        _found = _insert;
    }
    _found->second.push_back(_callback);
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::report_error(S const& _event,
                                            zpt::fsm::payload<D, I>& _content,
                                            const char* _what,
                                            const char* _description,
                                            const char* _backtrace,
                                            int _error,
                                            int _status) -> bool {
    if (this->__error_callback != nullptr) {
        return this->__error_callback(
          _event, std::get<0>(_content), std::get<1>(_content), _what, _description, _backtrace, _error, _status);
    }
    return false;
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::start_threads() -> machine& {
    for (long _i = 0; _i != this->__processor_threads; ++_i) { this->add_consumer(); }
    return (*this);
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::to_string() const -> std::string {
    std::ostringstream _return;
    _return << "> begin: " << this->__begin << std::endl
            << "> end: " << this->__end << std::endl
            << "> undefined: " << this->__undefined << std::endl
            << "> pause: " << this->__pause << std::endl
            << "> transitions: " << std::endl;
    for (auto [_source, _potential] : this->__transitions) {
        _return << "  S(" << _source << ")" << std::endl << std::flush;
        for (auto [_target, _] : _potential) { _return << "\t-> S(" << _target << ")" << std::endl << std::flush; }
    }
    return _return.str();
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::coalesce(S _current, S _potential) const -> S {
    if (_potential != this->__undefined && _potential != this->__pause) {
        auto _possible = this->__transitions.find(_current);
        expect(_possible != this->__transitions.end(),
               "state '" << _current << "' not found as a possible source of a transition to '" << _potential << "'",
               500,
               0);
        expect(_possible->second.find(_potential) != _possible->second.end(),
               "state '" << _potential << "' not found as possible transitions from '" << _current << "'");
    }
    return _potential;
}

template<typename C, typename S, typename D, typename I>
auto
zpt::fsm::machine<C, S, D, I>::pause(S _current, zpt::fsm::payload<D, I> _content) -> void {
    {
        zpt::locks::spin_lock::guard _shared_sentry{ this->__stalled_lock, zpt::locks::spin_lock::exclusive };
        this->__stalled.insert(std::make_pair(std::get<1>(_content), std::make_pair(_current, std::get<0>(_content))));
    }
    this->trigger(this->__pause, _content);
}

} // namespace fsm
} // namespace zpt
