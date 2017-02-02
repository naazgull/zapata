
lib_LTLIBRARIES = libzapata-lisp.la

libzapata_lisp_la_LIBADD = -lpthread -lssl -lcrypto -lzapata-base -lzapata-events -lzapata-json
libzapata_lisp_la_LDFLAGS = -version-info 9:1:9
libzapata_lisp_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_lisp_la_SOURCES = \
./Bridge.cpp \
./convert_lisp.cpp 
