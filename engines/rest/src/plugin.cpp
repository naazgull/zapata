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
#include <zapata/rest.h>
#include <zapata/rest/services.h>
#include <zapata/startup.h>
#include <zapata/transport.h>

namespace {
class rest_resolver_minion_shutdown : public zpt::system_event {
  public:
    using zpt::system_event::system_event;
    ~rest_resolver_minion_shutdown() = default;

    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state override {
        zpt::SYSTEM_EVENTS_RESOLVER()->remove<::rest_resolver_minion_shutdown>(
          zpt::system_event_type::EXITING);

        auto _config = zpt::GLOBAL_CONFIG();
        if (_config("transport")("default")->ok() && _config("upnp")->ok()) {
            zpt::rest::services::broadcast("/minions/shutdown", _config);
        }
        reinterpret_cast<zpt::rest::resolver_t&>(*zpt::REST_RESOLVER().get()).clear();

        return zpt::events::finish;
    }
};
} // namespace

/** @brief Plugin entry point: registers the REST resolver and builtin minion event handlers.
 * @param _plugin Plugin instance.
 * @return void. */
extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    auto _config = zpt::GLOBAL_CONFIG();
    zpt::TRANSPORT_ENGINE() //
      ->add_resolver(zpt::REST_RESOLVER(_config));

    if (_config("rest")("prefix")->ok()) {
        _config["rest"]["prefix_path_len"] =
          zpt::json::integer(zpt::split(_config("rest")("prefix")->string(), "/")->size());
    }
    else { _config["rest"]["prefix_path_len"] = 0; }

    zpt::REST_RESOLVER() //
      ->add<zpt::rest::minion_boot>(zpt::Notify, "/minions/boot")
      .add<zpt::rest::minion_shutdown>(zpt::Notify, "/minions/shutdown")
      .add<zpt::rest::minion_hello>("/minions/hello")
      .add<zpt::rest::minion_state>("/minions/state");

    if (_config("transport")("default")->ok() && _config("upnp")->ok()) {
        zpt::rest::services::broadcast("/minions/boot", _config);
    }

    zpt::SYSTEM_EVENTS_RESOLVER()->add<::rest_resolver_minion_shutdown>(
      zpt::system_event_type::EXITING);

    zlog("Added REST event resolver", zpt::info);
}

/** @brief Plugin unload entry point: removes REST resolver and builtin handlers.
 * @param _plugin Plugin instance.
 * @return void. */
extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    auto _config = zpt::GLOBAL_CONFIG();
    zlog("Disposing REST event resolver", zpt::info);

    zpt::REST_RESOLVER() //
      ->remove<zpt::rest::minion_boot>(zpt::Notify, "/minions/boot")
      .remove<zpt::rest::minion_shutdown>(zpt::Notify, "/minions/shutdown")
      .remove<zpt::rest::minion_hello>("/minions/hello")
      .remove<zpt::rest::minion_state>("/minions/state");

    zpt::TRANSPORT_ENGINE() //
      ->remove_resolver(zpt::REST_RESOLVER());

    expect(zpt::REST_RESOLVER()->count() == 0,
           zpt::REST_RESOLVER()->count()
             << " callbacks still registered in REST resolver, it usually leads to segmentation "
                "faults due to dynamic library unloading");
}
