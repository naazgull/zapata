
lib_LTLIBRARIES = libzapata-lua.la

libzapata_lua_la_LIBADD = -lpthread -lssl -lcrypto -lzapata-base -lzapata-events -lzapata-json -llua3.6m
libzapata_lua_la_LDFLAGS = -version-info 9:7:9
libzapata_lua_la_CPPFLAGS = -O3 -std=c++11 -I../../include -I/usr/include/lua3.6 -fwrapv

libzapata_lua_la_SOURCES = \
./Bridge.cpp \
./convert_lua.cpp
