lib_LTLIBRARIES = libzapata-net-socket.la

libzapata_net_socket_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-common-lockfree -lzapata-io-streams
libzapata_net_socket_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs				\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs		\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs
libzapata_net_socket_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall				\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/common/lockfree/include -I@PROJECT_DIR@/io/stream/include

libzapata_net_socket_la_SOURCES = \
./socket_stream.cpp

bin_PROGRAMS = zsocks zsockc

zsocks_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-io-streams libzapata-net-socket.la
zsocks_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs
zsocks_CPPFLAGS = @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@						\
-I@PROJECT_DIR@/common/base/include -I@PROJECT_DIR@/common/globals/include	\
-I@PROJECT_DIR@/common/lockfree/include -I@PROJECT_DIR@/io/stream/include

zsocks_SOURCES = \
./server.cpp

zsockc_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-io-streams libzapata-net-socket.la
zsockc_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs
zsockc_CPPFLAGS = @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@						\
-I@PROJECT_DIR@/common/base/include -I@PROJECT_DIR@/common/globals/include	\
-I@PROJECT_DIR@/common/lockfree/include -I@PROJECT_DIR@/io/stream/include

zsockc_SOURCES = \
./client.cpp
