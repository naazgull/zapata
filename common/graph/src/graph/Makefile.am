lib_LTLIBRARIES = libzapata-kernel-graph.la

libzapata_kernel_graph_la_LIBADD = -lpthread -lzapata-base
libzapata_kernel_graph_la_LDFLAGS = -version-info 1:0:0
libzapata_kernel_graph_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_kernel_graph_la_SOURCES = \
./graph.cpp

bin_PROGRAMS = zgraph

zgraph_LDADD = -lpthread -lzapata-base -lzapata-lex-json -lzapata-lex-uri -lzapata-kernel-graph
zgraph_LDFLAGS =
zgraph_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zgraph_SOURCES = \
./main.cpp
