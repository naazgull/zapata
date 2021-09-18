lib_LTLIBRARIES = libzapata-common-allocator.la

libzapata_common_allocator_la_LIBADD = -lpthread -lzapata-base
libzapata_common_allocator_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs
libzapata_common_allocator_la_CPPFLAGS = -O3 @CPP_STD_VERSION@	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include

libzapata_common_allocator_la_SOURCES = \
./allocator.cpp

bin_PROGRAMS = zallocator

zallocator_LDADD = -lpthread -lzapata-base libzapata-common-allocator.la
zallocator_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs
zallocator_CPPFLAGS = -O3 @CPP_STD_VERSION@ @INCLUDE_DIR@	\
-I@PROJECT_DIR@/common/base/include

zallocator_SOURCES = \
./main.cpp
