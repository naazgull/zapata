
lib_LTLIBRARIES = libzapata-mongodb.la

libzapata_mongodb_la_LIBADD = -lpthread -lzapata-core -lssl -lcrypto -lmongoclient -lboost_program_options -lboost_thread -lboost_filesystem
libzapata_mongodb_la_LDFLAGS= -L../zapata-core/.libs -version-info 9:1:9
libzapata_mongodb_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_mongodb_la_SOURCES = \
./db/convert_mongo.cpp 
