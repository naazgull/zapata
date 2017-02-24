
lib_LTLIBRARIES = libzapata-mariadb.la

libzapata_mariadb_la_LIBADD = -lpthread -lssl -lcrypto -lmysqlcppconn -lzapata-base -lzapata-json
libzapata_mariadb_la_LDFLAGS = -version-info 9:6:9
libzapata_mariadb_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_mariadb_la_SOURCES = \
./convert_sql.cpp \
./Client.cpp
