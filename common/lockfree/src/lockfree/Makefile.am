lib_LTLIBRARIES = libzapata-common-lockfree.la

libzapata_common_lockfree_la_LIBADD = -lpthread -lzapata-base
libzapata_common_lockfree_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_common_lockfree_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

libzapata_common_lockfree_la_SOURCES = \
./spin_lock.cpp

bin_PROGRAMS = zlf

zlf_LDADD = -lpthread -lzapata-base libzapata-common-lockfree.la
zlf_LDFLAGS =
zlf_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

zlf_SOURCES = \
./main.cpp
