lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lzapata-base
libzapata_rest_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_rest_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_rest_la_SOURCES = \
./XXX.cpp

bin_PROGRAMS = zrest

zrest_LDADD = -lpthread libzapata-rest.la
zrest_LDFLAGS =
zrest_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zrest_SOURCES = \
./main.cpp
