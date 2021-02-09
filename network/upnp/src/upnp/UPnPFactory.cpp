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

#include <zapata/upnp/UPnPFactory.h>

zpt::UPnPFactory::UPnPFactory()
  : zpt::ChannelFactory() {}

zpt::UPnPFactory::~UPnPFactory() {}

auto
zpt::UPnPFactory::produce(zpt::json _options) -> zpt::socket {
    zpt::socket _return;
    auto _found = this->__channels.find(_options["connection"]->string());
    if (_found != this->__channels.end()) { _return = _found->second; }
    else {
        zpt::UPnP* _upnp = new zpt::UPnP(_options);
        _return = zpt::socket(_upnp);
    }
    return _return;
}

auto
zpt::UPnPFactory::is_reusable(std::string const& _type) -> bool {
    return true;
}

auto
zpt::UPnPFactory::clean(zpt::socket _socket) -> bool {
    return false;
}

extern "C" void
_zpt_plugin_load_() {
    zpt::ev::emitter_factory _emitter = zpt::emitter();
    zpt::channel_factory _factory(new zpt::UPnPFactory());
    _emitter->channel({ { "upnp", _factory }, { "upnps", _factory } });
    zpt::json _options = _emitter->options();

    if (_options["upnp"]->ok() && _options["upnp"]->is_array()) {
        for (auto _definition : _options["upnp"]->array()) {
            zpt::socket _upnp = _factory->produce(_definition["bind"]);
            zpt::poll::instance<zpt::ChannelPoll>()->poll(_upnp);

            zlog(std::string("binding ") + _upnp->protocol() + std::string(" listener to ") +
                   std::string(_upnp->uri()["scheme"]) + std::string("://") +
                   std::string(_upnp->uri()["domain"]) + std::string(":") +
                   std::string(_upnp->uri()["port"]),
                 zpt::info);
        }
    }

    _options["rest"] << "discoverable" << bool(_options["discoverable"]);
    if (bool(_options["discoverable"])) {}
}
