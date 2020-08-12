
lib_LTLIBRARIES = libzapata-couchdb.la

libzapata_couchdb_la_LIBADD = -lpthread -lssl -lcrypto -lzapata-base -lzapata-lex-json -lzapata-lex-http
libzapata_couchdb_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_couchdb_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_couchdb_la_SOURCES = \
./convert_couchdb.cpp \
./Client.cpp
