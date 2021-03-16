
lib_LTLIBRARIES = libzapata-python.la

libzapata_python_la_LIBADD = -lpthread -lssl -lcrypto -lzapata-base -lzapata-common-events -lzapata-lex-json -lpython3.6m
libzapata_python_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_python_la_CPPFLAGS = -O3 -std=c++20 -I../../../include -I../../include

libzapata_python_la_SOURCES = \
./Bridge.cpp \
./convert_python.cpp
