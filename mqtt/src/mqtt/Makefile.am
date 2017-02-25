
lib_LTLIBRARIES = libzapata-mqtt.la

libzapata_mqtt_la_LIBADD = -lpthread -lmosquitto -lzapata-base -lzapata-json
libzapata_mqtt_la_LDFLAGS = -version-info 9:6:9 
libzapata_mqtt_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_mqtt_la_SOURCES = \
./MQTT.cpp
