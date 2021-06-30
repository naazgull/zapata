lib_LTLIBRARIES = libzapata-bridge-lua.la libzapata-bridge-lua-plugin.la

libzapata_bridge_lua_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json	\
-lzapata-bridge-base -llua@LUA_VERSION@-c++
libzapata_bridge_lua_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_bridge_lua_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall		\
-I../../../include -I../../include -I/usr/include/lua@LUA_VERSION@

libzapata_bridge_lua_la_SOURCES = \
./lua.cpp

libzapata_bridge_lua_plugin_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-lex-json -lzapata-bridge-base -llua@LUA_VERSION@-c++	\
libzapata-bridge-lua.la
libzapata_bridge_lua_plugin_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_bridge_lua_plugin_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
-I../../../include -I../../include -I/usr/include/lua@LUA_VERSION@

libzapata_bridge_lua_plugin_la_SOURCES = \
./plugin.cpp

bin_PROGRAMS = zlua

zlua_LDADD = -lpthread -lzapata-base -lzapata-lex-json			\
-lzapata-bridge-base -lzapata-lex-json -llua@LUA_VERSION@-c++	\
libzapata-bridge-lua.la
zlua_LDFLAGS =
zlua_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include	\
-I../../include -I/usr/include/lua@LUA_VERSION@

zlua_SOURCES = \
./main.cpp
