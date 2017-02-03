lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lpython3.5m -lecl -lzapata-lisp -lzapata-python
libzapata_rest_la_LDFLAGS = -version-info 9:1:9
libzapata_rest_la_CPPFLAGS = -O3 -std=c++14 -I../../include -I/usr/include/python3.5 -fwrapv

libzapata_rest_la_SOURCES = \
./RESTEmitter.cpp \
./RESTServer.cpp 

bin_PROGRAMS = zpt zcli

zpt_LDADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-rest
zpt_LDFLAGS =  -L../rest/.libs
zpt_CPPFLAGS = -O3 -std=c++14 -I../../include

zpt_SOURCES = \
./main.cpp

zcli_LDADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-rest
zcli_LDFLAGS =  -L../rest/.libs
zcli_CPPFLAGS = -O3 -std=c++14 -I../../include

zcli_SOURCES = \
./zclient.cpp
