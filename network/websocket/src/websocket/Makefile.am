lib_LTLIBRARIES = libzapata-net-websocket.la

libzapata_net_websocket_la_LIBADD = -lpthread -lzapata-base -lzapata-io-streams -lzapata-net-socket
libzapata_net_websocket_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_net_websocket_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_net_websocket_la_SOURCES = \
./websocket.cpp

bin_PROGRAMS = zwebsocket

zwebsocket_LDADD = -lpthread -lzapata-io-streams -lzapata-net-socket libzapata-net-websocket.la
zwebsocket_LDFLAGS =
zwebsocket_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zwebsocket_SOURCES = \
./main.cpp
