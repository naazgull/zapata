lib_LTLIBRARIES = libzapata-io-streams.la

libzapata_io_streams_la_LIBADD = -lpthread -lsystemd -lzapata-base	\
-lzapata-common-lockfree
libzapata_io_streams_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs				\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs
libzapata_io_streams_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include				\
-I@PROJECT_DIR@/common/globals/include							\
-I@PROJECT_DIR@/common/lockfree/include

libzapata_io_streams_la_SOURCES = \
./streams.cpp

bin_PROGRAMS = zstreams

zstreams_LDADD = -lpthread -lsystemd -lzapata-base	\
-lzapata-common-lockfree libzapata-io-streams.la
zstreams_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs
zstreams_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@				\
-I@PROJECT_DIR@/common/base/include -I@PROJECT_DIR@/common/globals/include	\
-I@PROJECT_DIR@/common/lockfree/include

zstreams_SOURCES = \
./main.cpp
