lib_LTLIBRARIES = libzapata-common-graph.la

libzapata_common_graph_la_LIBADD = -lpthread -lzapata-base
libzapata_common_graph_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_common_graph_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_common_graph_la_SOURCES = \
./graph.cpp

bin_PROGRAMS = zgraph

zgraph_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-lex-json -lzapata-lex-uri libzapata-common-graph.la
zgraph_LDFLAGS =
zgraph_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zgraph_SOURCES = \
./main.cpp
