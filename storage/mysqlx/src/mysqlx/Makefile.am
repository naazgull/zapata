lib_LTLIBRARIES = libzapata-storage-mysqlx.la

libzapata_storage_mysqlx_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json -lzapata-storage-connector
libzapata_storage_mysqlx_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_storage_mysqlx_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_storage_mysqlx_la_SOURCES = \
./connector.cpp \
./plugin.cpp
