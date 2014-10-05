
lib_LTLIBRARIES = libzapata-users.la

libzapata_users_la_LIBADD = -lpthread -lzapata-core -lzapata-http -lzapata-net -lzapata-rest -lzapata-mongodb
libzapata_users_la_LDFLAGS= -L../zapata-core/.libs -L../zapata-http/.libs -L../zapata-net/.libs -L../zapata-rest/.libs -L../zapata-mongodb/.libs
libzapata_users_la_CPPFLAGS = -std=c++0x -I../zapata-core -I../zapata-http -I../zapata-net -I../zapata-rest -I../zapata-mongodb

libzapata_users_la_SOURCES = ./api/users.cpp 
nobase_pkginclude_HEADERS = ./api/codes_users.h
