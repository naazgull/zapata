lib_LTLIBRARIES = libzapata-net-local-file.la					\
libzapata-net-local-file-plugin.la libzapata-net-local-unix.la	\
libzapata-net-local-unix-plugin.la

libzapata_net_local_file_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-io-streams -lzapata-transport
libzapata_net_local_file_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_net_local_file_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

libzapata_net_local_file_la_SOURCES = \
./file/file.cpp

libzapata_net_local_file_plugin_la_LIBADD = -lpthread -lzapata-base		\
-lzapata-io-streams -lzapata-transport libzapata-net-local-file.la
libzapata_net_local_file_plugin_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_net_local_file_plugin_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

libzapata_net_local_file_plugin_la_SOURCES = \
./file/plugin.cpp

libzapata_net_local_unix_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-io-streams -lzapata-net-socket -lzapata-transport
libzapata_net_local_unix_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_net_local_unix_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

libzapata_net_local_unix_la_SOURCES = \
./unix/unix.cpp

libzapata_net_local_unix_plugin_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-io-streams -lzapata-net-socket -lzapata-transport			\
libzapata-net-local-unix.la
libzapata_net_local_unix_plugin_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_net_local_unix_plugin_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

libzapata_net_local_unix_plugin_la_SOURCES = \
./unix/plugin.cpp
