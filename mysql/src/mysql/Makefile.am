
lib_LTLIBRARIES = libzapata-mysql.la

libzapata_mysql_la_LIBADD = -lpthread -lssl -lcrypto -lmysqlcppconn -lzapata-base -lzapata-json
libzapata_mysql_la_LDFLAGS = -version-info 9:7:9
libzapata_mysql_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_mysql_la_SOURCES = \
./convert_sql.cpp \
./Client.cpp
