
lib_LTLIBRARIES = libzapata-http.la

libzapata_http_la_LIBADD = -lpthread -lzapata-core
libzapata_http_la_LDFLAGS= -L../zapata-core/.libs -version-info 9:1:9
libzapata_http_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_http_la_SOURCES = \
./http/HTTPElement.cpp \
./http/HTTPReq.cpp \
./http/HTTPRep.cpp \
./exceptions/NoHeaderNameException.cpp \
./parsers/HTTPTokenizer.cpp \
./parsers/http.cpp \
./parsers/HTTPLexer.cpp \
./parsers/HTTPParser.cpp \
./parsers/HTTPTokenizerLexer.cpp 
