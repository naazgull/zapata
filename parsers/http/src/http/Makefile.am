
lib_LTLIBRARIES = libzapata-http.la

libzapata_http_la_LIBADD = -lpthread -lzapata-base -lzapata-events
libzapata_http_la_LDFLAGS = -version-info 1:0:0
libzapata_http_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_http_la_SOURCES = \
./http.cpp \
./HTTPElement.cpp \
./HTTPLexer.cpp \
./HTTPParser.cpp \
./HTTPRep.cpp \
./HTTPReq.cpp \
./HTTPTokenizer.cpp \
./HTTPTokenizerLexer.cpp
