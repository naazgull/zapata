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
#include <zapata/ontology.h>
#include <zapata/rest.h>
#include <zapata/rest/services.h>
#include <zapata/startup.h>
#include <zapata/transport.h>

class test_plugin_collection : public zpt::events::process {
  public:
    test_plugin_collection(zpt::message _received, zpt::call_context::ptr _context)
      : zpt::events::process{ _received, _context } {}
    ~test_plugin_collection() = default;

    auto blocked() const -> bool { return false; }

    auto operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
      -> zpt::events::state {
        this
          ->to_send() //
          ->status(200)
          .body() = { "echo", this->received()->body() };
        return zpt::events::finish;
    }
};

class test_redirect : public zpt::events::process {
  public:
    test_redirect(zpt::message _received, zpt::call_context::ptr _context)
      : zpt::events::process{ _received, _context } {}
    ~test_redirect() = default;

    auto blocked() const -> bool {
        return this->context() != nullptr && !this->context()->is_replied();
    }

    auto operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
      -> zpt::events::state {
        if (this->context() == nullptr) {
            this->context() = zpt::make_call(zpt::REST_RESOLVER(), this->received());
        }
        else if (this->context()->is_replied()) {
            this
              ->to_send() //
              ->status(this->context()->reply()->status())
              .body() = this->context()->reply()->body();
            return zpt::events::finish;
        }
        return zpt::events::retrigger;
    }
};

class test_client_service : public zpt::events::process {
  public:
    test_client_service(zpt::message _received, zpt::call_context::ptr _context)
      : zpt::events::process{ _received, _context } {}
    ~test_client_service() = default;

    auto blocked() const -> bool { return false; }

    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        zlog("Received response:\n" << zpt::pretty{ this->received()->body() }, zpt::debug);
        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Registering listeners for module 'test_plugin'", zpt::info);
    auto _config = zpt::GLOBAL_CONFIG();
    auto _resolver = zpt::REST_RESOLVER();
    auto _prefix = _config("rest")("prefix")->ok() ? _config("rest")("prefix")->string() : "";
    _resolver //
      ->add<test_plugin_collection>(std::format("{}/test_plugin", _prefix))
      .add<test_plugin_collection>(std::format("{}/test_redirect", _prefix));

    auto _test_message = zpt::TRANSPORT_LAYER() //
                           .get("tcp")
                           ->make_request();
    _test_message //
      ->performative(zpt::Post)
      .uri(std::format("{}/test_plugin", _prefix))
      .body() = { "from", "self", "date", zpt::json::date(), "id", zpt::generate::r_uuid() };
    zpt::make_call<test_client_service>(zpt::REST_RESOLVER(), _test_message);

    _test_message = zpt::TRANSPORT_LAYER() //
                      .get("tcp")
                      ->make_request();
    _test_message //
      ->performative(zpt::Post)
      .uri(std::format("{}/test_redirect", _prefix))
      .body() = { "from", "self", "date", zpt::json::date(), "id", zpt::generate::r_uuid() };
    zpt::make_call<test_client_service>(zpt::REST_RESOLVER(), _test_message);
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloading module 'test_plugin'", zpt::info);
    auto _config = zpt::GLOBAL_CONFIG();
    auto _resolver = zpt::REST_RESOLVER();
    auto _prefix = _config("rest")("prefix")->ok() ? _config("rest")("prefix")->string() : "";
    _resolver //
      ->remove<test_plugin_collection>(std::format("{}/test_plugin", _prefix))
      .remove<test_plugin_collection>(std::format("{}/test_redirect", _prefix));
}
