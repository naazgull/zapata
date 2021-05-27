lib_LTLIBRARIES = libzapata-common-allocator.la

libzapata_common_allocator_la_LIBADD = -lpthread -lzapata-base
libzapata_common_allocator_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_common_allocator_la_CPPFLAGS = -O3 -std=c++20 -I../../../include -I../../include

libzapata_common_allocator_la_SOURCES = \
./allocator.cpp

bin_PROGRAMS = zallocator

zallocator_LDADD = -lpthread -lzapata-base libzapata-common-allocator.la
zallocator_LDFLAGS =
zallocator_CPPFLAGS = -O3 -std=c++20 -I../../../include -I../../include

zallocator_SOURCES = \
./main.cpp
