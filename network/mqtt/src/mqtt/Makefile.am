
lib_LTLIBRARIES = libzapata-mqtt.la libzapata-mqtt-factory.la

libzapata_mqtt_la_LIBADD = -lpthread -lmosquitto -lzapata-base -lzapata-lex-json -lzapata-common-events
libzapata_mqtt_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_mqtt_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -I../../../include -I../../include

libzapata_mqtt_la_SOURCES = \
./utils.cpp \
./MQTT.cpp

libzapata_mqtt_factory_la_LIBADD = -lpthread -lmosquitto -lzapata-base -lzapata-lex-json -lzapata-common-events -lzapata-mqtt
libzapata_mqtt_factory_la_LDFLAGS = -version-info 2:0:0 -shared -L../mqtt/.libs
libzapata_mqtt_factory_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -I../../../include -I../../include

libzapata_mqtt_factory_la_SOURCES = \
./MQTTFactory.cpp
