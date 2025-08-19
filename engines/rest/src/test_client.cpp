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
#include <zapata/rest/services.h>
#include <zapata/transport.h>

class test_client_service : public zpt::events::process {
  public:
    test_client_service(zpt::message _received)
      : zpt::events::process{ _received } {}
    ~test_client_service() = default;

    auto blocked() const -> bool { return false; }

    auto operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
      -> zpt::events::state {
        zlog(zpt::pretty{ this->received()->body() }, zpt::debug);
        return zpt::events::finish;
    }
};

class test_client_boot : public zpt::events::process {
  public:
    test_client_boot(zpt::message _received)
      : zpt::events::process{ _received } {}
    ~test_client_boot() = default;

    auto blocked() const -> bool { return false; }

    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state {
        auto _config = zpt::GLOBAL_CONFIG();
        auto _prefix = _config("rest")("prefix")->ok() ? _config("rest")("prefix")->string() : "";
        auto _test_message = zpt::TRANSPORT_LAYER() //
                               .get("tcp")
                               ->make_request();
        _test_message //
          ->performative(zpt::Post)
          .uri(std::format("{}/test_plugin", _prefix))
          .body() = { "test", "something" };

        _dispatcher->trigger<zpt::events::call<test_client_service>>(zpt::REST_RESOLVER(),
                                                                     _test_message);

        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Sending request to 'test_plugin'", zpt::info);
    zpt::REST_RESOLVER()->add<test_client_boot>(zpt::Notify, "/minions/boot");
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloading module 'test_plugin_client'", zpt::info);
    zpt::REST_RESOLVER()->remove<test_client_boot>(zpt::Notify, "/minions/boot");
}
