lib_LTLIBRARIES = libzapata-kernel-lockfree.la

libzapata_kernel_lockfree_la_LIBADD = -lpthread -lzapata-base
libzapata_kernel_lockfree_la_LDFLAGS = -version-info 1:0:0
libzapata_kernel_lockfree_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_kernel_lockfree_la_SOURCES = \
./hptr.cpp

bin_PROGRAMS = zlf

zlf_LDADD = -lpthread -lzapata-base -lzapata-kernel-lockfree
zlf_LDFLAGS =
zlf_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zlf_SOURCES = \
./main.cpp
