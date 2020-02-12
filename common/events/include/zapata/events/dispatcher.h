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
    dispatcher(int _max_threads = 1, int _max_per_thread = 1, long _max_pop_wait_micro = 0);
    virtual ~dispatcher();

    auto add_consumer() -> C&;
    auto trigger(E _type, V _content) -> C&;
    auto trap() -> C&;
    template<typename F>
    auto listen(E _type, F _callback) -> C&;
    template<typename F>
    auto mute(E _type, F _callback) -> C&;
    auto shutdown() -> C&;
    auto loop() -> void;

  public:
    zpt::lf::queue<std::tuple<E, V>> __queue;
    long __max_pop_wait{ 0 };
    std::vector<std::thread> __consumers;
    std::atomic<bool> __shutdown{ false };
};

} // namespace events
} // namespace zpt

template<typename C, typename E, typename V>
zpt::events::dispatcher<C, E, V>::dispatcher(int _max_threads,
                                             int _max_per_thread,
                                             long _max_pop_wait_micro)
  : __queue{ _max_threads, _max_per_thread, 5 }
  , __max_pop_wait{ _max_pop_wait_micro } {
    expect(_max_threads > 1, "`_max_threads` expected to be higher than 1", 500, 0);
    expect(_max_per_thread > 0, "`_max_per_thread` expected to be higher than 0", 500, 0);
}

template<typename C, typename E, typename V>
zpt::events::dispatcher<C, E, V>::~dispatcher() {
    this->__shutdown.store(true);
    for (size_t _idx = 0; _idx != this->__consumers.size(); ++_idx) {
        this->__consumers[_idx].join();
    }
}

template<typename C, typename E, typename V>
auto
zpt::events::dispatcher<C, E, V>::add_consumer() -> C& {
    this->__consumers.emplace_back([=]() { this->loop(); });
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
    static_cast<C*>(this)->trapped(_event, _content);
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
    this->__shutdown.store(true);
    return (*static_cast<C*>(this));
}

template<typename C, typename E, typename V>
auto
zpt::events::dispatcher<C, E, V>::loop() -> void {
    long _waiting_iterations{ 0 };
    do {
        if (this->__shutdown.load()) {
            return;
        }

        try {
            this->trap();
            _waiting_iterations = 0;
        }
        catch (zpt::NoMoreElementsException& e) {
            _waiting_iterations =
              zpt::this_thread::adaptive_sleep_for(_waiting_iterations, this->__max_pop_wait);
        }
    } while (true);
}
