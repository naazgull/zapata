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
#include <zapata/transport.h>
#include <zapata/transport/engine.h>

namespace {
class transport_engine_stop_threads : public zpt::system_event {
  public:
    using zpt::system_event::system_event;
    ~transport_engine_stop_threads() = default;

    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state override {
        zpt::SYSTEM_EVENTS_RESOLVER()->remove<::transport_engine_stop_threads>(
          zpt::system_event_type::EXITING);
        zpt::TRANSPORT_ENGINE()->shutdown();
        return zpt::events::finish;
    }
};
} // namespace

/** @brief Plugin entry point: initializes the global transport engine.
 * @param _plugin Plugin instance.
 * @return void. */
extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    zpt::TRANSPORT_ENGINE(_plugin.config());
    zlog("Started multi-transport engine ("
           << (_plugin.config()("limits")("max_workers")->ok()
                 ? _plugin.config()("limits")("max_workers")->integer()
                 : 1)
           << " threads)",
         zpt::info);

    zpt::SYSTEM_EVENTS_RESOLVER()->add<::transport_engine_stop_threads>(
      zpt::system_event_type::EXITING);
}

/** @brief Plugin unload entry point: shuts down the transport engine.
 * @param _plugin Plugin instance.
 * @return void. */
extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Stopped multi-transport engine", zpt::info);
}
