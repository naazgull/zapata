
lib_LTLIBRARIES = libzapata-http.la

libzapata_http_la_LIBADD = -lpthread -lzapata-core
libzapata_http_la_LDFLAGS= -L../zapata-core/.libs
libzapata_http_la_CPPFLAGS = -std=c++0x -I../zapata-core 

libzapata_http_la_SOURCES = ./http/HTTPElement.cpp ./http/HTTPReq.cpp ./http/HTTPRep.cpp ./exceptions/NoHeaderNameException.cpp ./parsers/HTTPTokenizer.cpp ./parsers/http.cpp ./parsers/HTTPLexer.cpp ./parsers/HTTPParser.cpp ./parsers/HTTPTokenizerLexer.cpp 
nobase_pkginclude_HEADERS = ./zapata/http.h ./http/HTTPObj.h ./exceptions/NoHeaderNameException.h ./parsers/http.h ./parsers/HTTPTokenizerLexer.h ./parsers/HTTPParser.h ./parsers/HTTPTokenizer.h ./parsers/HTTPTokenizerbase.h ./parsers/HTTPLexer.h ./parsers/HTTPLexerimpl.h ./parsers/HTTPLexerbase.h ./parsers/HTTPTokenizerimpl.h ./parsers/HTTPinc.h 