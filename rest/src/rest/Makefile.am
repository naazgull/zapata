lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-addons -lzapata-zmq
libzapata_rest_la_LDFLAGS = -version-info 9:1:9
libzapata_rest_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_rest_la_SOURCES = \
./RESTEmitter.cpp \
./RESTServer.cpp 

bin_PROGRAMS = zapa zcli

zapa_LDADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-addons -lzapata-zmq -lzapata-rest
zapa_LDFLAGS =  -L../rest/.libs
zapa_CPPFLAGS = -O3 -std=c++11 -I../../include

zapa_SOURCES = \
./main.cpp

zcli_LDADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-addons -lzapata-zmq -lzapata-rest
zcli_LDFLAGS =  -L../rest/.libs
zcli_CPPFLAGS = -O3 -std=c++11 -I../../include

zcli_SOURCES = \
./zclient.cpp
