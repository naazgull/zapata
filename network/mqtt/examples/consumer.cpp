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
#include <zapata/events.h>
#include <zapata/net/mqtt.h>
#include <zapata/ontology.h>
#include <zapata/rest.h>
#include <zapata/rest/services.h>
#include <zapata/startup.h>
#include <zapata/transport.h>

class on_message : public zpt::events::process {
  public:
    using zpt::events::process::process;
    ~on_message() = default;

    auto blocked() const -> bool { return false; }

    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        zlog(zpt::pretty{ this->received()->body() }, zpt::debug);
        ++_count;
        if (_count < 5) {
            auto _config = zpt::GLOBAL_CONFIG();
            auto _prefix =
              _config("rest")("prefix")->ok() ? _config("rest")("prefix")->string() : "";
            auto _to_publish = zpt::make_message<zpt::json_message>();
            _to_publish //
              ->performative(zpt::Inform)
              .uri(std::format("{}/test/topic", _prefix))
              .body() = {
                "from", "self", "date", zpt::json::date(), "id", zpt::uuid{}.to_string()
            };
            // zpt::MQTT_STREAM()->publish(_to_publish);
            zpt::make_call<zpt::events::discard>(zpt::REST_RESOLVER(), _to_publish);
        }
        return zpt::events::finish;
    }

  private:
    size_t _count{ 0 };
};

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading module 'mqtt_plugin_client'", zpt::info);
    auto _config = zpt::GLOBAL_CONFIG();
    auto _resolver = zpt::REST_RESOLVER();
    auto _prefix = _config("rest")("prefix")->ok() ? _config("rest")("prefix")->string() : "";
    _resolver //
      ->add<on_message>(std::format("{}/test/topic", _prefix));
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    auto _config = zpt::GLOBAL_CONFIG();
    auto _resolver = zpt::REST_RESOLVER();
    auto _prefix = _config("rest")("prefix")->ok() ? _config("rest")("prefix")->string() : "";
    _resolver //
      ->remove<on_message>(std::format("{}/test/topic", _prefix));
    zlog("Unloading module 'mqtt_plugin_client'", zpt::info);
}
