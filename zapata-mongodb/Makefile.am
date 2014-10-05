
lib_LTLIBRARIES = libzapata-mongodb.la

libzapata_mongodb_la_LIBADD = -lpthread -lzapata-core -lssl -lcrypto -lmongoclient -lboost_program_options -lboost_thread -lboost_filesystem
libzapata_mongodb_la_LDFLAGS= -L../zapata-core/.libs
libzapata_mongodb_la_CPPFLAGS = -std=c++0x -I../zapata-core 

libzapata_mongodb_la_SOURCES = ./db/convert_mongo.cpp 
nobase_pkginclude_HEADERS = ./zapata/mongodb.h ./db/convert_mongo.h 