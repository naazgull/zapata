
lib_LTLIBRARIES = libzapata-redis.la

libzapata_redis_la_LIBADD = -lpthread -lssl -lcrypto -lhiredis -lzapata-base -lzapata-lex-json
libzapata_redis_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_redis_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -I../../../include -I../../include

libzapata_redis_la_SOURCES = \
./convert_redis.cpp \
./Client.cpp \
./ZList.cpp
