
lib_LTLIBRARIES = libzapata-datum.la

libzapata_datum_la_LIBADD = -lpthread -lssl -lcrypto -lzapata-base -lzapata-json -lzapata-redis -lzapata-mongodb -lzapata-mariadb -lzapata-postgresql
libzapata_datum_la_LDFLAGS = -version-info 9:1:9
libzapata_datum_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_datum_la_SOURCES = \
./Datum.cpp 
