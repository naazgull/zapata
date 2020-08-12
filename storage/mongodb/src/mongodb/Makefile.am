
lib_LTLIBRARIES = libzapata-mongodb.la

libzapata_mongodb_la_LIBADD = -lpthread -lssl -lcrypto -lmongoclient -lboost_program_options -lboost_thread -lboost_filesystem -lzapata-base -lzapata-lex-json
libzapata_mongodb_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_mongodb_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_mongodb_la_SOURCES = \
./Client.cpp \
./convert_mongo.cpp
