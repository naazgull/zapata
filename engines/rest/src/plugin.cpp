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
#include <zapata/transport.h>

namespace {
auto register_service_broadcast_listeners(zpt::json) -> void {
    auto& _resolver = zpt::global_cast<zpt::rest::resolver>(zpt::REST_RESOLVER());
    _resolver->add<zpt::rest::service_broadcast>(zpt::Notify, "/services");
}
} // namespace

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    auto _config = zpt::global_cast<zpt::json>(zpt::GLOBAL_CONFIG());
    zpt::global_cast<zpt::transports::engine>(zpt::TRANSPORT_ENGINE()) //
      .add_resolver(zpt::make_global<zpt::rest::resolver>(zpt::REST_RESOLVER(),
                                                          new zpt::rest::resolver_t(_config)));
    if (_config("rest")("prefix")->ok()) {
        _config["rest"]["prefix_path_len"] =
          zpt::json::integer(zpt::split(_config("rest")("prefix")->string(), "/")->size());
    }
    else { _config["rest"]["prefix_path_len"] = 0; }
    ::register_service_broadcast_listeners(_config);
    zlog("Added REST event resolver", zpt::info);
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Disposing REST event resolver", zpt::info);
    zpt::release_global<zpt::rest::resolver>(zpt::REST_RESOLVER());
}
