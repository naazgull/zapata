lib_LTLIBRARIES = libzapata-net-websocket.la libzapata-net-websocket-plugin.la

libzapata_net_websocket_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-lex-json -lzapata-io-streams -lzapata-net-socket	\
-lzapata-transport
libzapata_net_websocket_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs					\
-L@PROJECT_DIR@/io/socket/build/src/socket/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs
libzapata_net_websocket_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall			\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/io/socket/include -I@PROJECT_DIR@/parsers/json/include		\
-I@PROJECT_DIR@/network/transport/include									\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/engines/startup/include
libzapata_net_websocket_la_SOURCES = \
./websocket.cpp

libzapata_net_websocket_plugin_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-lex-json -lzapata-io-streams -lzapata-net-socket			\
-lzapata-transport libzapata-net-websocket.la
libzapata_net_websocket_plugin_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs						\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs				\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs					\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs						\
-L@PROJECT_DIR@/io/socket/build/src/socket/.libs						\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs						\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs
libzapata_net_websocket_plugin_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/io/socket/include -I@PROJECT_DIR@/parsers/json/include		\
-I@PROJECT_DIR@/network/transport/include									\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/engines/startup/include
libzapata_net_websocket_plugin_la_SOURCES = \
./plugin.cpp
