lib_LTLIBRARIES = libzapata-bridge-base.la

libzapata_bridge_base_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json
libzapata_bridge_base_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_bridge_base_la_CPPFLAGS = -O3 -std=c++20 -Wall -I../../../include -I../../include

libzapata_bridge_base_la_SOURCES = \
./bridge.cpp

bin_PROGRAMS = zbridge

zbridge_LDADD = -lpthread -lzapata-base -lzapata-lex-json libzapata-bridge-base.la
zbridge_LDFLAGS =
zbridge_CPPFLAGS = -O3 -std=c++20 -Wall -I../../../include -I../../include

zbridge_SOURCES = \
./main.cpp
