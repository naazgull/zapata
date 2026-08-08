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

class ws_test_client_boot : public zpt::system_event {
  public:
    using zpt::system_event::system_event;
    ~ws_test_client_boot() = default;

    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state override {
        if (this->__received->body()("_id")->ok() &&
            this->__received->body()("_id")->string().find("/ws/chat") != std::string::npos) {
            zpt::SYSTEM_EVENTS_RESOLVER() //
              ->remove<ws_test_client_boot>(zpt::system_event_type::REGISTERED_REMOTE_SERVICE);

            auto _polling = zpt::STREAM_POLLING();
            auto _test_message = zpt::TRANSPORT_LAYER() //
                                   .get("http")
                                   ->make_request();
            _test_message //
              ->performative(zpt::Get)
              .uri("/ws/chat")
              .header("Connection", "upgrade")
              .header("Upgrade", "websocket");

            auto _context = zpt::make_call(zpt::REST_RESOLVER(), _test_message);

            while (!_polling->is_in_shutdown() && !_context->is_replied()) {
                std::this_thread::yield();
            }

            if (_context->is_replied()) {
                expect(_context->reply()->body()("stream")->is_string(), "No stream identifier");
                auto _transport = zpt::TRANSPORT_LAYER().get("ws");
                auto _stream =
                  _polling->mute(zpt::uuid{ _context->reply()->body()("stream")->string() });

                _test_message = _transport->make_request();
                _test_message //
                  ->performative(zpt::Post)
                  .uri("/ws/chat")
                  .body() = { "message", "Hello there" };
                _transport->send(_stream, _test_message);
                _polling->unmute(_stream);

                _test_message = _transport->make_request();
                _test_message //
                  ->performative(zpt::Post)
                  .uri("ws:/ws/chat")
                  .body() = { "message", "Hello there again" };
                zpt::make_call(zpt::REST_RESOLVER(), _test_message);
            }
        }
        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading 'ws_test_consumer'", zpt::info);
    zpt::SYSTEM_EVENTS_RESOLVER() //
      ->add<ws_test_client_boot>(zpt::system_event_type::REGISTERED_REMOTE_SERVICE);
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloading module 'ws_test_consumer'", zpt::info);
}
