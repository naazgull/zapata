
lib_LTLIBRARIES = libzapata-users.la

libzapata_users_la_LIBADD = -lpthread -lzapata-core -lzapata-http -lzapata-net -lzapata-rest -lzapata-mongodb
libzapata_users_la_LDFLAGS= -L../zapata-core/.libs -L../zapata-http/.libs -L../zapata-net/.libs -L../zapata-rest/.libs -L../zapata-mongodb/.libs -version-info 0:8:0
libzapata_users_la_CPPFLAGS = -std=c++11 -I../../include

libzapata_users_la_SOURCES = ./api/users.cpp 
