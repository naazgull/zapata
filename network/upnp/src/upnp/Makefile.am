
lib_LTLIBRARIES = libzapata-upnp.la

libzapata_upnp_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json -lzapata-lex-http
libzapata_upnp_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_upnp_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_upnp_la_SOURCES = \
./UPnP.cpp \
./UPnPFactory.cpp
