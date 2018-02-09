
lib_LTLIBRARIES = libzapata-python.la

libzapata_python_la_LIBADD = -lpthread -lssl -lcrypto -lzapata-base -lzapata-events -lzapata-json -lpython3.6m
libzapata_python_la_LDFLAGS = -version-info 9:7:9
libzapata_python_la_CPPFLAGS = -O3 -std=c++11 -I../../include -I/usr/include/python3.6 -fwrapv

libzapata_python_la_SOURCES = \
./Bridge.cpp \
./convert_python.cpp
