
lib_LTLIBRARIES = libzapata-mysql.la libzapata-mysql-binlog-stream.la

libzapata_mysql_la_LIBADD = -lpthread -lssl -lcrypto -lmysqlcppconn -lzapata-base -lzapata-json
libzapata_mysql_la_LDFLAGS = -version-info 9:7:9
libzapata_mysql_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_mysql_la_SOURCES = \
./convert_sql.cpp \
./Client.cpp

libzapata_mysql_binlog_stream_la_LIBADD = -lpthread
libzapata_mysql_binlog_stream_la_LDFLAGS = -version-info 0:1:0
libzapata_mysql_binlog_stream_la_CPPFLAGS = -std=c++11 -Wall -I../../include

libzapata_mysql_binlog_stream_la_SOURCES = \
./LogEvent.cpp

bin_PROGRAMS = zblreader

zblreader_LDADD = -lpthread -lzapata-mysql-binlog-stream
zblreader_LDFLAGS =  -L./.libs
zblreader_CPPFLAGS = -O3 -std=c++14 -I../../include

zblreader_SOURCES = \
./reader.cpp
