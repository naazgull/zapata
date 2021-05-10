lib_LTLIBRARIES = libzapata-lex-json.la

libzapata_lex_json_la_LIBADD = -lpthread -lmagic -lzapata-base
libzapata_lex_json_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_lex_json_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -I../../../include -I../../include

libzapata_lex_json_la_SOURCES = \
./JSONArr.cpp \
./JSONObj.cpp \
./JSONLambda.cpp \
./JSONRegex.cpp \
./JSONElement.cpp \
./JSONPtr.cpp \
./JSONLexer.cpp \
./JSONParser.cpp \
./JSONTokenizer.cpp \
./JSONTokenizerLexer.cpp \
./json.cpp

bin_PROGRAMS = zat zjson zinit

zat_LDADD = -lpthread -lzapata-base libzapata-lex-json.la
zat_LDFLAGS=
zat_CPPFLAGS = @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

zat_SOURCES = \
zat.cpp

zjson_LDADD = -lpthread -lzapata-base libzapata-lex-json.la
zjson_LDFLAGS=
zjson_CPPFLAGS = @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

zjson_SOURCES = \
main.cpp

zinit_LDADD = -lpthread -lzapata-base libzapata-lex-json.la
zinit_LDFLAGS=
zinit_CPPFLAGS = @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

zinit_SOURCES = \
zinit.cpp
