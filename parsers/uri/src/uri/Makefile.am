
lib_LTLIBRARIES = libzapata-uri.la

libzapata_uri_la_LIBADD = -lpthread -lzapata-base -lzapata-json
libzapata_uri_la_LDFLAGS = -version-info 1:0:0
libzapata_uri_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_uri_la_SOURCES = \
./uri.cpp \
./URILexer.cpp \
./URIParser.cpp \
./URITokenizer.cpp \
./URITokenizerLexer.cpp
