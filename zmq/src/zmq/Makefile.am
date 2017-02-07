lib_LTLIBRARIES = libzapata-zmq.la

libzapata_zmq_la_LIBADD = -lzapata-base -lzapata-json -lzapata-events -lzmq -lczmq -lsodium -lcurve
libzapata_zmq_la_LDFLAGS = -version-info 9:5:9
libzapata_zmq_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_zmq_la_SOURCES = \
./SocketStreams.cpp \
./SSLSocketStreams.cpp \
./WebSocket.cpp \
./ZMQPoll.cpp \
./ZMQClient.cpp \
./ZMQMutationEmitter.cpp

bin_PROGRAMS = zcert zmut

zcert_LDADD = -lzapata-base -lzapata-json -lzapata-events -lzmq -lczmq -lsodium -lcurve -lzapata-zmq
zcert_LDFLAGS = -L../zmq/.libs
zcert_CPPFLAGS = -O3 -std=c++14 -I../../include

zcert_SOURCES = \
./zcert.cpp

zmut_LDADD = -lzapata-base -lzapata-json -lzapata-events -lzmq -lczmq -lsodium -lcurve -lzapata-zmq
zmut_LDFLAGS = -L../zmq/.libs
zmut_CPPFLAGS = -O3 -std=c++14 -I../../include

zmut_SOURCES = \
./main.cpp
