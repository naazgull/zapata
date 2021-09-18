lib_LTLIBRARIES = libzapata-net-local-file.la					\
libzapata-net-local-file-plugin.la libzapata-net-local-unix.la	\
libzapata-net-local-unix-plugin.la

libzapata_net_local_file_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-io-streams -lzapata-transport
libzapata_net_local_file_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs						\
-L@PROJECT_DIR@/common/events/build/src/events/.libs				\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs			\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
libzapata_net_local_file_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall			\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/uri/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/network/transport/include									\
-I@PROJECT_DIR@/engines/startup/include
libzapata_net_local_file_la_SOURCES = \
./file/file.cpp

libzapata_net_local_file_plugin_la_LIBADD = -lpthread -lzapata-base		\
-lzapata-io-streams -lzapata-transport libzapata-net-local-file.la
libzapata_net_local_file_plugin_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs							\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs					\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs						\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs							\
-L@PROJECT_DIR@/io/socket/build/src/socket/.libs \
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs							\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs								\
-L@PROJECT_DIR@/common/events/build/src/events/.libs						\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs					\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
libzapata_net_local_file_plugin_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/uri/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/network/transport/include									\
-I@PROJECT_DIR@/engines/startup/include
libzapata_net_local_file_plugin_la_SOURCES = \
./file/plugin.cpp

libzapata_net_local_unix_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-io-streams -lzapata-net-socket -lzapata-transport
libzapata_net_local_unix_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs					\
-L@PROJECT_DIR@/io/socket/build/src/socket/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs						\
-L@PROJECT_DIR@/common/events/build/src/events/.libs				\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs			\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
libzapata_net_local_unix_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall			\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/uri/include	\
-I@PROJECT_DIR@/io/socket/include -I@PROJECT_DIR@/common/events/include		\
-I@PROJECT_DIR@/network/transport/include									\
-I@PROJECT_DIR@/engines/startup/include
libzapata_net_local_unix_la_SOURCES = \
./unix/unix.cpp

libzapata_net_local_unix_plugin_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-io-streams -lzapata-net-socket -lzapata-transport			\
libzapata-net-local-unix.la
libzapata_net_local_unix_plugin_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs							\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs					\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs						\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs							\
-L@PROJECT_DIR@/io/socket/build/src/socket/.libs							\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs							\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs								\
-L@PROJECT_DIR@/common/events/build/src/events/.libs						\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs					\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
libzapata_net_local_unix_plugin_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/io/socket/include -I@PROJECT_DIR@/parsers/json/include		\
-I@PROJECT_DIR@/parsers/uri/include -I@PROJECT_DIR@/common/events/include	\
-I@PROJECT_DIR@/network/transport/include									\
-I@PROJECT_DIR@/engines/startup/include
libzapata_net_local_unix_plugin_la_SOURCES = \
./unix/plugin.cpp
