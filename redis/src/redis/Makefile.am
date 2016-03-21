
lib_LTLIBRARIES = libzapata-redis.la

libzapata_redis_la_LIBADD = -lpthread -lssl -lcrypto -lhiredis -lzapata-base -lzapata-json 
libzapata_redis_la_LDFLAGS = -version-info 9:1:9
libzapata_redis_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_redis_la_SOURCES = \
./Collection.cpp 
