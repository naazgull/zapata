
lib_LTLIBRARIES = libzapata-lisp.la

libzapata_lisp_la_LIBADD = -lpthread -lssl -lcrypto -lzapata-base -lzapata-events -lzapata-lex-json
libzapata_lisp_la_LDFLAGS = -version-info 1:0:0
libzapata_lisp_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_lisp_la_SOURCES = \
./Bridge.cpp \
./convert_lisp.cpp
