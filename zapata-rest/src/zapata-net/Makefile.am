
lib_LTLIBRARIES = libzapata-net.la

libzapata_net_la_LIBADD = -lpthread -lzapata-core -lssl -lcrypto
libzapata_net_la_LDFLAGS= -L../zapata-core/.libs -version-info 9:1:9
libzapata_net_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_net_la_SOURCES = \
./exceptions/ClosedException.cpp \
./stream/SocketStreams.cpp \
./stream/SSLSocketStreams.cpp \
./stream/WebSocket.cpp 
