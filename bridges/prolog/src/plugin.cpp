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
#include <zapata/prolog.h>
#include <zapata/startup.h>

namespace {
/** @brief Loads the dynamic Prolog bindings library (libzapata_bridge_prolog_bindings). */
auto register_bindings() -> void {
    auto& _bridge = zpt::PROLOG_BRIDGE();
    _bridge.call(zpt::prolog::term{ std::format(
      "use_foreign_library(\"{}/lib/libzapata_bridge_prolog_bindings\")", ZPT_INSTALL_PREFIX) });
}
} // namespace

/**
 * @brief System event handler that executes configured Prolog goals after boot.
 *
 * Iterates over the `exec` configuration entries and calls the specified Prolog goals
 * via the bridge. Removes itself from the event resolver after execution.
 */
class execute_after_boot : public zpt::system_event {
  public:
    using zpt::system_event::system_event;
    ~execute_after_boot() = default;

    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state override {
        auto& _bridge = zpt::PROLOG_BRIDGE().thread_instance();

        for (auto&& [_, __, _execute] : _bridge.options()("exec")) {
            zlog("Executing `" << _execute->string() << "`", zpt::info);
            _bridge.call(zpt::prolog::term{ _execute->string() });
        }

        zpt::SYSTEM_EVENTS_RESOLVER()->remove<execute_after_boot>(
          zpt::system_event_type::FINISHED_BOOT);

        return zpt::events::finish;
    }
};

/** @brief Plugin load callback: initializes the Prolog bridge and registers the `zpt` module. */
extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    auto& _bridge = zpt::PROLOG_BRIDGE(zpt::GLOBAL_CONFIG()("self")("cmd")->string());
    _bridge.set_options(_plugin.config());

    _bridge //
      .add_module(::register_bindings, { "module", "zpt" });

    if (_bridge.options()("modules")->is_array()) {
        for (auto&& [_, __, _module] : _bridge.options()("modules")) {
            _bridge.add_module(_module("file")->string(), _module);
        }
    }

    if (_bridge.options()("exec")->is_array()) {
        zpt::SYSTEM_EVENTS_RESOLVER()->add<execute_after_boot>(
          zpt::system_event_type::FINISHED_BOOT);
    }

    zlog("Initialized PROLOG bridge", zpt::info);
}

/** @brief Plugin unload callback: cleans up the Prolog bridge state. */
extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zpt::PROLOG_BRIDGE().cleanup();
    zlog("Unloaded PROLOG bridge", zpt::info);
}
