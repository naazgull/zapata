/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <zapata/upnp/UPnPFactory.h>

zpt::UPnPFactory::UPnPFactory() : zpt::ChannelFactory() {}

zpt::UPnPFactory::~UPnPFactory() {}

auto zpt::UPnPFactory::produce(zpt::json _options) -> zpt::socket {
	zpt::socket _return;
	auto _found = this->__channels.find(_options["connection"]->str());
	if (_found != this->__channels.end()) {
		_return = _found->second;
	} else {
		zpt::UPnP* _upnp = new zpt::UPnP(_options);
		_return = zpt::socket(_upnp);
	}
	return _return;
}

auto zpt::UPnPFactory::is_reusable(std::string _type) -> bool { return true; }

auto zpt::UPnPFactory::clean(zpt::socket _socket) -> bool { return false; }

extern "C" void _zpt_load_() {
	zpt::ev::emitter_factory _emitter = zpt::emitter();
	zpt::socket_factory _factory(new zpt::UPnPFactory());
	_emitter->channel({
	    {"upnp", _factory},
	});
	zpt::json _options = _emitter->options();

	if (!_options["discoverable"]->ok()) {
		_options["rest"] << "discoverable" << false;
	}

	if (bool(_options["discoverable"])) {
		zpt::socket _upnp = _factory->produce(_options["upnp"]);
		zpt::poll::instance<zpt::ChannelPoll>()->poll(_upnp);

		zlog(std::string("binding ") + _upnp->protocol() + std::string(" listener to ") +
			 std::string(_upnp->uri()["scheme"]) + std::string("://") +
			 std::string(_upnp->uri()["domain"]) + std::string(":") +
			 std::string(_upnp->uri()["port"]),
		     zpt::info);
	}
}
