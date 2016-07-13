lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-addons -lzapata-zmq
libzapata_rest_la_LDFLAGS = -version-info 9:1:9
libzapata_rest_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_rest_la_SOURCES = \
./RESTEmitter.cpp \
./RESTServer.cpp 

bin_PROGRAMS = zapi zcli

zapi_LDADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-addons -lzapata-zmq -lzapata-rest
zapi_LDFLAGS =  -L../rest/.libs
zapi_CPPFLAGS = -O3 -std=c++11 -I../../include

zapi_SOURCES = \
./main.cpp

zcli_LDADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-addons -lzapata-zmq -lzapata-rest
zcli_LDFLAGS =  -L../rest/.libs
zcli_CPPFLAGS = -O3 -std=c++11 -I../../include

zcli_SOURCES = \
./zclient.cpp
