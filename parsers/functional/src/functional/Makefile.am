lib_LTLIBRARIES = libzapata-lex-functional.la

libzapata_lex_functional_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json
libzapata_lex_functional_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs
libzapata_lex_functional_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include					\
-I@PROJECT_DIR@/parsers/json/include

libzapata_lex_functional_la_SOURCES = \
./FunctionalLexer.cpp \
./FunctionalParser.cpp \
./FunctionalTokenizer.cpp \
./FunctionalTokenizerLexer.cpp \
./functional.cpp

bin_PROGRAMS = zfunctional

zfunctional_LDADD = -lpthread -lzapata-base -lzapata-lex-json	\
libzapata-lex-functional.la
zfunctional_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs
zfunctional_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@			\
-I@PROJECT_DIR@/common/base/include -I@PROJECT_DIR@/parsers/json/include

zfunctional_SOURCES = \
./main.cpp
