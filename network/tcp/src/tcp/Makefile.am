lib_LTLIBRARIES = libzapata-net-tcp.la libzapata-net-tcp-plugin.la

libzapata_net_tcp_la_LIBADD = -lpthread -lzapata-base -lzapata-io-streams	\
-lzapata-net-socket -lzapata-transport
libzapata_net_tcp_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_net_tcp_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_net_tcp_la_SOURCES = \
./tcp.cpp

libzapata_net_tcp_plugin_la_LIBADD = -lpthread -lzapata-base					\
-lzapata-net-socket -lzapata-io-streams -lzapata-transport libzapata-net-tcp.la
libzapata_net_tcp_plugin_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_net_tcp_plugin_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_net_tcp_plugin_la_SOURCES = \
./plugin.cpp
