lib_LTLIBRARIES = libzapata-0mq.la

libzapata_0mq_la_LIBADD = -lpthread -lzapata-base -lzapata-json -lzapata-events
libzapata_0mq_la_LDFLAGS = -version-info 9:1:9
libzapata_0mq_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_0mq_la_SOURCES = \
./0mqPoll.cpp \
./0mqClient.cpp
