
lib_LTLIBRARIES = libzapata-postgresql.la

libzapata_postgresql_la_LIBADD = -lpthread -lssl -lcrypto -lmysqlcppconn -lzapata-base -lzapata-json -lzapata-addons
libzapata_postgresql_la_LDFLAGS = -version-info 9:1:9
libzapata_postgresql_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_postgresql_la_SOURCES = \
./convert_sql.cpp \
./Client.cpp
