lib_LTLIBRARIES = libzapata-zmq.la

libzapata_zmq_la_LIBADD = -lsystemd -lzapata-base -lzapata-lex-json -lzapata-common-events -lzapata-lex-http -lzmq
libzapata_zmq_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_zmq_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_zmq_la_SOURCES = \
./SocketStreams.cpp \
./WebSocket.cpp \
./ZMQFactory.cpp
