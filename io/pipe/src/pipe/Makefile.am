lib_LTLIBRARIES = libzapata-io-pipe.la

libzapata_io_pipe_la_LIBADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-io-streams
libzapata_io_pipe_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_io_pipe_la_CPPFLAGS = -O3 -std=c++20 -Wall -I../../../include -I../../include

libzapata_io_pipe_la_SOURCES = \
./pipe_stream.cpp

bin_PROGRAMS = zpipe

zpipe_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-io-streams libzapata-io-pipe.la
zpipe_LDFLAGS=
zpipe_CPPFLAGS = -std=c++20 -Wall -I../../../include -I../../include

zpipe_SOURCES = \
./main.cpp
