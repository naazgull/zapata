lib_LTLIBRARIES = libzapata-rest.la

libzapata_rest_la_LIBADD = -lpthread -lsystemd -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-mqtt -lzapata-upnp -lpython3.6m -lecl -lzapata-lisp -lzapata-python
libzapata_rest_la_LDFLAGS = -version-info 9:7:9
libzapata_rest_la_CPPFLAGS = -O3 -std=c++11 -I../../include -I/usr/include/python3.6 -fwrapv

libzapata_rest_la_SOURCES = \
./RESTEmitter.cpp \
./RESTServer.cpp

bin_PROGRAMS = zpt z0mqc zsysd

zpt_LDADD = -lpthread -lmagic -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-mqtt -lzapata-upnp -lpython3.6m -lecl -lzapata-lisp -lzapata-python libzapata-rest.la
zpt_LDFLAGS =
zpt_CPPFLAGS = -O3 -std=c++11 -I../../include -I/usr/include/python3.6 -fwrapv

zpt_SOURCES = \
./main.cpp

z0mqc_LDADD = -lpthread -lmagic  -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-mqtt -lzapata-upnp -lpython3.6m -lecl -lzapata-lisp -lzapata-python libzapata-rest.la
z0mqc_LDFLAGS =
z0mqc_CPPFLAGS = -O3 -std=c++11 -I../../include -I/usr/include/python3.6 -fwrapv

z0mqc_SOURCES = \
./zclient.cpp

zsysd_LDADD = -lpthread -lmagic  -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-mqtt -lzapata-upnp -lpython3.6m -lecl -lzapata-lisp -lzapata-python libzapata-rest.la
zsysd_LDFLAGS =
zsysd_CPPFLAGS = -O3 -std=c++11 -I../../include -I/usr/include/python3.6 -fwrapv

zsysd_SOURCES = \
./zsysd.cpp
