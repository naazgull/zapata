
lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lzapata-core -lzapata-http -lzapata-net
libzapata_rest_la_LDFLAGS= -L../zapata-core/.libs -L../zapata-http/.libs -L../zapata-net/.libs -version-info 9:1:9
libzapata_rest_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_rest_la_SOURCES = \
./thread/RESTServer.cpp \
./thread/RESTJob.cpp \
./http/params.cpp \
./http/requester.cpp \
./api/RESTPool.cpp 
