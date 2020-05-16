lib_LTLIBRARIES = libzapata-net-local-file.la libzapata-net-local-unix.la

libzapata_net_local_file_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-io-streams -lzapata-transport
libzapata_net_local_file_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_net_local_file_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_net_local_file_la_SOURCES = \
./file/file.cpp \
./file/plugin.cpp

libzapata_net_local_unix_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-io-streams -lzapata-transport
libzapata_net_local_unix_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_net_local_unix_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_net_local_unix_la_SOURCES = \
./unix/unix.cpp \
./unix/plugin.cpp

bin_PROGRAMS = zlocal

zlocal_LDADD = -lpthread libzapata-net-local-file.la	\
libzapata-net-local-unix.la
zlocal_LDFLAGS =
zlocal_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zlocal_SOURCES = \
./main.cpp
