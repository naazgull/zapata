lib_LTLIBRARIES = libzapata-bridge-prolog.la

libzapata_bridge_prolog_la_LIBADD = -lpthread -lzapata-base
libzapata_bridge_prolog_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_bridge_prolog_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

libzapata_bridge_prolog_la_SOURCES = \
./prolog.cpp

bin_PROGRAMS = zprolog

zprolog_LDADD = -lpthread libzapata-bridge-prolog.la
zprolog_LDFLAGS =
zprolog_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

zprolog_SOURCES = \
./main.cpp
