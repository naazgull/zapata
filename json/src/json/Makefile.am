lib_LTLIBRARIES = libzapata-json.la

libzapata_json_la_LIBADD = -lpthread -lmagic -lzapata-base
libzapata_json_la_LDFLAGS = -version-info 9:1:9 
libzapata_json_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_json_la_SOURCES = \
./json.cpp \
./JSONLexer.cpp \
./JSONObj.cpp \
./JSONParser.cpp \
./JSONTokenizer.cpp \
./JSONTokenizerLexer.cpp

bin_PROGRAMS = zat

zat_LDADD = -lpthread -lzapata-base -lzapata-json
zat_LDFLAGS= 
zat_CPPFLAGS = -std=c++11 -Wall -I../../include

zat_SOURCES = \
zat.cpp