lib_LTLIBRARIES = libzapata-net-pipe.la libzapata-net-pipe-plugin.la

libzapata_net_pipe_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-io-streams -lzapata-transport
libzapata_net_pipe_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_net_pipe_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../../include -I../../include

libzapata_net_pipe_la_SOURCES = \
./pipe.cpp

libzapata_net_pipe_plugin_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-io-streams -lzapata-transport libzapata-net-pipe.la
libzapata_net_pipe_plugin_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_net_pipe_plugin_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../../include -I../../include

libzapata_net_pipe_plugin_la_SOURCES = \
./plugin.cpp
