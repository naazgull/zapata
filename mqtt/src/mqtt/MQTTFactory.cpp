/*
  Copyright (c) 2017, Muzzley
*/

#include <zapata/mqtt/MQTTFactory.h>

zpt::MQTTFactory::MQTTFactory() : zpt::ChannelFactory() {}

zpt::MQTTFactory::~MQTTFactory() {}

auto zpt::MQTTFactory::produce(zpt::json _options) -> zpt::socket {
	std::string _connection;
	zpt::socket _return;
	auto _found = this->__channels.find(_connection);
	if (_found != this->__channels.end()) {
		_return = _found->second;
	} else {
		zpt::MQTT* _mqtt = new zpt::MQTT();
		_mqtt->connect(_options);
		_return = zpt::socket(_mqtt);
	}
	return _return;
}

auto is_reusable() -> bool  {
	return false;
}

auto clean(zpt::socket _socket) -> bool  {
	return false;
}

extern "C" void _zpt_load_() {
	zpt::ev::emitter_factory _emitter = zpt::emitter();
	_emitter->channel({
	    {"mqtt", zpt::socket_factory(new zpt::MQTTFactory())},
	});
}
