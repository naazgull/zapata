lib_LTLIBRARIES = libzapata-storage-connector.la

libzapata_storage_connector_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json
libzapata_storage_connector_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_storage_connector_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_storage_connector_la_SOURCES = \
./connector.cpp
