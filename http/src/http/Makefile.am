
lib_LTLIBRARIES = libzapata-http.la

libzapata_http_la_LIBADD = -lpthread -lzapata-base
libzapata_http_la_LDFLAGS = -version-info 9:5:9 
libzapata_http_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_http_la_SOURCES = \
./http.cpp \
./HTTPElement.cpp \
./HTTPLexer.cpp \
./HTTPParser.cpp \
./HTTPRep.cpp \
./HTTPReq.cpp \
./HTTPTokenizer.cpp \
./HTTPTokenizerLexer.cpp
