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
  : zpt::dom::engine{ 2, _configuration } {
    zpt::dom::engine::add_listener(
      0, "/paths", [=](zpt::pipeline::event<zpt::dom::element>& _event) mutable -> void {
          zpt::dom::element _root{ "/ROOT", "ROOT", this->__root["document"] };
          zpt::openapi::process_path(_event.content(), _root, this->__root["config"]);
      });
}

auto
zpt::openapi::engine::add_source(zpt::json _source) -> zpt::openapi::engine& {
    try {
        auto _uri = zpt::uri::parse(_source["$ref"]->str());
        auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
        auto& _transport = _layer.get(_uri["scheme"]->str());

        zpt::exchange _channel = _transport->resolve(_uri);
        _transport->send_request(_channel);
        _transport->receive_reply(_channel);
        zpt::json _document{ "config", _source, "document", _channel->received()["body"] };

        this->__sources << _source["$ref"]->str() << _document;
        this->evaluate_ref(_document["document"], "document", _document);
    }
    catch (...) {
    }
    return (*this);
}

auto
zpt::openapi::engine::traverse() -> zpt::openapi::engine& {
    for (auto [_, __, _document] : this->__sources) {
        if (_document["config"]["target"]->ok()) {
            this->__root = _document;
            zpt::dom::engine::traverse(_document["document"]);
        }
    }
    return (*this);
}

auto
zpt::openapi::engine::evaluate_ref(zpt::json _document, std::string _parent_key, zpt::json _parent)
  -> zpt::openapi::engine& {
    for (auto [_, _key, _value] : _document) {
        if (_key == "$ref") {
            if (!this->__sources[_value->str()]->ok()) {
                this->add_source({ _key, _value });
            }
            if (this->__sources[_value->str()]->ok()) {
                _parent << _parent_key << this->__sources[_value->str()]["document"];
            }
            return (*this);
        }
        else {
            this->evaluate_ref(_value, _key, _document);
        }
    }
    return (*this);
}

auto
zpt::openapi::process_path(zpt::dom::element& _element,
                           zpt::dom::element& _root,
                           zpt::json _source_config) -> void {
    zlog(zpt::pretty(_root.content()), zpt::info)
}
