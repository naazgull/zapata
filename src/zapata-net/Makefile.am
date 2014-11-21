
lib_LTLIBRARIES = libzapata-net.la

libzapata_net_la_LIBADD = -lpthread -lzapata-core -lssl -lcrypto
libzapata_net_la_LDFLAGS= -L../zapata-core/.libs -version-info 0:8:0
libzapata_net_la_CPPFLAGS = -std=c++11 -I../../include

libzapata_net_la_SOURCES = \
./exceptions/ClosedException.cpp \
./stream/SocketStreams.cpp \
./stream/SSLSocketStreams.cpp \
./stream/WebSocket.cpp 
