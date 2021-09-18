lib_LTLIBRARIES = libzapata-io-pipe.la

libzapata_io_pipe_la_LIBADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-io-streams
libzapata_io_pipe_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs			\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs	\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs
libzapata_io_pipe_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@	\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include -I@PROJECT_DIR@/io/stream/include

libzapata_io_pipe_la_SOURCES = \
./pipe_stream.cpp

bin_PROGRAMS = zpipe

zpipe_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-io-streams libzapata-io-pipe.la
zpipe_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs
zpipe_CPPFLAGS = @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@						\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include -I@PROJECT_DIR@/io/stream/include

zpipe_SOURCES = \
./main.cpp
