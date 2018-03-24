
lib_LTLIBRARIES = libzapata-postgresql.la

libzapata_postgresql_la_LIBADD = -lpthread -lssl -lcrypto -lpqxx -lpq -lzapata-base -lzapata-json
libzapata_postgresql_la_LDFLAGS = -version-info 1:0:0
libzapata_postgresql_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_postgresql_la_SOURCES = \
./convert_sql.cpp \
./Client.cpp
