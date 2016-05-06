lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-addons -lzapata-zmq
libzapata_rest_la_LDFLAGS = -version-info 9:1:9
libzapata_rest_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_rest_la_SOURCES = \
./RESTEmitter.cpp \
./RESTServer.cpp \
./SocketStreams.cpp \
./SSLSocketStreams.cpp \
./WebSocket.cpp

bin_PROGRAMS = zapata-rest-daemon

zapata_rest_daemon_LDADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-addons -lzapata-zmq -lzapata-rest
zapata_rest_daemon_LDFLAGS =  -L../rest/.libs
zapata_rest_daemon_CPPFLAGS = -O3 -std=c++11 -I../../include

zapata_rest_daemon_SOURCES = \
./main.cpp
