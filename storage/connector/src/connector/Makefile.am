lib_LTLIBRARIES = libzapata-storage-connector.la

libzapata_storage_connector_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json
libzapata_storage_connector_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_storage_connector_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall

libzapata_storage_connector_la_SOURCES = \
./connector.cpp
