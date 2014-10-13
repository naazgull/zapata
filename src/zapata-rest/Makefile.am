
lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lzapata-core -lzapata-http -lzapata-net
libzapata_rest_la_LDFLAGS= -L../zapata-core/.libs -L../zapata-http/.libs -L../zapata-net/.libs
libzapata_rest_la_CPPFLAGS = -std=c++0x -I../zapata-core -I../zapata-http -I../zapata-net

libzapata_rest_la_SOURCES = ./thread/RESTServer.cpp ./thread/RESTJob.cpp ./http/params.cpp ./http/requester.cpp ./api/RESTPool.cpp 
nobase_pkginclude_HEADERS = ./zapata/rest.h ./thread/RESTJob.h ./thread/RESTServer.h ./http/params.h ./http/requester.h ./api/RESTPool.h ./api/codes_rest.h 
