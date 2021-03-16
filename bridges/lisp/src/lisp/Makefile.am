
lib_LTLIBRARIES = libzapata-lisp.la

libzapata_lisp_la_LIBADD = -lpthread -lssl -lcrypto -lzapata-base -lzapata-common-events -lzapata-lex-json
libzapata_lisp_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_lisp_la_CPPFLAGS = -O3 -std=c++20 -I../../../include -I../../include

libzapata_lisp_la_SOURCES = \
./Bridge.cpp \
./convert_lisp.cpp
