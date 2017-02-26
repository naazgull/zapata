lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-mqtt -lpython3.6m -lecl -lzapata-lisp -lzapata-python
libzapata_rest_la_LDFLAGS = -version-info 9:6:9
libzapata_rest_la_CPPFLAGS = -O3 -std=c++14 -I../../include -I/usr/include/python3.6 -fwrapv

libzapata_rest_la_SOURCES = \
./MutationEmitter.cpp \
./RESTEmitter.cpp \
./RESTServer.cpp 

bin_PROGRAMS = zpt z0mqc

zpt_LDADD = -lpthread -lmagic -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-mqtt -lpython3.6m -lecl -lzapata-lisp -lzapata-python -lzapata-rest
zpt_LDFLAGS =  -L../rest/.libs
zpt_CPPFLAGS = -O3 -std=c++14 -I../../include

zpt_SOURCES = \
./main.cpp

z0mqc_LDADD = -lpthread -lmagic  -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-mqtt -lpython3.6m -lecl -lzapata-lisp -lzapata-python -lzapata-rest
z0mqc_LDFLAGS =  -L../rest/.libs
z0mqc_CPPFLAGS = -O3 -std=c++14 -I../../include

z0mqc_SOURCES = \
./zclient.cpp
