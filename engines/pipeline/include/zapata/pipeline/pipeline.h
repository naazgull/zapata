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

#include <zapata/base.h>
#include <zapata/json.h>
#include <zapata/uri.h>
#include <zapata/graph.h>
#include <zapata/events.h>

namespace zpt {
namespace pipeline {

template<typename T>
class stage;
template<typename T>
class engine;

template<typename T>
class event {
  public:
    event(zpt::pipeline::engine<T>& _parent, T _content, zpt::json const& _path);
    event(zpt::pipeline::event<T> const& _rhs);
    event(zpt::pipeline::event<T>&& _rhs);
    virtual ~event() = default;

    auto operator=(zpt::pipeline::event<T> const& _rhs) -> zpt::pipeline::event<T>&;
    auto operator=(zpt::pipeline::event<T>&& _rhs) -> zpt::pipeline::event<T>&;
    operator size_t();

    auto path() -> zpt::json;
    auto content() -> T;
    auto set_path(zpt::json const& _path) -> zpt::pipeline::event<T>&;
    auto set_content(T const& _content) -> zpt::pipeline::event<T>&;
    auto next_stage() -> zpt::pipeline::event<T>&;
    auto trigger(zpt::json _path,
                 T _content,
                 std::function<void(zpt::pipeline::event<T>&)> _callback = nullptr)
      -> zpt::pipeline::event<T>&;

  private:
    T __content;
    zpt::json __path;
    zpt::pipeline::engine<T>& __parent;
    size_t __current_stage{ 0 };
};

template<typename T>
class stage : public zpt::events::dispatcher<zpt::pipeline::stage<T>, zpt::json, T> {
  public:
    stage(int _max_threads = 1, int _max_per_thread = 1, long _pop_wait_milli = 0);
    stage(zpt::pipeline::stage<T> const&) = delete;
    stage(zpt::pipeline::stage<T>&&) = delete;
    virtual ~stage() = default;

    auto operator=(zpt::pipeline::stage<T> const&) = delete;
    auto operator=(zpt::pipeline::stage<T>&&) = delete;

    auto trapped(zpt::json _path, T _content) -> void;
    auto listen_to(zpt::json _path, std::function<void(zpt::pipeline::event<T>&)> _callback)
      -> void;

  private:
    zpt::tree::node<zpt::json, zpt::regex, std::function<void(zpt::pipeline::event<T>&)>>
      __callbacks;
};

template<typename T>
class engine {
  public:
    engine(size_t _pipeline_size = 1, int _max_threads_per_stage = 1);
    engine(zpt::pipeline::engine<T> const&) = delete;
    engine(zpt::pipeline::engine<T>&&) = delete;
    virtual ~engine() = default;

    auto operator=(zpt::pipeline::engine<T> const&) = delete;
    auto operator=(zpt::pipeline::engine<T>&&) = delete;

    auto add_to_stage(size_t _stage,
                      zpt::json _path,
                      std::function<void(zpt::pipeline::event<T>&)> _callback)
      -> zpt::pipeline::engine<T>&;
    auto next_stage(zpt::pipeline::event<T>& _content) -> zpt::pipeline::engine<T>&;
    auto trigger(zpt::json _path,
                 T _content,
                 std::function<void(zpt::pipeline::event<T>&)> _callback = nullptr)
      -> zpt::pipeline::engine<T>&;

  private:
    std::vector<zpt::pipeline::stage<T>> __stages;
    size_t __engine_size{ 1 };
    int __max_threads_per_stage{ 1 };
};
} // namepsace pipeline
} // namepsace zpt

template<typename T>
zpt::pipeline::event<T>::event(zpt::pipeline::engine<T>& _parent,
                               T _content,
                               zpt::json const& _path)
  : __content{ _content }
  , __path{ _path }
  , __parent{ _parent } {}

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
zpt::pipeline::event<T>::content() -> T {
    return this->__content;
}

template<typename T>
auto
zpt::pipeline::event<T>::set_path(zpt::json const& _path) -> zpt::pipeline::event<T>& {
    this->__path = _path;
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
    this->__parent.next_stage(*this);
    return (*this);
}

template<typename T>
auto
zpt::pipeline::event<T>::trigger(zpt::json _path,
                                 T _content,
                                 std::function<void(zpt::pipeline::event<T>&)> _callback)
  -> zpt::pipeline::event<T>& {
    this->__parent.trigger(_path, _content, _callback);
    return (*this);
}

template<typename T>
zpt::pipeline::stage<T>::stage(int _max_threads, int _max_per_thread, long _pop_wait_milli)
  : zpt::events::dispatcher<zpt::pipeline::stage<T>, std::string, T>{ _max_threads,
                                                                      _max_per_thread,
                                                                      _pop_wait_milli } {}

template<typename T>
auto
zpt::pipeline::stage<T>::trapped(zpt::json _path, T _content) -> void {
    this->__callbacks.eval(
      _path["splitted"].begin(), _path["splitted"].end(), _path["raw"]->str(), _content);
}

template<typename T>
auto
zpt::pipeline::stage<T>::listen_to(zpt::json _path,
                                   std::function<void(zpt::pipeline::event<T>&)> _callback)
  -> void {
    this->__callbacks.merge(
      _path["splitted"].begin(), _path["splitted"].end(), _path["regex"]->rgx(), _callback);
}

template<typename T>
zpt::pipeline::engine<T>::engine(size_t _engine_size, int _max_threads_per_stage)
  : __stages{ _engine_size }
  , __engine_size{ _engine_size }
  , __max_threads_per_stage{ _max_threads_per_stage } {}

template<typename T>
auto
zpt::pipeline::engine<T>::add_to_stage(size_t _stage,
                                       zpt::json _path,
                                       std::function<void(zpt::pipeline::event<T>&)> _callback)
  -> zpt::pipeline::engine<T>& {
    expect(_stage < this->__engine_size,
           std::string("invalid pipeline::stage number, should be lower then ") +
             std::to_string(this->__engine_size),
           500,
           0);
    this->__stages[_stage].listen_to(_path, _callback);
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::next_stage(zpt::pipeline::event<T>& _event) -> zpt::pipeline::engine<T>& {
    this->__stages[_event].trigger(_event.path(), _event.content());
    return (*this);
}

template<typename T>
auto
zpt::pipeline::engine<T>::trigger(zpt::json _path,
                                  T _content,
                                  std::function<void(zpt::pipeline::event<T>&)> _callback)
  -> zpt::pipeline::engine<T>& {
    return (*this);
}