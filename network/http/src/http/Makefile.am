lib_LTLIBRARIES = libzapata-net-http.la

libzapata_net_http_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json -lzapata-lex-http -lzapata-net-socket -lzapata-transport
libzapata_net_http_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_net_http_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_net_http_la_SOURCES = \
./http.cpp \
./plugin.cpp
