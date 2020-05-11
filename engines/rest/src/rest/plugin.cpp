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

#include <iostream>
#include <zapata/startup.h>
#include <zapata/rest.h>
#include <zapata/transport.h>

std::atomic<bool> _shutdown{ false };

extern "C" auto
_zpt_load_(zpt::plugin& _plugin) -> void {
    auto& _boot = zpt::globals::get<zpt::startup::engine>(zpt::BOOT_ENGINE());
    auto& _config = zpt::globals::get<zpt::json>(zpt::GLOBAL_CONFIG());
    size_t _stages = std::max(static_cast<size_t>(_plugin->config()["pipeline"]["n_stages"]), 1UL);
    long _max_queue_spin_sleep = _config["limits"]["max_queue_spin_sleep"];
    zpt::globals::alloc<zpt::rest::engine>(zpt::REST_ENGINE(), _stages, _config["limits"]);

    _boot.add_thread([_max_queue_spin_sleep]() -> void {
        zlog("Starting REST engine", zpt::info);
        auto& _polling = zpt::globals::get<zpt::stream::polling>(zpt::STREAM_POLLING());
        auto& _rest = zpt::globals::get<zpt::rest::engine>(zpt::REST_ENGINE());
        _rest.start_threads();

        long _waiting_iterations{ 0 };
        do {
            try {
                auto _stream = _polling.pop();
                std::string _scheme{ _stream->transport() };
                zpt::message _message{ _stream };
                _rest.trigger(_scheme + std::string(":"), _message);
                _waiting_iterations = 0;
            }
            catch (zpt::NoMoreElementsException const& _e) {
                if (_rest.is_shutdown_ongoing()) {
                    zlog("Exiting REST router", zpt::trace);
                    return;
                }
                _waiting_iterations =
                  zpt::this_thread::adaptive_sleep_for(_waiting_iterations, _max_queue_spin_sleep);
            }
        } while (true);
    });
}

extern "C" auto
_zpt_unload_(zpt::plugin& _plugin) -> void {
    zlog("Stopping REST engine", zpt::info);
    zpt::globals::get<zpt::rest::engine>(zpt::REST_ENGINE()).shutdown();
    zpt::globals::dealloc<zpt::rest::engine>(zpt::REST_ENGINE());
}
