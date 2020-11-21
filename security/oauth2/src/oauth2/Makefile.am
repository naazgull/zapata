lib_LTLIBRARIES = libzapata-oauth2.la libzapata-oauth2-plugin.la

libzapata_oauth2_la_LIBADD = -lpthread -lmagic -lzapata-lex-json -lzapata-rest
libzapata_oauth2_la_LDFLAGS = -version-info 2:0:0
libzapata_oauth2_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_oauth2_la_SOURCES = \
./oauth2.cpp \
./listeners.cpp

libzapata_oauth2_plugin_la_LIBADD = -lpthread -lzapata-rest libzapata-oauth2.la
libzapata_oauth2_plugin_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_oauth2_plugin_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_oauth2_plugin_la_SOURCES = \
./plugin.cpp
