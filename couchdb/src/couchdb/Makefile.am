
lib_LTLIBRARIES = libzapata-couchdb.la

libzapata_couchdb_la_LIBADD = -lpthread -lssl -lcrypto -lzapata-base -lzapata-json -lzapata-http
libzapata_couchdb_la_LDFLAGS = -version-info 1:0:0
libzapata_couchdb_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_couchdb_la_SOURCES = \
./convert_couchdb.cpp \
./Client.cpp
