
lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lzapata-core -lzapata-http -lzapata-net
libzapata_rest_la_LDFLAGS= -L../zapata-core/.libs -L../zapata-http/.libs -L../zapata-net/.libs
libzapata_rest_la_CPPFLAGS = -std=c++0x -I../../include

libzapata_rest_la_SOURCES = ./thread/RESTServer.cpp ./thread/RESTJob.cpp ./http/params.cpp ./http/requester.cpp ./api/RESTPool.cpp 
