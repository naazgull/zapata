
lib_LTLIBRARIES = libzapata-python.la

libzapata_python_la_LIBADD = -lpthread -lssl -lcrypto -lzapata-base -lzapata-events -lzapata-json -lpython3.5m
libzapata_python_la_LDFLAGS = -version-info 9:5:9
libzapata_python_la_CPPFLAGS = -O3 -std=c++14 -I../../include -I/usr/include/python3.5 -fwrapv

libzapata_python_la_SOURCES = \
./Bridge.cpp \
./convert_python.cpp 
