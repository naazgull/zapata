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

template<typename D, typename I>
using payload = std::tuple<D, I>;

template<typename S, typename D, typename I>
class machine {
  public:
    class engine_t : public zpt::events::dispatcher<engine_t, S, zpt::fsm::payload<D, I>> {
      public:
        friend class machine;

        using hazard_domain =
          typename zpt::events::dispatcher<engine_t, S, zpt::fsm::payload<D, I>>::hazard_domain;
        using callback = std::function<S(S&, D&, I const&)>;
        using callback_list = std::vector<std::tuple<zpt::padded_atomic<bool>, callback>>;
        using state_list = std::map<S, bool>;
        using error_callback = std::function<bool(S const& _event,
                                                  D const& _content,
                                                  I const& _id,
                                                  const char* _what,
                                                  const char* _description,
                                                  const char* _backtrace,
                                                  int _error,
                                                  int status)>;

        engine_t(hazard_domain& _hazard_domain, zpt::json _config);
        virtual ~engine_t() = default;

        auto add_allowed_transitions(zpt::json _states) -> engine_t&;

        auto begin(D _content, I _id) -> engine_t&;
        auto resume(I _id, S _state) -> engine_t&;
        auto add_transition(S _event, callback _callback) -> engine_t&;
        auto set_error_callback(error_callback _error_callback) -> engine_t&;

        auto trapped(S _current_state, zpt::fsm::payload<D, I> _content) -> void;
        auto listen_to(S _event, callback _callback) -> void;
        auto mute_from(S _event, callback _callback) -> void;
        auto report_error(S const& _event,
                          zpt::fsm::payload<D, I> const& _content,
                          const char* _what,
                          const char* _description = nullptr,
                          const char* _backtrace = nullptr,
                          int _error = -1,
                          int _status = 500) -> bool;

        auto to_string() -> std::string;
        friend auto operator<<(std::ostream& _out, engine_t& _in) -> std::ostream& {
            _out << _in.to_string() << std::flush;
            return _out;
        }

      private:
        zpt::lf::spin_lock __stalled_lock;
        std::map<S, callback_list> __callbacks;
        std::map<S, state_list> __transitions;
        std::map<I, std::tuple<S, D>> __stalled;
        S __undefined;
        S __begin;
        S __end;
        S __pause;
        error_callback __error_callback{ nullptr };

        auto coalesce(S _current, S _potential) -> S;
        auto pause(S _current, zpt::fsm::payload<D, I> _content) -> void;
        auto set_states(zpt::json _states) -> engine_t&;
    };

    machine(long _max_threads, zpt::json _config);
    virtual ~machine();

    auto operator*() -> engine_t&;
    auto operator->() -> engine_t*;

    friend auto operator<<(std::ostream& _out, machine& _in) -> std::ostream& {
        _out << _in.__underlying.to_string() << std::flush;
        return _out;
    }

  protected:
    auto set_states(zpt::json _states) -> engine_t&;

  private:
    typename engine_t::hazard_domain __hazard_domain;
    engine_t __underlying;
};

template<typename S, typename D, typename I>
zpt::fsm::machine<S, D, I>::engine_t::engine_t(hazard_domain& _hazard_domain, zpt::json _config)
  : zpt::events::dispatcher<zpt::fsm::machine<S, D, I>::engine_t, S, zpt::fsm::payload<D, I>>{
      _hazard_domain
  } {
    this->set_states(_config);
    if (_config["transitions"]->ok()) { this->add_allowed_transitions(_config["transitions"]); }
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::set_states(zpt::json _states) -> engine_t& {
    this->__undefined = static_cast<S>(_states["undefined"]);
    this->__begin = static_cast<S>(_states["begin"]);
    this->__end = static_cast<S>(_states["end"]);
    this->__pause = static_cast<S>(_states["pause"]);
    return (*this);
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::add_allowed_transitions(zpt::json _states) -> engine_t& {
    expect(_states->ok() && _states->is_array(),
           "states JSON configuration must be a JSON array",
           500,
           0);

    for (auto [_, __, _potential] : _states) {
        expect(_potential[1]->is_array(),
               "each state allowed states member value must be an array",
               500,
               0);
        for (auto [_, __, _target] : _potential[1]) {
            std::cout << "adding transition " << _potential[0] << " -> " << _target << std::endl
                      << std::flush;
            this->__transitions[static_cast<S>(_potential[0])][static_cast<S>(_target)] = true;
        }
    }
    return (*this);
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::begin(D _content, I _id) -> engine_t& {
    this->trigger(this->__begin, zpt::fsm::payload<D, I>{ std::make_tuple(_content, _id) });
    return (*this);
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::resume(I _id, S _state) -> engine_t& {
    auto _found = this->__stalled.find(_id);
    expect(_found != this->__stalled.end(), "no such payload identified by " << _id, 500, 0);
    S _next_state = this->coalesce(std::get<0>(_found->second), _state);
    this->trigger(
      _next_state,
      zpt::fsm::payload<D, I>{ std::make_tuple(std::get<1>(_found->second), _found->first) });
    return (*this);
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::add_transition(S _source, callback _callback) -> engine_t& {
    this->listen(_source, _callback);
    return (*this);
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::set_error_callback(error_callback _error_callback)
  -> engine_t& {
    this->__error_callback = _error_callback;
    return (*this);
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::trapped(S _current_state, zpt::fsm::payload<D, I> _content)
  -> void {
    auto _it = this->__callbacks.find(_current_state);
    if (_it != this->__callbacks.end()) {
        S _next_state = this->__undefined;

        for (auto _callback = _it->second.begin(); _callback != _it->second.end(); ++_callback) {
            try {
                if (std::get<0>(*_callback)->load()) {
                    S _potential = std::get<1>(*_callback)(
                      _current_state, std::get<0>(_content), std::get<1>(_content));
                    expect(_next_state == this->__undefined || _potential == this->__undefined ||
                             _next_state == _potential,
                           "state '" << _potential
                                     << "' returned from transition callback isn't consistent with "
                                        "previously returned state '"
                                     << _next_state << "'",
                           500,
                           0);
                    if (_potential != this->__undefined) {
                        _next_state = this->coalesce(_current_state, _potential);
                    }
                }
            }
            catch (zpt::events::unregister const& _e) {
                this->mute(_current_state, std::get<1>(*_callback));
            }
        }
        if (_next_state == this->__pause) { return; }
        if (_next_state != this->__undefined) { this->trigger(_next_state, _content); }
        else if (_current_state != this->__end) {
            this->pause(_current_state, _content);
        }
    }
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::listen_to(S _event, callback _callback) -> void {
    this->__callbacks[_event].push_back(std::make_tuple(true, _callback));
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::mute_from(S _event, callback _callback) -> void {
    auto _found = this->__callbacks.find(_event);
    if (_found != this->__callbacks.end()) {
        for (auto _c = _found->second.begin(); _c != _found->second.end(); ++_c) {
            std::get<0>(*_c) = false;
        }
    }
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::report_error(S const& _event,
                                                   zpt::fsm::payload<D, I> const& _content,
                                                   const char* _what,
                                                   const char* _description,
                                                   const char* _backtrace,
                                                   int _error,
                                                   int _status) -> bool {
    if (this->__error_callback != nullptr) {
        return this->__error_callback(_event,
                                      std::get<0>(_content),
                                      std::get<1>(_content),
                                      _what,
                                      _description,
                                      _backtrace,
                                      _error,
                                      _status);
    }
    return false;
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::to_string() -> std::string {
    std::ostringstream _return;
    _return << "> begin: " << this->__begin << std::endl
            << "> end: " << this->__end << std::endl
            << "> undefined: " << this->__undefined << std::endl
            << "> pause: " << this->__pause << std::endl
            << "> transitions: " << std::endl;
    for (auto [_source, _potential] : this->__transitions) {
        _return << "  S(" << _source << ")" << std::endl << std::flush;
        for (auto [_target, _] : _potential) {
            _return << "\t-> S(" << _target << ")" << std::endl << std::flush;
        }
    }
    return _return.str();
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::coalesce(S _current, S _potential) -> S {
    if (_potential != this->__undefined && _potential != this->__pause) {
        auto _possible = this->__transitions[_current];
        expect(_possible.find(_potential) != _possible.end(),
               "state '" << _potential << "' not found in possible transitions from '" << _current
                         << "'",
               500,
               0);
    }
    return _potential;
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::engine_t::pause(S _current, zpt::fsm::payload<D, I> _content) -> void {
    this->__stalled.insert(
      std::make_pair(std::get<1>(_content), std::make_pair(_current, std::get<0>(_content))));
    this->trigger(this->__pause, _content);
}

template<typename S, typename D, typename I>
zpt::fsm::machine<S, D, I>::machine(long _threads, zpt::json _config)
  : __hazard_domain{ _threads + 1, 4 }
  , __underlying{ __hazard_domain, _config } {
    for (auto _i = 0; _i != _threads; ++_i) { this->__underlying.add_consumer(); }
}

template<typename S, typename D, typename I>
zpt::fsm::machine<S, D, I>::~machine() {
    this->__underlying.shutdown();
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::operator*() -> zpt::fsm::machine<S, D, I>::engine_t& {
    return this->__underlying;
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::operator->() -> zpt::fsm::machine<S, D, I>::engine_t* {
    return &this->__underlying;
}

template<typename S, typename D, typename I>
auto
zpt::fsm::machine<S, D, I>::set_states(zpt::json _states) -> zpt::fsm::machine<S, D, I>::engine_t& {
    return this->__underlying.set_states(_states);
}

} // currentspace fsm
} // nanespace zpt
