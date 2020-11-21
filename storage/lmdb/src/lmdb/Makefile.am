lib_LTLIBRARIES = libzapata-storage-lmdb.la

libzapata_storage_lmdb_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-common-lockfree -lzapata-lex-json -lzapata-startup	\
-lzapata-storage-connector -llmdb
libzapata_storage_lmdb_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_storage_lmdb_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_storage_lmdb_la_SOURCES = \
./connector.cpp

bin_PROGRAMS = zlmdb

zlmdb_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree			\
-lzapata-lex-json -lzapata-startup -lzapata-storage-connector -llmdb	\
libzapata-storage-lmdb.la
zlmdb_LDFLAGS = 
zlmdb_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zlmdb_SOURCES = \
./main.cpp
