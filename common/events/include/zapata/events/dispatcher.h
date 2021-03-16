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
#include <zapata/lockfree.h>

namespace zpt {
namespace events {

class unregister : public std::exception {
  public:
    unregister() = default;
    virtual ~unregister() = default;
};

class factory {
  public:
    factory() = default;
    virtual ~factory() = default;
};

template<typename C, typename E, typename V>
class dispatcher : public factory {
  public:
    using hazard_domain = typename zpt::lf::queue<std::tuple<E, V>>::hazard_domain;

    dispatcher(hazard_domain& _hazard_domain, long _max_pop_wait_micro = 50000L);
    virtual ~dispatcher();

    auto add_consumer() -> C&;
    auto trigger(E _type, V _content) -> C&;
    auto trap() -> C&;
    template<typename F>
    auto listen(E _type, F _callback) -> C&;
    template<typename F>
    auto mute(E _type, F _callback) -> C&;
    auto shutdown() -> C&;
    auto is_shutdown_ongoing() -> bool;
    auto loop() -> void;

  public:
    zpt::lf::queue<std::tuple<E, V>> __queue;
    long __max_pop_wait{ 50000L };
    std::vector<std::thread> __consumers;
    std::atomic<bool> __shutdown{ false };
};

} // namespace events
} // namespace zpt

template<typename C, typename E, typename V>
zpt::events::dispatcher<C, E, V>::dispatcher(hazard_domain& _hazard_domain,
                                             long _max_pop_wait_micro)
  : __queue{ _hazard_domain }
  , __max_pop_wait{ _max_pop_wait_micro } {}

template<typename C, typename E, typename V>
zpt::events::dispatcher<C, E, V>::~dispatcher() {
    expect(this->__shutdown.load(), "dispatcher has not been shutdown", 500, 0);
}

template<typename C, typename E, typename V>
auto
zpt::events::dispatcher<C, E, V>::add_consumer() -> C& {
    this->__consumers.emplace_back([this]() mutable -> void { this->loop(); });
    return (*static_cast<C*>(this));
}

template<typename C, typename E, typename V>
auto
zpt::events::dispatcher<C, E, V>::trigger(E _event, V _content) -> C& {
    this->__queue.push(std::make_tuple(_event, _content));
    return (*static_cast<C*>(this));
}

template<typename C, typename E, typename V>
auto
zpt::events::dispatcher<C, E, V>::trap() -> C& {
    auto [_event, _content] = this->__queue.pop();
    try {
        static_cast<C*>(this)->trapped(_event, _content);
    }
    catch (zpt::failed_expectation const& _e) {
        if (!static_cast<C*>(this)->report_error(_event,
                                                 _content,
                                                 _e.what(),
                                                 _e.description(),
                                                 _e.backtrace(),
                                                 _e.code(),
                                                 _e.status())) {
            throw;
        }
    }
    catch (std::exception const& _e) {
        if (!static_cast<C*>(this)->report_error(
              _event, _content, _e.what(), nullptr, nullptr, -1, 500)) {
            throw;
        }
    }
    return (*static_cast<C*>(this));
}

template<typename C, typename E, typename V>
template<typename F>
auto
zpt::events::dispatcher<C, E, V>::listen(E _event, F _listener) -> C& {
    static_cast<C*>(this)->listen_to(_event, _listener);
    return (*static_cast<C*>(this));
}

template<typename C, typename E, typename V>
template<typename F>
auto
zpt::events::dispatcher<C, E, V>::mute(E _event, F _listener) -> C& {
    static_cast<C*>(this)->mute_from(_event, _listener);
    return (*static_cast<C*>(this));
}

template<typename C, typename E, typename V>
auto
zpt::events::dispatcher<C, E, V>::shutdown() -> C& {
    expect(!this->__shutdown.load(),
           "`shutdown()` already been called from another execution path",
           500,
           0);
    this->__shutdown.store(true);
    for (size_t _idx = 0; _idx != this->__consumers.size(); ++_idx) {
        this->__consumers[_idx].join();
    }
    return (*static_cast<C*>(this));
}

template<typename C, typename E, typename V>
auto
zpt::events::dispatcher<C, E, V>::is_shutdown_ongoing() -> bool {
    return this->__shutdown.load();
}

template<typename C, typename E, typename V>
auto
zpt::events::dispatcher<C, E, V>::loop() -> void {
    zpt::this_thread::adaptive_timer<long, 5> _timer;
    do {
        try {
            this->trap();
            _timer.reset();
        }
        catch (zpt::NoMoreElementsException const& e) {
            if (this->__shutdown.load()) {
                zlog("Worker is exiting", zpt::trace);
                return;
            }
            _timer.sleep_for(this->__max_pop_wait);
        }
    } while (true);
}
