lib_LTLIBRARIES = libzapata-transport.la

libzapata_transport_la_LIBADD = -lpthread -lzapata-base -lzapata-io-streams -lzapata-lex-json
libzapata_transport_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_transport_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_transport_la_SOURCES = \
./transport.cpp

bin_PROGRAMS = ztransport

ztransport_LDADD = -lpthread -lzapata-base -lzapata-io-streams -lzapata-lex-json -lzapata-lex-http -lzapata-net-socket libzapata-transport.la
ztransport_LDFLAGS =
ztransport_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

ztransport_SOURCES = \
./main.cpp
