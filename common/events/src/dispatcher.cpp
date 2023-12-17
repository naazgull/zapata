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

#include <zapata/events/dispatcher.h>

auto zpt::DISPATCHER() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::events::dispatcher::dispatcher(long _max_consumers)
  : __queue{ _max_consumers + 1 }
  , __max_consumers{ _max_consumers } {}

zpt::events::dispatcher::~dispatcher() { this->stop_consumers(); }

auto zpt::events::dispatcher::start_consumers(long _n_consumers) -> dispatcher& {
    if (_n_consumers == 0 ||
        _n_consumers + this->__consumers.size() >= static_cast<size_t>(this->__max_consumers)) {
        _n_consumers = this->__max_consumers - this->__consumers.size();
    }
    for (long _consumer_nr = this->__consumers.size(), _idx = 0; _idx != _n_consumers;
         ++_idx, ++_consumer_nr) {
        auto& _consumer = this->__consumers.emplace_back(
          [this, _consumer_nr]() mutable -> void { this->loop(_consumer_nr); });
        _consumer.detach();
        ++(*this->__running_consumers);
    }
    return (*this);
}

auto zpt::events::dispatcher::stop_consumers() -> dispatcher& {
    expect(!this->__shutdown->load(),
           "`stop_consunmers()` already been called from another execution path");
    this->__shutdown->store(true);
    while (this->__running_consumers->load(std::memory_order_relaxed) != 0) {
        std::this_thread::sleep_for(std::chrono::duration<int, std::milli>{ 100 });
    }
    this->__consumers.clear();
    this->__shutdown->store(false);
    return (*this);
}

auto zpt::events::dispatcher::trigger(zpt::event _event) -> dispatcher& {
    this->__queue.push(_event);
    return (*this);
}

auto zpt::events::dispatcher::trap() -> dispatcher& {
    auto _event = this->__queue.pop();
    if (_event->blocked()) {
        this->trigger(_event);
        std::this_thread::yield();
        return (*this);
    }
    try {
        auto state = (*_event)((*this));
        if (state == zpt::events::retrigger) { this->trigger(_event); }
    }
    catch (zpt::failed_expectation const& _e) {
        if (!_event->catch_error(_e)) {
            zlog("Uncaught exception found: " << _e.what(), zpt::error);
        }
    }
    catch (std::bad_alloc const& _e) {
        if (!_event->catch_error(_e)) {
            zlog("Uncaught exception found: " << _e.what(), zpt::error);
        }
    }
    catch (std::exception const& _e) {
        if (!_event->catch_error(_e)) {
            zlog("Uncaught exception found: " << _e.what(), zpt::error);
        }
    }
    return (*this);
}

auto zpt::events::dispatcher::is_stopping_ongoing() -> bool { return this->__shutdown->load(); }

auto zpt::events::dispatcher::loop(long _consumer_nr) -> void {
    zpt::this_thread::timer<float> _timer{ 0.005f };
    zlog("Thread@" << _consumer_nr << " starting", zpt::trace);
    do {
        try {
            this->trap();
            _timer.reset();
        }
        catch (zpt::NoMoreElementsException const& e) {
            _timer.sleep_for(0.1f);
        }
    } while (!this->__shutdown->load(std::memory_order_relaxed));
    this->__queue.clear_thread_context();
    --(*this->__running_consumers);
    zlog("Thread@" << _consumer_nr << " stopping", zpt::trace);
}
