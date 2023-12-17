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

#include <memory>
#include <zapata/base.h>
#include <zapata/lockfree.h>
#include <zapata/allocator.h>

namespace zpt {
auto DISPATCHER() -> ssize_t&;

namespace events {
enum state { retrigger = -2, ready = -1, finish = 0, abort = 1 };
class dispatcher;
} // namespace events
} // namespace zpt

template<typename T>
concept Operation = requires(T t,
                             zpt::events::dispatcher& _d,
                             std::exception const& _e,
                             std::bad_alloc const& _bae,
                             zpt::failed_expectation const& _fe) {
                        { t(_d) } -> std::convertible_to<zpt::events::state>;
                        { t.blocked() } -> std::convertible_to<bool>;
                        { t.catch_error(_e) } -> std::convertible_to<bool>;
                        { t.catch_error(_bae) } -> std::convertible_to<bool>;
                        { t.catch_error(_fe) } -> std::convertible_to<bool>;
                    };

namespace zpt {
class abstract_event {
  public:
    abstract_event() = default;
    virtual ~abstract_event() = default;

    virtual auto blocked() const -> bool = 0;
    virtual auto catch_error(std::exception const& _e) -> bool = 0;
    virtual auto catch_error(std::bad_alloc const& _e) -> bool = 0;
    virtual auto catch_error(zpt::failed_expectation const& _e) -> bool = 0;
    virtual auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state = 0;
};
using event = std::shared_ptr<zpt::abstract_event>;

template<Operation T>
class event_t : public zpt::abstract_event {
  public:
    template<typename... Args>
    event_t(Args&&... _args);
    virtual ~event_t() = default;

    auto operator*() -> T&;
    auto operator*() const -> T const&;
    virtual auto blocked() const -> bool override final;
    virtual auto catch_error(std::exception const& _e) -> bool override final;
    virtual auto catch_error(std::bad_alloc const& _e) -> bool override final;
    virtual auto catch_error(zpt::failed_expectation const& _e) -> bool override final;
    virtual auto operator()(zpt::events::dispatcher& _dispatcher)
      -> zpt::events::state override final;

  private:
    T __underlying;
};

template<typename T>
auto make_event(T _operator) -> zpt::event;
template<typename T, typename... Args>
auto make_event(Args&&... _args) -> zpt::event;

namespace events {
class dispatcher {
  public:
    dispatcher(long _max_consumers);
    virtual ~dispatcher();

    auto start_consumers(long n_consumers = 0) -> dispatcher&;
    auto stop_consumers() -> dispatcher&;
    auto trigger(zpt::event _event) -> dispatcher&;
    template<typename T, typename... Args>
    auto trigger(Args&&... _args) -> dispatcher&;
    auto trap() -> dispatcher&;
    auto is_stopping_ongoing() -> bool;

  public:
    zpt::lf::queue<zpt::event> __queue;
    std::vector<std::thread> __consumers;
    zpt::padded_atomic<bool> __shutdown{ false };
    zpt::padded_atomic<long> __running_consumers{ 0 };
    long __max_consumers{ 2 };

    auto loop(long _consumer_nr) -> void;
};
} // namespace events
template<typename T>
auto event_cast(zpt::event& _event) -> T&;
} // namespace zpt

template<Operation T>
template<typename... Args>
zpt::event_t<T>::event_t(Args&&... _args)
  : __underlying{ std::forward<Args>(_args)... } {}

template<Operation T>
auto zpt::event_t<T>::operator*() -> T& {
    return this->__underlying;
}

template<Operation T>
auto zpt::event_t<T>::operator*() const -> T const& {
    return this->__underlying;
}

template<Operation T>
auto zpt::event_t<T>::blocked() const -> bool {
    return this->__underlying.blocked();
}

template<Operation T>
auto zpt::event_t<T>::catch_error(std::exception const& _e) -> bool {
    return this->__underlying.catch_error(_e);
}

template<Operation T>
auto zpt::event_t<T>::catch_error(std::bad_alloc const& _e) -> bool {
    return this->__underlying.catch_error(_e);
}

template<Operation T>
auto zpt::event_t<T>::catch_error(zpt::failed_expectation const& _e) -> bool {
    return this->__underlying.catch_error(_e);
}

template<Operation T>
auto zpt::event_t<T>::operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state {
    return this->__underlying(_dispatcher);
}

template<typename T>
auto zpt::make_event(T _operator) -> zpt::event {
    return std::allocate_shared<zpt::event_t<T>>(
      zpt::allocator<zpt::event_t<T>>{ zpt::global_cast<zpt::mem::pool>(zpt::MEM_POOL()) },
      _operator);
    return std::allocate_shared<zpt::event_t<T>>(std::allocator<zpt::event_t<T>>{}, _operator);
}

template<typename T, typename... Args>
auto zpt::make_event(Args&&... _args) -> zpt::event {
    return std::allocate_shared<zpt::event_t<T>>(
      zpt::allocator<zpt::event_t<T>>{ zpt::global_cast<zpt::mem::pool>(zpt::MEM_POOL()) },
      std::forward<Args>(_args)...);
}

template<typename T, typename... Args>
auto zpt::events::dispatcher::trigger(Args&&... _args) -> dispatcher& {
    this->trigger(zpt::make_event<T>(std::forward<Args>(_args)...));
    return (*this);
}

template<typename T>
auto zpt::event_cast(zpt::event& _event) -> T& {
    return *static_cast<zpt::event_t<T>&>(*_event.get());
}
