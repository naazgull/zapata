lib_LTLIBRARIES = libzapata-bridge-lua.la

libzapata_bridge_lua_la_LIBADD = -lpthread -lzapata-base
libzapata_bridge_lua_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_bridge_lua_la_CPPFLAGS = -O3 -std=c++20 -Wall -I../../../include -I../../include

libzapata_bridge_lua_la_SOURCES = \
./lua.cpp

bin_PROGRAMS = zlua

zlua_LDADD = -lpthread libzapata-bridge-lua.la
zlua_LDFLAGS =
zlua_CPPFLAGS = -O3 -std=c++20 -Wall -I../../../include -I../../include

zlua_SOURCES = \
./main.cpp
