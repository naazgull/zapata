lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lsystemd -lzapata-base -lzapata-json -lzapata-http -lzapata-events
libzapata_rest_la_LDFLAGS = -version-info 1:0:0
libzapata_rest_la_CPPFLAGS = -O3 -std=c++17 -I../../include -I/usr/include/python3.6 -fwrapv

libzapata_rest_la_SOURCES = \
./RESTProtocol.cpp \
./RESTEmitter.cpp
