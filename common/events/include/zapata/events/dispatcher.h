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

#include <zapata/lockfree/queue.h>

namespace zpt {
namespace events {

class factory {
  public:
    factory() = default;
    virtual ~factory() = default;
};

template<typename C, typename E, typename V>
class dispatcher : public factory {
  public:
    dispatcher(int _max_threads = 1, int _max_per_thread = 1, long _pop_wait_milli = 0);
    virtual ~dispatcher() = default;

    auto trigger(E _type, V _content) -> dispatcher&;
    auto trap() -> dispatcher&;
    template<typename F>
    auto listen(E _type, F _callback) -> dispatcher&;
    auto loop() -> void;

  public:
    zpt::lf::queue<std::tuple<E, V>> __queue;
    long __pop_wait{ 0 };
};

} // namespace events
} // namespace zpt

template<typename C, typename E, typename V>
zpt::events::dispatcher<C, E, V>::dispatcher(int _max_threads,
                                             int _max_per_thread,
                                             long _pop_wait_milli)
  : __queue{ _max_threads, _max_per_thread }
  , __pop_wait{ _pop_wait_milli } {
    expect(_max_threads > 0, "`_max_threads` expected to be higher than 0", 500, 0);
    expect(_max_per_thread > 0, "`_max_per_thread` expected to be higher than 0", 500, 0);
}

template<typename C, typename E, typename V>
auto
zpt::events::dispatcher<C, E, V>::trigger(E _event, V _content)
  -> zpt::events::dispatcher<C, E, V>& {
    this->__queue.push(std::make_pair(_event, _content));
    return (*this);
}

template<typename C, typename E, typename V>
auto
zpt::events::dispatcher<C, E, V>::trap() -> zpt::events::dispatcher<C, E, V>& {
    auto [_event, _content] = this->__queue.pop();
    static_cast<C*>(this)->trapped(_event, _content);
    return (*this);
}

template<typename C, typename E, typename V>
template<typename F>
auto
zpt::events::dispatcher<C, E, V>::listen(E _event, F _listener)
  -> zpt::events::dispatcher<C, E, V>& {
    static_cast<C*>(this)->listen_to(_event, _listener);
    return (*this);
}

template<typename C, typename E, typename V>
auto
zpt::events::dispatcher<C, E, V>::loop() -> void {
    do {
        try {
            this->trap();
        }
        catch (zpt::NoMoreElementsException& e) {
            if (this->__pop_wait == 0) {
                std::this_thread::yield();
            }
            else {
                std::this_thread::sleep_for(
                  std::chrono::duration<int, std::milli>{ this->__pop_wait });
            }
        }
    } while (true);
}
