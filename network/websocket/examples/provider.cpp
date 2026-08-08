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

namespace {
inline std::shared_mutex ___mutex;
inline zpt::uuid ___stream_id;
} // namespace

class ws_example_endpoint : public zpt::events::process {
  public:
    using zpt::events::process::process;
    ~ws_example_endpoint() = default;

    auto blocked() const -> bool { return false; }

    auto operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
      -> zpt::events::state {
        std::unique_lock _guard{ ::___mutex };
        ::___stream_id = this->stream()->uuid();
        this
          ->to_send() //
          ->status(200)
          .body() = { "echo", this->received()->body() };
        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    zlog("Registering listeners for module 'ws_test'", zpt::info);
    auto _resolver = zpt::REST_RESOLVER();
    _resolver->add<ws_example_endpoint>("/ws/chat");

    auto _null_id = ___stream_id;
    _plugin.add_thread([=]() mutable -> void {
        zpt::this_thread::name("ws_test_sender");
        auto _polling = zpt::STREAM_POLLING();
        auto _transport = zpt::TRANSPORT_LAYER() //
                            .get("ws");

        while (!_polling->is_in_shutdown()) {
            std::shared_lock _guard{ ::___mutex };
            if (___stream_id != _null_id) {
                try {
                    auto _stream = _polling->mute(___stream_id);
                    auto _message = _transport->make_request();
                    _message //
                      ->performative(zpt::Post)
                      .uri("/ws/chat")
                      .body() = { "echo", std::format("Message for {}", ___stream_id.to_string()) };

                    _transport->send(_stream, _message);
                    _polling->unmute(_stream);
                    _null_id = ___stream_id;
                }
                catch (...) {
                }
            }
            std::this_thread::yield();
        }
    });
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloading module 'ws_test'", zpt::info);
    auto _resolver = zpt::REST_RESOLVER();
    _resolver->remove<ws_example_endpoint>("/ws/chat");
}
