lib_LTLIBRARIES = libzapata-lex-uri.la

libzapata_lex_uri_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json
libzapata_lex_uri_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_lex_uri_la_CPPFLAGS = -O3 -std=c++17 -I../../../include -I../../include

libzapata_lex_uri_la_SOURCES = \
./URILexer.cpp \
./URIParser.cpp \
./URITokenizer.cpp \
./URITokenizerLexer.cpp \
./uri.cpp

bin_PROGRAMS = zuri

zuri_LDADD = -lpthread -lzapata-base -lzapata-lex-json libzapata-lex-uri.la
zuri_LDFLAGS =
zuri_CPPFLAGS = -O3 -std=c++17 -Wall -I../../../include -I../../include

zuri_SOURCES = \
./main.cpp
