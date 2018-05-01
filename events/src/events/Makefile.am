lib_LTLIBRARIES = libzapata-events.la

libzapata_events_la_LIBADD = -lpthread -lmagic -lzmq -lzapata-base -lzapata-json
libzapata_events_la_LDFLAGS = -version-info 1:0:0
libzapata_events_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_events_la_SOURCES = \
./EventEmitter.cpp \
./Polling.cpp \
./Connector.cpp
