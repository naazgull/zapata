/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright inteautomaton in the software to the public
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

#include <iostream>
#include <zapata/startup.h>
#include <zapata/automaton.h>
#include <zapata/transport.h>

extern "C" auto
_zpt_load_(zpt::plugin& _plugin) -> void {
    auto& _boot = zpt::globals::get<zpt::startup::engine>(zpt::BOOT_ENGINE());
    auto& _config = zpt::globals::get<zpt::json>(zpt::GLOBAL_CONFIG());
    long _threads = std::max(static_cast<long>(_config["limits"]["max_queue_threads"]), 1L);
    long _max_queue_spin_sleep =
      std::max(static_cast<long>(_config["limits"]["max_queue_spin_sleep"]), 50000L);
    zpt::globals::alloc<zpt::automaton::engine>(
      zpt::AUTOMATON_ENGINE(), _threads, _plugin->config());

    _boot.add_thread([_max_queue_spin_sleep]() -> void {
        zlog("Starting AUTOMATON engine", zpt::info);
        auto& _polling = zpt::globals::get<zpt::stream::polling>(zpt::STREAM_POLLING());
        auto& _automaton = zpt::globals::get<zpt::automaton::engine>(zpt::AUTOMATON_ENGINE());
        _automaton.start_threads();

        zpt::this_thread::adaptative_timer<long, 5> _timer;
        size_t _serial{ 0 };
        do {
            try {
                auto _stream = _polling.pop();
                std::string _scheme{ _stream->transport() };
                zpt::exchange _channel{ _stream };
                _automaton.begin(_channel, zpt::json{ ++_serial });
                _timer.reset();
            }
            catch (zpt::NoMoreElementsException const& _e) {
                if (_automaton.is_shutdown_ongoing()) {
                    zlog("Exiting AUTOMATON router", zpt::trace);
                    return;
                }
                _timer.sleep_for(_max_queue_spin_sleep);
            }
        } while (true);
    });
}

extern "C" auto
_zpt_unload_(zpt::plugin& _plugin) -> void {
    zlog("Stopping AUTOMATON engine", zpt::info);
    zpt::globals::dealloc<zpt::automaton::engine>(zpt::AUTOMATON_ENGINE());
}
