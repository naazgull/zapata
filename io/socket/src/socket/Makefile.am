lib_LTLIBRARIES = libzapata-net-socket.la

libzapata_net_socket_la_LIBADD = -lpthread -lzapata-base -lzapata-common-lockfree -lzapata-io-streams
libzapata_net_socket_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_net_socket_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_net_socket_la_SOURCES = \
./socket_stream.cpp

bin_PROGRAMS = zsocks zsockc

zsocks_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree -lzapata-io-streams libzapata-net-socket.la
zsocks_LDFLAGS=
zsocks_CPPFLAGS = -std=c++17 -Wall -I../../include

zsocks_SOURCES = \
./server.cpp

zsockc_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree -lzapata-io-streams libzapata-net-socket.la
zsockc_LDFLAGS=
zsockc_CPPFLAGS = -std=c++17 -Wall -I../../include

zsockc_SOURCES = \
./client.cpp
