lib_LTLIBRARIES = libzapata-zmq.la

libzapata_zmq_la_LIBADD = -lsystemd -lzapata-base -lzapata-json -lzapata-events -lzapata-http -lzmq
libzapata_zmq_la_LDFLAGS = -version-info 1:0:0
libzapata_zmq_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_zmq_la_SOURCES = \
./SocketStreams.cpp \
./WebSocket.cpp \
./ZMQClient.cpp
