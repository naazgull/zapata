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

#include <zapata/base.h>
#include <zapata/json.h>
#include <zapata/uri.h>
#include <zapata/graph.h>
#include <zapata/lockfree.h>
#include <zapata/events.h>
#include <zapata/mem/ref_ptr.h>

namespace zpt {
namespace pipeline {

template<typename T>
class event;
template<typename T>
class step;
template<typename T>
class engine;

template<typename T>
class event {
  private:
    class event_t {
      public:
        class position {
          public:
            friend class zpt::pipeline::event<T>::event_t;
            friend class zpt::pipeline::event<T>;

            position() = default;
            position(position const& _rhs) = default;
            position(position&& _rhs) = default;
            virtual ~position() = default;

            auto operator=(position const& _rhs) -> position& = default;
            auto operator=(position&& _rhs) -> position& = default;

            auto current() -> size_t;
            auto marked() -> size_t;
            auto at(size_t _requested_next) -> position&;
            auto first() -> position&;
            auto last() -> position&;
            auto next() -> position&;
            auto mark() -> size_t;

          private:
            size_t __current{ 0 };
            size_t __next{ 0 };
            size_t __requested{ 0 };
            size_t __max{ 0 };
            bool __pause{ false };

            position(size_t _max);
        };
        friend class zpt::pipeline::event<T>::event_t::position;
        friend class zpt::pipeline::event<T>;
        friend class zpt::pipeline::step<T>;
        friend class zpt::pipeline::engine<T>;

        event_t() = default;
        event_t(zpt::pipeline::engine<T>& _parent, T _content, zpt::json const& _path);
        event_t(zpt::pipeline::event<T>::event_t const& _rhs) = delete;
        event_t(zpt::pipeline::event<T>::event_t&& _rhs) = delete;
        virtual ~event_t() = default;

        auto operator=(zpt::pipeline::event<T>::event_t const& _rhs) -> zpt::pipeline::event<T>::event_t& = delete;
        auto operator=(zpt::pipeline::event<T>::event_t&& _rhs) -> zpt::pipeline::event<T>::event_t& = delete;

        auto path() -> zpt::json;
        auto content() -> T&;
        auto set_path(std::string const& _path) -> zpt::pipeline::event<T>::event_t&;
        auto set_content(T const& _content) -> zpt::pipeline::event<T>::event_t&;
        auto trigger(std::string const& _uri, T _content) -> zpt::pipeline::event<T>::event_t&;
        auto trigger(zpt::json _uri, T _content) -> zpt::pipeline::event<T>::event_t&;
        auto steps() -> zpt::pipeline::event<T>::event_t::position&;

      private:
        T __content;
        zpt::json __path;
        zpt::ref_ptr<zpt::pipeline::engine<T>> __parent{ nullptr };
        zpt::pipeline::event<T>::event_t::position __current_step;
    };

  public:
    friend class zpt::pipeline::step<T>;
    friend class zpt::pipeline::engine<T>;

    event() = default;
    event(zpt::pipeline::engine<T>& _parent, T _content, zpt::json const& _path);
    event(zpt::pipeline::event<T> const& _rhs) = default;
    event(zpt::pipeline::event<T>&& _rhs) = default;
    virtual ~event() = default;

    auto operator=(zpt::pipeline::event<T> const& _rhs) -> zpt::pipeline::event<T>& = default;
    auto operator=(zpt::pipeline::event<T>&& _rhs) -> zpt::pipeline::event<T>& = default;

    auto operator->() -> zpt::pipeline::event<T>::event_t*;
    auto operator*() -> zpt::pipeline::event<T>::event_t&;

    auto pause() -> zpt::pipeline::event<T>&;
    auto resume() -> zpt::pipeline::event<T>&;
    auto cancel() -> zpt::pipeline::event<T>&;

  private:
    std::shared_ptr<zpt::pipeline::event<T>::event_t> __underlying;

    auto send_to_next_step() -> void;
};

template<typename T>
class step : public zpt::events::dispatcher<zpt::pipeline::step<T>, zpt::json, zpt::pipeline::event<T>> {
  public:
    friend class zpt::pipeline::event<T>;

    using event_callback = typename zpt::pipeline::engine<T>::event_callback;
    using error_callback = typename zpt::pipeline::engine<T>::error_callback;

    step(long _max_threads, long _max_pop_wait_micro = -1);
    step(zpt::pipeline::step<T> const&) = delete;
    step(zpt::pipeline::step<T>&&) = delete;
    virtual ~step() override = default;

    auto operator=(zpt::pipeline::step<T> const&) = delete;
    auto operator=(zpt::pipeline::step<T>&&) = delete;

    auto trapped(zpt::json _path, zpt::pipeline::event<T> _content) -> void;
    auto listen_to(zpt::json _path, event_callback _callback) -> void;
    auto report_error(zpt::json& _event,
                      zpt::pipeline::event<T>& _content,
                      const char* _what,
                      const char* _description = nullptr,
                      const char* _backtrace = nullptr,
                      int _error = -1,
                      int status = 500) -> bool;

    auto set_error_callback(error_callback _error_callback) -> zpt::pipeline::step<T>&;

  private:
    zpt::tree::node<zpt::json, zpt::regex, event_callback> __callbacks;
    zpt::locks::spin_lock __callback_lock;
    error_callback __error_callback;
};

template<typename T>
class engine {
  public:
    friend class zpt::pipeline::event<T>;
    using event_callback = std::function<void(zpt::pipeline::event<T>&)>;
    using error_callback = std::function<bool(zpt::json& _event,
                                              zpt::pipeline::event<T>& _content,
                                              const char* _what,
                                              const char* _description,
                                              const char* _backtrace,
                                              int _error,
                                              int status)>;

    engine(size_t _pipeline_size = 1,
           zpt::json _step_queue_configuration = { "max_queue_threads",
                                                   1,
                                                   "max_producer_threads",
                                                   1,
                                                   "max_consumer_threads",
                                                   0,
                                                   "max_queue_spin_sleep",
                                                   5000 });
    engine(zpt::pipeline::engine<T> const&) = delete;
    engine(zpt::pipeline::engine<T>&&) = delete;
    virtual ~engine() = default;

    auto operator=(zpt::pipeline::engine<T> const&) = delete;
    auto operator=(zpt::pipeline::engine<T>&&) = delete;

    auto size() -> size_t;

    auto start_threads() -> zpt::pipeline::engine<T>&;
    auto shutdown() -> zpt::pipeline::engine<T>&;
    auto is_shutdown_ongoing() -> bool;

    auto add_listener(size_t _step, std::string _pattern, event_callback _callback) -> zpt::pipeline::engine<T>&;
    auto trigger(std::string const& _uri, T _content) -> zpt::pipeline::engine<T>&;
    auto trigger(zpt::json _uri, T _content) -> zpt::pipeline::engine<T>&;

    auto set_error_callback(error_callback _error_callback) -> zpt::pipeline::engine<T>&;

  private:
    std::vector<std::shared_ptr<zpt::pipeline::step<T>>> __steps;
    size_t __pipeline_size{ 1 };
    long __threads_per_step{ 1 };

    auto send_to_next_step(zpt::pipeline::event<T> _content) -> zpt::pipeline::engine<T>&;
};

auto
to_pattern(std::string const& _path) -> zpt::json;
auto
to_path(std::string const& _path) -> zpt::json;

} // namespace pipeline
} // namespace zpt

template<typename T>
auto
zpt::pipeline::event<T>::event_t::position::current() -> size_t {
    return this->__current;
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::position::marked() -> size_t {
    return this->__next;
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::position::at(size_t _requested_next) -> position& {
    this->__requested = _requested_next;
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::position::first() -> position& {
    this->__requested = 0;
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::position::last() -> position& {
    this->__requested = this->__max - 1;
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::position::next() -> position& {
    this->__requested = this->__current + 1;
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::position::mark() -> size_t {
    if (this->__requested <= this->__current || (this->__next > this->__current && this->__requested > this->__next)) {
        return this->__next;
    }
    this->__next = this->__requested;
    return this->__requested;
}

template<typename T>
zpt::pipeline::event<T>::event_t::position::position(size_t _max)
  : __max{ _max } {}

template<typename T>
zpt::pipeline::event<T>::event_t::event_t(zpt::pipeline::engine<T>& _parent, T _content, zpt::json const& _path)
  : __content{ _content }
  , __path{ _path }
  , __parent{ _parent }
  , __current_step{ _parent.size() } {}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::path() -> zpt::json {
    return this->__path;
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::content() -> T& {
    return this->__content;
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::set_path(std::string const& _path) -> zpt::pipeline::event<T>::event_t& {
    this->__path = zpt::pipeline::to_path(_path);
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::set_content(T const& _content) -> zpt::pipeline::event<T>::event_t& {
    this->__content = _content;
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::trigger(std::string const& _uri, T _content) -> zpt::pipeline::event<T>::event_t& {
    this->__parent->trigger(_uri, _content);
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::trigger(zpt::json _uri, T _content) -> zpt::pipeline::event<T>::event_t& {
    this->__parent->trigger(_uri, _content);
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::event_t::steps() -> zpt::pipeline::event<T>::event_t::position& {
    return this->__current_step;
}

template<typename T>
zpt::pipeline::event<T>::event(zpt::pipeline::engine<T>& _parent, T _content, zpt::json const& _path)
  : __underlying{ std::make_shared<zpt::pipeline::event<T>::event_t>(_parent, _content, _path) } {}

template<typename T>
auto
zpt::pipeline::event<T>::operator->() -> zpt::pipeline::event<T>::event_t* {
    return this->__underlying.get();
}

template<typename T>
auto
zpt::pipeline::event<T>::operator*() -> zpt::pipeline::event<T>::event_t& {
    return *this->__underlying.get();
}

template<typename T>
auto
zpt::pipeline::event<T>::pause() -> zpt::pipeline::event<T>& {
    this->__underlying->__current_step.__pause = true;
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::resume() -> zpt::pipeline::event<T>& {
    this->__underlying->__current_step.__pause = false;
    this->send_to_next_step();
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::cancel() -> zpt::pipeline::event<T>& {
    this->__underlying->__current_step.__pause = true;
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::send_to_next_step() -> void {
    if (!this->__underlying->__current_step.__pause) {
        this->__underlying->__current_step.__current = this->__underlying->__current_step.__next;
        this->__underlying->__parent->send_to_next_step(*this);
    }
}

template<typename T>
zpt::pipeline::step<T>::step(long _max_threads, long _max_pop_wait_micro)
  : zpt::events::dispatcher<zpt::pipeline::step<T>, zpt::json, zpt::pipeline::event<T>>{ _max_threads,
                                                                                         _max_pop_wait_micro } {}

template<typename T>
auto
zpt::pipeline::step<T>::trapped(zpt::json _path, zpt::pipeline::event<T> _event) -> void {
    zpt::locks::spin_lock::guard _sentry{ this->__callback_lock, zpt::locks::spin_lock::shared };
    this->__callbacks.eval(_path["splitted"].begin(), _path["splitted"].end(), _path["raw"]->string(), _event);
    if (_event->steps().marked() == _event->steps().current()) { _event->steps().next().mark(); }
    _event.send_to_next_step();
}

template<typename T>
auto
zpt::pipeline::step<T>::listen_to(zpt::json _path, event_callback _callback) -> void {
    zpt::locks::spin_lock::guard _sentry{ this->__callback_lock, zpt::locks::spin_lock::exclusive };
    this->__callbacks.merge(_path["splitted"].begin(), _path["splitted"].end(), _path["regex"]->regex(), _callback);
}

template<typename T>
auto
zpt::pipeline::step<T>::report_error(zpt::json& _event,
                                     zpt::pipeline::event<T>& _content,
                                     const char* _what,
                                     const char* _description,
                                     const char* _backtrace,
                                     int _error,
                                     int status) -> bool {
    bool _to_return{ true };
    if (this->__error_callback != nullptr) {
        _to_return = this->__error_callback(_event, _content, _what, _description, _backtrace, _error, status);
    }
    _content->steps().last().mark();
    _content.send_to_next_step();
    return _to_return;
}

template<typename T>
auto
zpt::pipeline::step<T>::set_error_callback(error_callback _error_callback) -> zpt::pipeline::step<T>& {
    this->__error_callback = _error_callback;
    return (*this);
}

template<typename T>
zpt::pipeline::engine<T>::engine(size_t _pipeline_size, zpt::json _step_queue_configuration)
  : __pipeline_size{ _pipeline_size }
  , __threads_per_step{ std::max(static_cast<long>(_step_queue_configuration["max_queue_threads"]), 1L) } {
    long _max_threads =
      std::max(static_cast<long>(_step_queue_configuration["max_queue_threads"]), 1L) * (signed)_pipeline_size +
      std::max(static_cast<long>(_step_queue_configuration["max_producer_threads"]), 1L) +
      static_cast<long>(_step_queue_configuration["max_consumer_threads"]);

    for (size_t _i = 0; _i != this->__pipeline_size; ++_i) {
        this->__steps.push_back(std::make_shared<zpt::pipeline::step<T>>(
          _max_threads, std::max(static_cast<long>(_step_queue_configuration["max_queue_spin_sleep"]), 50000L)));
    }
}

template<typename T>
auto
zpt::pipeline::engine<T>::size() -> size_t {
    return this->__pipeline_size;
}

template<typename T>
auto
zpt::pipeline::engine<T>::start_threads() -> zpt::pipeline::engine<T>& {
    size_t _n_step{ 1 };
    for (auto _step : this->__steps) {
        for (int _idx = 0; _idx != this->__threads_per_step; ++_idx) {
            _step->add_consumer();
            zlog("Started thread n. " << (_idx + 1) << " for step " << _n_step, zpt::trace);
        }
        ++_n_step;
    }
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::shutdown() -> zpt::pipeline::engine<T>& {
    size_t _n_step{ 1 };
    for (auto _step : this->__steps) {
        _step->shutdown();
        zlog("Exited from step " << _n_step, zpt::trace);
        ++_n_step;
    }
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::is_shutdown_ongoing() -> bool {
    for (auto _step : this->__steps) {
        if (_step->is_shutdown_ongoing()) { return true; }
    }
    return false;
}

template<typename T>
auto
zpt::pipeline::engine<T>::add_listener(size_t _step, std::string _pattern, event_callback _callback)
  -> zpt::pipeline::engine<T>& {
    expect(_step < this->__pipeline_size,
           std::string("invalid pipeline::step number, should be lower then ") + std::to_string(this->__pipeline_size));
    this->__steps[_step]->listen(zpt::pipeline::to_pattern(_pattern), _callback);
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::trigger(std::string const& _uri, T _content) -> zpt::pipeline::engine<T>& {
    return this->trigger(zpt::pipeline::to_path(_uri), _content);
}

template<typename T>
auto
zpt::pipeline::engine<T>::trigger(zpt::json _uri, T _content) -> zpt::pipeline::engine<T>& {
    this->__steps[0]->trigger(_uri, zpt::pipeline::event<T>{ *this, _content, _uri });
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::set_error_callback(error_callback _error_callback) -> zpt::pipeline::engine<T>& {
    for (auto _step : this->__steps) { _step->set_error_callback(_error_callback); }
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::send_to_next_step(zpt::pipeline::event<T> _event) -> zpt::pipeline::engine<T>& {
    std::size_t _step{ _event->steps().current() };
    if (_step < this->__steps.size()) {
        zlog("Event " << std::hex << &(*_event) << std::dec << " going to step " << _step << " with path '"
                      << _event->path()["raw"] << "'",
             zpt::trace);
        this->__steps[_step]->trigger(_event->path(), _event);
    }
    return (*this);
}
