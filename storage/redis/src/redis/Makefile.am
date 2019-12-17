
lib_LTLIBRARIES = libzapata-redis.la

libzapata_redis_la_LIBADD = -lpthread -lssl -lcrypto -lhiredis -lzapata-base -lzapata-lex-json
libzapata_redis_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_redis_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_redis_la_SOURCES = \
./convert_redis.cpp \
./Client.cpp \
./ZList.cpp
