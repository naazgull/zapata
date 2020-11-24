lib_LTLIBRARIES = libzapata-rest.la libzapata-rest-plugin.la

libzapata_rest_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json			\
-lzapata-lex-uri -lzapata-common-graph -lzapata-common-lockfree					\
-lzapata-common-events -lzapata-net-socket -lzapata-transport -lzapata-pipeline
libzapata_rest_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_rest_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../../include -I../../include

libzapata_rest_la_SOURCES = \
./rest.cpp

libzapata_rest_plugin_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json			\
-lzapata-lex-uri -lzapata-common-graph -lzapata-common-lockfree					\
-lzapata-common-events -lzapata-net-socket -lzapata-transport -lzapata-pipeline	\
libzapata-rest.la
libzapata_rest_plugin_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_rest_plugin_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../../include -I../../include

libzapata_rest_plugin_la_SOURCES = \
./plugin.cpp
