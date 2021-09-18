
lib_LTLIBRARIES = libzapata-postgresql.la

libzapata_postgresql_la_LIBADD = -lpthread -lssl -lcrypto -lpqxx -lpq -lzapata-base -lzapata-lex-json
libzapata_postgresql_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_postgresql_la_CPPFLAGS = -O3 @CPP_STD_VERSION@

libzapata_postgresql_la_SOURCES = \
./convert_sql.cpp \
./Client.cpp
