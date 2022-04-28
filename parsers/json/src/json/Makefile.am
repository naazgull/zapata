lib_LTLIBRARIES = libzapata-lex-json.la

libzapata_lex_json_la_LIBADD = -lpthread -lmagic -lzapata-base
libzapata_lex_json_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs
libzapata_lex_json_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ @INCLUDE_DIR@	\
-I@PROJECT_DIR@/common/base/include

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

bin_PROGRAMS = zat zjson

zat_LDADD = -lpthread -lzapata-base libzapata-lex-json.la
zat_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs
zat_CPPFLAGS = @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@	\
-I@PROJECT_DIR@/common/base/include

zat_SOURCES = \
zat.cpp

zjson_LDADD = -lpthread -lzapata-base libzapata-lex-json.la
zjson_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs
zjson_CPPFLAGS = @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@	\
-I@PROJECT_DIR@/common/base/include

zjson_SOURCES = \
main.cpp
