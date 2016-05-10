lib_LTLIBRARIES = libzapata-zmq.la

libzapata_zmq_la_LIBADD = -lpthread -lzapata-base -lzapata-json -lzapata-events
libzapata_zmq_la_LDFLAGS = -version-info 9:1:9
libzapata_zmq_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_zmq_la_SOURCES = \
./ZMQPoll.cpp \
./ZMQClient.cpp
