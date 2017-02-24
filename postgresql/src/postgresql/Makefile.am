
lib_LTLIBRARIES = libzapata-postgresql.la

libzapata_postgresql_la_LIBADD = -lpthread -lssl -lcrypto -lpqxx -lpq -lzapata-base -lzapata-json
libzapata_postgresql_la_LDFLAGS = -version-info 9:6:9
libzapata_postgresql_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_postgresql_la_SOURCES = \
./convert_sql.cpp \
./Client.cpp
