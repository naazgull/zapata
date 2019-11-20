
lib_LTLIBRARIES = libzapata-mysql.la libzapata-mysql-binlog-stream.la

libzapata_mysql_la_LIBADD = -lpthread -lssl -lcrypto -lmysqlcppconn -lzapata-base -lzapata-lex-json
libzapata_mysql_la_LDFLAGS = -version-info 1:0:0
libzapata_mysql_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_mysql_la_SOURCES = \
./convert_sql.cpp \
./Client.cpp

libzapata_mysql_binlog_stream_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json
libzapata_mysql_binlog_stream_la_LDFLAGS = -version-info 0:1:0
libzapata_mysql_binlog_stream_la_CPPFLAGS = -std=c++17 -Wall -I../../include

libzapata_mysql_binlog_stream_la_SOURCES = \
./LogEvent.cpp

bin_PROGRAMS = zblreader

zblreader_LDADD = -lpthread -lzapata-base -lzapata-lex-json -lzapata-mysql-binlog-stream
zblreader_LDFLAGS =  -L./.libs
zblreader_CPPFLAGS = -O3 -std=c++17 -I../../include

zblreader_SOURCES = \
./reader.cpp
