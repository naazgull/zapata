lib_LTLIBRARIES = libzapata-bridge-lua.la libzapata-bridge-lua-plugin.la

libzapata_bridge_lua_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-lex-json -lzapata-bridge-base -llua@LUA_VERSION@
libzapata_bridge_lua_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs				\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs		\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs				\
-L@PROJECT_DIR@/bridges/base/build/src/bridge/.libs				\
-L@PROJECT_DIR@/common/events/build/src/events/.libs			\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
libzapata_bridge_lua_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall				\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/bridges/base/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/engines/startup/include -I/usr/include/lua@LUA_VERSION@

libzapata_bridge_lua_la_SOURCES = \
./lua.cpp

libzapata_bridge_lua_plugin_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-lex-json -lzapata-bridge-base -llua@LUA_VERSION@		\
libzapata-bridge-lua.la
libzapata_bridge_lua_plugin_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs						\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs				\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs						\
-L@PROJECT_DIR@/bridges/base/build/src/bridge/.libs						\
-L@PROJECT_DIR@/common/events/build/src/events/.libs					\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
libzapata_bridge_lua_plugin_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall		\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/bridges/base/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/engines/startup/include -I/usr/include/lua@LUA_VERSION@

libzapata_bridge_lua_plugin_la_SOURCES = \
./plugin.cpp

bin_PROGRAMS = zlua

zlua_LDADD = -lpthread -lzapata-base -lzapata-lex-json		\
-lzapata-bridge-base -lzapata-lex-json -llua@LUA_VERSION@	\
libzapata-bridge-lua.la
zlua_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs		\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs				\
-L@PROJECT_DIR@/bridges/base/build/src/bridge/.libs				\
-L@PROJECT_DIR@/common/events/build/src/events/.libs			\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
zlua_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@					\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/bridges/base/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/engines/startup/include -I/usr/include/lua@LUA_VERSION@

zlua_SOURCES = \
./main.cpp
