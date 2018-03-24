/*
Copyright (c) 2017, Muzzley
*/

/**
 * This example uses the mosquitto C library (http://mosquitto.org).
 * In Ubuntu based systems is installable by executing:
 * $ sudo apt-get install libmosquitto0 libmosquitto0-dev
 *
 * Compile with '-lmosquitto'.
 */
#pragma once

#include <unistd.h>
#include <iostream>
#include <functional>
#include <memory>
#include <map>
#include <string>
#include <zapata/mqtt/MQTT.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

class MQTTFactory : public zpt::ChannelFactory {
      public:
	MQTTFactory();
	virtual ~MQTTFactory();
	virtual auto produce(zpt::json _options) -> zpt::socket;
	virtual auto is_reusable() -> bool;
	virtual auto clean(zpt::socket _socket) -> bool;

      private:
	std::map<std::string, zpt::socket> __channels;
};
}
