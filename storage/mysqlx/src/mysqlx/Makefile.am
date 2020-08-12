lib_LTLIBRARIES = libzapata-storage-mysqlx.la

libzapata_storage_mysqlx_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-common-lockfree -lzapata-lex-json -lzapata-startup		\
-lzapata-storage-connector -lmysqlcppconn8
libzapata_storage_mysqlx_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_storage_mysqlx_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_storage_mysqlx_la_SOURCES = \
./connector.cpp

bin_PROGRAMS = zmysqlx

zmysqlx_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree				\
-lzapata-lex-json -lzapata-startup -lzapata-storage-connector -lmysqlcppconn8	\
libzapata-storage-mysqlx.la
zmysqlx_LDFLAGS = 
zmysqlx_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zmysqlx_SOURCES = \
./main.cpp
