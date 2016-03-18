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
