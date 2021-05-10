lib_LTLIBRARIES = libzapata-io-streams.la

libzapata_io_streams_la_LIBADD = -lpthread -lsystemd -lzapata-base	\
-lzapata-common-lockfree
libzapata_io_streams_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_io_streams_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

libzapata_io_streams_la_SOURCES = \
./streams.cpp

bin_PROGRAMS = zstreams

zstreams_LDADD = -lpthread -lsystemd -lzapata-base -lzapata-common-lockfree	\
libzapata-io-streams.la
zstreams_LDFLAGS =
zstreams_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

zstreams_SOURCES = \
./main.cpp
