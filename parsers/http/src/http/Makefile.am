lib_LTLIBRARIES = libzapata-lex-http.la

libzapata_lex_http_la_LIBADD = -lpthread -lzapata-base
libzapata_lex_http_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_lex_http_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -I../../../include -I../../include

libzapata_lex_http_la_SOURCES = \
./http.cpp \
./HTTPElement.cpp \
./HTTPLexer.cpp \
./HTTPParser.cpp \
./HTTPRep.cpp \
./HTTPReq.cpp \
./HTTPTokenizer.cpp \
./HTTPTokenizerLexer.cpp

bin_PROGRAMS = zhttps

zhttps_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree -lzapata-io-streams -lzapata-net-socket -lzapata-lex-json libzapata-lex-http.la
zhttps_LDFLAGS=
zhttps_CPPFLAGS = @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

zhttps_SOURCES = \
./server.cpp
