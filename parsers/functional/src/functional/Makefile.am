lib_LTLIBRARIES = libzapata-lex-uri.la

libzapata_lex_uri_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json
libzapata_lex_uri_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs
libzapata_lex_uri_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ @INCLUDE_DIR@			\
-I@PROJECT_DIR@/common/base/include -I@PROJECT_DIR@/parsers/json/include

libzapata_lex_uri_la_SOURCES = \
./URILexer.cpp \
./URIParser.cpp \
./URITokenizer.cpp \
./URITokenizerLexer.cpp \
./uri.cpp

bin_PROGRAMS = zuri

zuri_LDADD = -lpthread -lzapata-base -lzapata-lex-json libzapata-lex-uri.la
zuri_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs
zuri_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@					\
-I@PROJECT_DIR@/common/base/include -I@PROJECT_DIR@/parsers/json/include

zuri_SOURCES = \
./main.cpp
