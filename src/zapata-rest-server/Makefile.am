
bin_PROGRAMS = zapata-rest-daemon

zapata_rest_daemon_LDADD = -lpthread -lzapata-core -lzapata-http -lzapata-net -lzapata-rest
zapata_rest_daemon_LDFLAGS= -L../zapata-core/.libs -L../zapata-http/.libs -L../zapata-net/.libs -L../zapata-rest/.libs
zapata_rest_daemon_CPPFLAGS = -std=c++0x -I../../include

zapata_rest_daemon_SOURCES = ./main.cpp
