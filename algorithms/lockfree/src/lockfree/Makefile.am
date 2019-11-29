lib_LTLIBRARIES = libzapata-algo-lockfree.la

libzapata_algo_lockfree_la_LIBADD = -lpthread -lzapata-base
libzapata_algo_lockfree_la_LDFLAGS = -version-info 1:0:0
libzapata_algo_lockfree_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_algo_lockfree_la_SOURCES = \
./hptr.cpp

bin_PROGRAMS = zlf

zlf_LDADD = -lpthread -lzapata-algo-lockfree
zlf_LDFLAGS =
zlf_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zlf_SOURCES = \
./main.cpp
