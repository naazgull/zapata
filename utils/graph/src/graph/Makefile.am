lib_LTLIBRARIES = libzapata-algo-graph.la

libzapata_algo_graph_la_LIBADD = -lpthread -lzapata-base
libzapata_algo_graph_la_LDFLAGS = -version-info 1:0:0
libzapata_algo_graph_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_algo_graph_la_SOURCES = \
./graph.cpp

bin_PROGRAMS = zgraph

zgraph_LDADD = -lpthread -lzapata-lex-json -lzapata-lex-uri -lzapata-algo-graph
zgraph_LDFLAGS =
zgraph_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zgraph_SOURCES = \
./main.cpp
