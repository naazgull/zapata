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

#include <zapata/rest/rest.h>
#include <zapata/http.h>
#include <zapata/net/socket/socket_stream.h>

auto zpt::REST_RESOLVER() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::rest::resolver_t::resolver_t(zpt::json _rest_config)
  : __configuration{ _rest_config } {
    this->__broadcast_stream =
      zpt::make_stream<zpt::socketstream>(this->__configuration("broadcast")("host")->string(),
                                          this->__configuration("broadcast")("port")->integer(),
                                          false,
                                          IPPROTO_UDP);
}

auto zpt::rest::resolver_t::resolve(zpt::message _received,
                                    zpt::events::initializer_t _initializer) const
  -> std::list<zpt::event> {
    std::list<zpt::event> _return;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wrestrict"
    auto _to_search = std::string{ "/" } + zpt::ontology::to_str(_received->performative()) +
                      _received->resource()->string();
#pragma GCC diagnostic pop
    for (auto [_, __, _record] : this->__catalog.search(_to_search)) {
        auto _hash_code = _record("metadata")("callback")->integer();
        expect(static_cast<unsigned>(_hash_code) < this->__callbacks.size(),
               "Couldn't find callback for [" << _hash_code << "]("
                                              << _received->resource()->string() << ")");
        _return.push_back(this->__callbacks[_hash_code](_received, _initializer));
    }
    expect(_return.size() != 0,
           "Couldn't find callback for (" << _received->resource()->string() << ")");
    return _return;
}

auto zpt::rest::resolver_t::broadcast_service(zpt::performative _performative,
                                              std::string _path,
                                              zpt::json _metadata) -> void {
    if (_path != "*") {
        auto _service = zpt::make_message<zpt::http::basic_request>();
        _service->performative(zpt::Notify);
        _service->uri("*");
        _service->headers() << "Content-Type"
                            << "application/json";
        _service->body() = zpt::json{ "resource",     _path,
                                      "performative", static_cast<int>(_performative),
                                      "uri",          this->__configuration("uri") };

        std::cout << _service << std::endl;

        // (*this->__broadcast_stream) << _service << std::flush;
    }
}

zpt::rest::service_broadcast::service_broadcast(zpt::message _received)
  : zpt::events::process{ _received } {}

auto zpt::rest::service_broadcast::blocked() const -> bool { return false; }

auto zpt::rest::service_broadcast::operator()(zpt::events::dispatcher& _dispatcher)
  -> zpt::events::state {
    zlog(this->received(), zpt::debug);
    // auto& _resolver = zpt::global_cast<zpt::rest::resolver>(zpt::REST_RESOLVER());
    // _resolver.add<
    return zpt::events::finish;
}
