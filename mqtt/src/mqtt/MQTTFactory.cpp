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

#include <zapata/mqtt/MQTTFactory.h>

zpt::MQTTFactory::MQTTFactory() : zpt::ChannelFactory() {}

zpt::MQTTFactory::~MQTTFactory() {}

auto zpt::MQTTFactory::produce(zpt::json _options) -> zpt::socket {
	zpt::socket _return;
	auto _found = this->__channels.find(_options["connection"]->str());
	if (_found != this->__channels.end()) {
		_return = _found->second;
	} else {
		zpt::MQTT* _mqtt = new zpt::MQTT();
		_mqtt->connect(zpt::uri::parse(_options["connection"]->str()));
		_return = zpt::socket(_mqtt);
	}
	return _return;
}

auto zpt::MQTTFactory::is_reusable(std::string _type) -> bool  {
	return true;
}

auto zpt::MQTTFactory::clean(zpt::socket _socket) -> bool  {
	return false;
}

extern "C" void _zpt_load_() {
	zpt::ev::emitter_factory _emitter = zpt::emitter();
	_emitter->channel({
	    {"zmq", zpt::socket_factory(new zpt::MQTTFactory())},
	});
}
