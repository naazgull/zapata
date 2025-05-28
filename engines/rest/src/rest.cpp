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

zpt::rest::resolver_t::resolver_t(zpt::json _global_config)
  : __configuration{ _global_config } {}

auto zpt::rest::resolver_t::clear() -> zpt::rest::resolver_t& {
    this->__callbacks.clear();
    return (*this);
}

auto zpt::rest::resolver_t::resolve(zpt::message _received, zpt::events::initializer_t _initializer)
  const -> std::list<zpt::event> {
    std::list<zpt::event> _return;
    auto _to_search = std::format(
      "/{}{}", zpt::ontology::to_str(_received->performative()), _received->resource()->string());
    for (auto [_, __, _record] : this->__catalog.search(_to_search)) {
        auto _hash_code = _record("hash")->integer();
        expect(static_cast<unsigned>(_hash_code) < this->__callbacks.size(),
               "Couldn't find callback for [" << _hash_code << "]("
                                              << _received->resource()->string() << ")");
        _return.push_back(this->__callbacks[_hash_code](_received, _initializer));
    }
    expect(_return.size() != 0,
           "Couldn't find callback for (" << _received->resource()->string() << ")");
    return _return;
}

auto zpt::rest::resolver_t::set_broadcast_stream(zpt::stream _broadcast_stream) -> void {
    this->__broadcast_stream = _broadcast_stream;
}

auto zpt::rest::resolver_t::broadcast_services() -> void {
    if (this->__broadcast_stream == nullptr) { return; }

    // auto _service = zpt::allocate_message<zpt::upnp::basic_request>();
    // _service->performative(zpt::Notify);
    // _service->uri("/services");
    // _service->headers() << "Content-Type" << "application/json" << "ST"
    //                     << "urn:schemas-upnp-org:service:*" << "MAN" << "\"ssdp:discover\"" << "MX"
    //                     << "3";

    // auto _resources = zpt::json::array();
    // _service->body() = zpt::json{
    //     "resources", _resources, "addresses", this->__configuration("transport")("addresses")
    // };
    // (*this->__broadcast_stream) << _service << std::flush;
}

zpt::rest::service_broadcast::service_broadcast(zpt::message _received)
  : zpt::events::process{ _received } {}

auto zpt::rest::service_broadcast::blocked() const -> bool { return false; }

auto zpt::rest::service_broadcast::operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
    zlog(this->received(), zpt::debug);
    // auto& _resolver = zpt::REST_RESOLVER();
    // _resolver.add<
    return zpt::events::finish;
}

auto zpt::REST_RESOLVER(zpt::json _config) -> zpt::rest::resolver {
    static zpt::rest::resolver _global = std::make_shared<zpt::rest::resolver_t>(_config);
    return _global;
}
