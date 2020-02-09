lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json			\
-lzapata-lex-uri -lzapata-common-graph -lzapata-common-lockfree					\
-lzapata-common-events -lzapata-net-socket -lzapata-transport -lzapata-pipeline
libzapata_rest_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_rest_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_rest_la_SOURCES = \
./rest.cpp \
./plugin.cpp
