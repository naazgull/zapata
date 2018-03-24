
lib_LTLIBRARIES = libzapata-mqtt.la libzapata-mqtt-factory.la

libzapata_mqtt_la_LIBADD = -lpthread -lmosquitto -lzapata-base -lzapata-json-lzapata-events
libzapata_mqtt_la_LDFLAGS = -version-info 1:0:0
libzapata_mqtt_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_mqtt_la_SOURCES = \
./MQTT.cpp

libzapata_mqtt_factory_la_LIBADD = -lpthread -lmosquitto -lzapata-base -lzapata-json -lzapata-events -lzapata-mqtt
libzapata_mqtt_factory_la_LDFLAGS = -version-info 1:0:0 -L../mqtt/.libs
libzapata_mqtt_factory_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_mqtt_factory_la_SOURCES = \
./MQTTFactory.cpp
