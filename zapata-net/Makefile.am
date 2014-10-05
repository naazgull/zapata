
lib_LTLIBRARIES = libzapata-net.la

libzapata_net_la_LIBADD = -lpthread -lzapata-core -lssl -lcrypto
libzapata_net_la_LDFLAGS= -L../zapata-core/.libs
libzapata_net_la_CPPFLAGS = -std=c++0x -I../zapata-core 

libzapata_net_la_SOURCES = ./exceptions/ClosedException.cpp ./stream/SocketStreams.cpp ./stream/SSLSocketStreams.cpp 
nobase_pkginclude_HEADERS = ./zapata/net.h ./exceptions/ClosedException.h ./stream/SocketStreams.h ./stream/SSLSocketStreams.h 
