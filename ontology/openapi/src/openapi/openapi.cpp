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

#include <zapata/openapi.h>
#include <zapata/streams/streams.h>
#include <zapata/transport/transport.h>

auto
zpt::OPENAPI_ENGINE() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::openapi::engine::engine(zpt::json _configuration)
  : zpt::dom::engine{ 2, _configuration } {}

auto
zpt::openapi::engine::add_source(zpt::json _source) -> zpt::openapi::engine& {
    auto _uri = zpt::uri::parse(_source["source"]->str());
    auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
    auto& _transport = _layer.get(_uri["scheme"]->str());

    zpt::exchange _channel = _transport->resolve(_uri);
    _transport->send_request(_channel);
    _transport->receive_reply(_channel);

    // zpt::json _document;
    // _request->stream() >> _document;
    // this->__sources << _uri << zpt::json{ "config", _source, "document", _document };
    zlog(_channel, zpt::info);

    return (*this);
}

auto
zpt::openapi::engine::traverse() -> zpt::openapi::engine& {
    zlog(this->__sources, zpt::debug);
    return (*this);
}

auto
zpt::openapi::engine::evaluate_ref(zpt::json _document) -> zpt::openapi::engine& {
    for (auto [_, _key, _value] : _document) {
        if (_key == "$ref") {
        }
    }
    return (*this);
}
