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

namespace zpt {
namespace pipeline {

template<typename T>
class event;
template<typename T>
class stage;
template<typename T>
class engine;

template<typename T>
class event {
  public:
    event() = default;
    event(zpt::pipeline::engine<T>& _parent, T _content, zpt::json const& _path);
    event(zpt::pipeline::event<T> const& _rhs);
    event(zpt::pipeline::event<T>&& _rhs);
    virtual ~event() = default;

    auto operator=(zpt::pipeline::event<T> const& _rhs) -> zpt::pipeline::event<T>&;
    auto operator=(zpt::pipeline::event<T>&& _rhs) -> zpt::pipeline::event<T>&;
    operator size_t();

    auto path() -> zpt::json;
    auto content() -> T&;
    auto set_path(std::string const& _path) -> zpt::pipeline::event<T>&;
    auto set_content(T const& _content) -> zpt::pipeline::event<T>&;
    auto next_stage() -> zpt::pipeline::event<T>&;
    auto trigger(zpt::json _path,
                 T _content,
                 std::function<void(zpt::pipeline::event<T>&)> _callback = nullptr)
      -> zpt::pipeline::event<T>&;

  private:
    T __content;
    zpt::json __path;
    zpt::pipeline::engine<T>* __parent{ nullptr };
    size_t __current_stage{ 0 };
};

template<typename T>
class stage
  : public zpt::events::dispatcher<zpt::pipeline::stage<T>, zpt::json, zpt::pipeline::event<T>> {
  public:
    using hazard_domain = typename zpt::events::
      dispatcher<zpt::pipeline::stage<T>, zpt::json, zpt::pipeline::event<T>>::hazard_domain;

    stage(zpt::pipeline::stage<T>::hazard_domain& _hazard_domain, long _max_pop_wait_micro = -1);
    stage(zpt::pipeline::stage<T> const&) = delete;
    stage(zpt::pipeline::stage<T>&&) = delete;
    virtual ~stage() override = default;

    auto operator=(zpt::pipeline::stage<T> const&) = delete;
    auto operator=(zpt::pipeline::stage<T>&&) = delete;

    auto trapped(zpt::json _path, zpt::pipeline::event<T> _content) -> void;
    auto listen_to(zpt::json _path, std::function<void(zpt::pipeline::event<T>&)> _callback)
      -> void;
    auto error_callback(zpt::json& _event,
                        zpt::pipeline::event<T>& _content,
                        const char* _what,
                        const char* _description = nullptr,
                        int _error = 500) -> bool;

    auto set_error_callback(std::function<bool(zpt::json& _event,
                                               zpt::pipeline::event<T>& _content,
                                               const char* _what,
                                               const char* _description,
                                               int _error)> _error_callback)
      -> zpt::pipeline::stage<T>&;

  private:
    zpt::tree::node<zpt::json, zpt::regex, std::function<void(zpt::pipeline::event<T>&)>>
      __callbacks;
    zpt::lf::spin_lock __callback_lock;
    std::function<bool(zpt::json& _event,
                       zpt::pipeline::event<T>& _content,
                       const char* _what,
                       const char* _description,
                       int _error)>
      __error_callback;
};

template<typename T>
class engine {
  public:
    engine(size_t _pipeline_size = 1,
           zpt::json _stage_queue_configuration = { "max_stage_threads",
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

    auto start_threads() -> zpt::pipeline::engine<T>&;
    auto shutdown() -> zpt::pipeline::engine<T>&;
    auto is_shutdown_ongoing() -> bool;
    auto add_listener(size_t _stage,
                      std::string _pattern,
                      std::function<void(zpt::pipeline::event<T>&)> _callback)
      -> zpt::pipeline::engine<T>&;
    auto next_stage(zpt::pipeline::event<T> _content) -> zpt::pipeline::engine<T>&;
    auto trigger(std::string const& _uri,
                 T _content,
                 std::function<void(zpt::pipeline::event<T>&)> _callback = nullptr)
      -> zpt::pipeline::engine<T>&;

    auto set_error_callback(std::function<bool(zpt::json& _event,
                                               zpt::pipeline::event<T>& _content,
                                               const char* _what,
                                               const char* _description,
                                               int _error)> _error_callback)
      -> zpt::pipeline::engine<T>&;

  private:
    typename zpt::pipeline::stage<T>::hazard_domain __hazard_domain;
    std::vector<std::shared_ptr<zpt::pipeline::stage<T>>> __stages;
    size_t __pipeline_size{ 1 };
    long __threads_per_stage{ 1 };
};

auto
to_pattern(std::string const& _path) -> zpt::json;
auto
to_path(std::string const& _path) -> zpt::json;

} // namepsace pipeline
} // namepsace zpt

template<typename T>
zpt::pipeline::event<T>::event(zpt::pipeline::engine<T>& _parent,
                               T _content,
                               zpt::json const& _path)
  : __content{ _content }
  , __path{ _path }
  , __parent{ &_parent } {}

template<typename T>
zpt::pipeline::event<T>::event(zpt::pipeline::event<T> const& _rhs)
  : __content{ _rhs.__content }
  , __path{ _rhs.__path }
  , __parent{ _rhs.__parent }
  , __current_stage{ _rhs.__current_stage } {}

template<typename T>
zpt::pipeline::event<T>::event(zpt::pipeline::event<T>&& _rhs)
  : __content{ _rhs.__content }
  , __path{ _rhs.__path }
  , __parent{ _rhs.__parent }
  , __current_stage{ _rhs.__current_stage } {}

template<typename T>
auto
zpt::pipeline::event<T>::operator=(zpt::pipeline::event<T> const& _rhs)
  -> zpt::pipeline::event<T>& {
    this->__content = _rhs.__content;
    this->__path = _rhs.__path;
    this->__parent = _rhs.__parent;
    this->__current_stage = _rhs.__current_stage;
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::operator=(zpt::pipeline::event<T>&& _rhs) -> zpt::pipeline::event<T>& {
    this->__content = _rhs.__content;
    this->__path = _rhs.__path;
    this->__parent = _rhs.__parent;
    this->__current_stage = _rhs.__current_stage;
    return (*this);
}

template<typename T>
zpt::pipeline::event<T>::operator size_t() {
    return this->__current_stage;
}

template<typename T>
auto
zpt::pipeline::event<T>::path() -> zpt::json {
    return this->__path;
}

template<typename T>
auto
zpt::pipeline::event<T>::content() -> T& {
    return this->__content;
}

template<typename T>
auto
zpt::pipeline::event<T>::set_path(std::string const& _path) -> zpt::pipeline::event<T>& {
    this->__path = zpt::pipeline::to_path(_path);
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::set_content(T const& _content) -> zpt::pipeline::event<T>& {
    this->__content = _content;
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::next_stage() -> zpt::pipeline::event<T>& {
    ++this->__current_stage;
    this->__parent->next_stage(*this);
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::trigger(zpt::json _path,
                                 T _content,
                                 std::function<void(zpt::pipeline::event<T>&)> _callback)
  -> zpt::pipeline::event<T>& {
    this->__parent->trigger(_path, _content, _callback);
    return (*this);
}

template<typename T>
zpt::pipeline::stage<T>::stage(zpt::pipeline::stage<T>::hazard_domain& _hazard_domain,
                               long _max_pop_wait_micro)
  : zpt::events::dispatcher<zpt::pipeline::stage<T>, zpt::json, zpt::pipeline::event<T>>{
      _hazard_domain,
      _max_pop_wait_micro
  } {}

template<typename T>
auto
zpt::pipeline::stage<T>::trapped(zpt::json _path, zpt::pipeline::event<T> _event) -> void {
    zpt::lf::spin_lock::guard _sentry{ this->__callback_lock, zpt::lf::spin_lock::shared };
    if (!this->__callbacks.eval(
          _path["splitted"].begin(), _path["splitted"].end(), _path["raw"]->str(), _event)) {
        _event.next_stage();
    }
}

template<typename T>
auto
zpt::pipeline::stage<T>::listen_to(zpt::json _path,
                                   std::function<void(zpt::pipeline::event<T>&)> _callback)
  -> void {
    zpt::lf::spin_lock::guard _sentry{ this->__callback_lock, zpt::lf::spin_lock::exclusive };
    this->__callbacks.merge(
      _path["splitted"].begin(), _path["splitted"].end(), _path["regex"]->rgx(), _callback);
}

template<typename T>
auto
zpt::pipeline::stage<T>::error_callback(zpt::json& _event,
                                        zpt::pipeline::event<T>& _content,
                                        const char* _what,
                                        const char* _description,
                                        int _error) -> bool {
    if (this->__error_callback != nullptr) {
        return this->__error_callback(_event, _content, _what, _description, _error);
    }
    return false;
}

template<typename T>
auto
zpt::pipeline::stage<T>::set_error_callback(std::function<bool(zpt::json& _event,
                                                               zpt::pipeline::event<T>& _content,
                                                               const char* _what,
                                                               const char* _description,
                                                               int _error)> _error_callback)
  -> zpt::pipeline::stage<T>& {
    this->__error_callback = _error_callback;
    return (*this);
}

template<typename T>
zpt::pipeline::engine<T>::engine(size_t _pipeline_size, zpt::json _stage_queue_configuration)
  : __hazard_domain{ std::max(static_cast<long>(_stage_queue_configuration["max_stage_threads"]),
                              1L) *
                         (signed)_pipeline_size +
                       std::max(
                         static_cast<long>(_stage_queue_configuration["max_producer_threads"]),
                         1L) +
                       static_cast<long>(_stage_queue_configuration["max_consumer_threads"]),
                     4 }
  , __pipeline_size{ _pipeline_size }
  , __threads_per_stage{
      std::max(static_cast<long>(_stage_queue_configuration["max_stage_threads"]), 1L)
  } {
    for (size_t _i = 0; _i != this->__pipeline_size; ++_i) {
        this->__stages.push_back(std::make_shared<zpt::pipeline::stage<T>>(
          this->__hazard_domain,
          std::min(static_cast<long>(_stage_queue_configuration["max_queue_spin_sleep"]), 5000L)));
    }
}

template<typename T>
auto
zpt::pipeline::engine<T>::start_threads() -> zpt::pipeline::engine<T>& {
    size_t _n_stage{ 1 };
    for (auto _stage : this->__stages) {
        for (int _idx = 0; _idx != this->__threads_per_stage; ++_idx) {
            _stage->add_consumer();
            zlog("Started thread n. " << (_idx + 1) << " for stage " << _n_stage, zpt::trace);
        }
        ++_n_stage;
    }
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::shutdown() -> zpt::pipeline::engine<T>& {
    size_t _n_stage{ 1 };
    for (auto _stage : this->__stages) {
        _stage->shutdown();
        zlog("Exited from stage " << _n_stage, zpt::trace);
        ++_n_stage;
    }
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::is_shutdown_ongoing() -> bool {
    for (auto _stage : this->__stages) {
        if (_stage->is_shutdown_ongoing()) {
            return true;
        }
    }
    return false;
}

template<typename T>
auto
zpt::pipeline::engine<T>::add_listener(size_t _stage,
                                       std::string _pattern,
                                       std::function<void(zpt::pipeline::event<T>&)> _callback)
  -> zpt::pipeline::engine<T>& {
    expect(_stage < this->__pipeline_size,
           std::string("invalid pipeline::stage number, should be lower then ") +
             std::to_string(this->__pipeline_size),
           500,
           0);
    this->__stages[_stage]->listen(zpt::pipeline::to_pattern(_pattern), _callback);
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::next_stage(zpt::pipeline::event<T> _event) -> zpt::pipeline::engine<T>& {
    size_t _stage = _event;
    if (_stage < this->__stages.size()) {
        this->__stages[_stage]->trigger(_event.path(), _event);
    }
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::trigger(std::string const& _uri,
                                  T _content,
                                  std::function<void(zpt::pipeline::event<T>&)> _callback)
  -> zpt::pipeline::engine<T>& {
    if (_callback == nullptr) {
        zpt::json _path = zpt::pipeline::to_path(_uri);
        this->__stages[0]->trigger(_path, zpt::pipeline::event<T>{ *this, _content, _path });
    }
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::set_error_callback(std::function<bool(zpt::json& _event,
                                                                zpt::pipeline::event<T>& _content,
                                                                const char* _what,
                                                                const char* _description,
                                                                int _error)> _error_callback)
  -> zpt::pipeline::engine<T>& {
    for (auto _stage : this->__stages) {
        _stage->set_error_callback(_error_callback);
    }
    return (*this);
}
