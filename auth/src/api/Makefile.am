
lib_LTLIBRARIES = libzapata-auth.la libzapata-oauth2.la

libzapata_auth_la_LIBADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-rest -lzapata-data
libzapata_auth_la_LDFLAGS = -version-info 9:1:9
libzapata_auth_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_auth_la_SOURCES = \
./auth.cpp

libzapata_oauth2_la_LIBADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-rest -lzapata-data
libzapata_oauth2_la_LDFLAGS = -version-info 9:1:9
libzapata_oauth2_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_oauth2_la_SOURCES = \
./oauth2.cpp
