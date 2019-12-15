
lib_LTLIBRARIES = libzapata-python.la

libzapata_python_la_LIBADD = -lpthread -lssl -lcrypto -lzapata-base -lzapata-kernel-events -lzapata-lex-json -lpython3.6m
libzapata_python_la_LDFLAGS = -version-info 1:0:0
libzapata_python_la_CPPFLAGS = -O3 -std=c++17 -I../../include -I/usr/include/python3.6 -fwrapv

libzapata_python_la_SOURCES = \
./Bridge.cpp \
./convert_python.cpp
