lib_LTLIBRARIES = libzapata-lex-http.la

libzapata_lex_http_la_LIBADD = -lpthread -lzapata-base
libzapata_lex_http_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs
libzapata_lex_http_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ @INCLUDE_DIR@		\
-I@PROJECT_DIR@/common/base/include -I@PROJECT_DIR@/parsers/json/include

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

zhttps_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-io-streams -lzapata-net-socket -lzapata-lex-json		\
libzapata-lex-http.la
zhttps_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs					\
-L@PROJECT_DIR@/io/socket/build/src/socket/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs
zhttps_CPPFLAGS = @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@						\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/io/socket/include -I@PROJECT_DIR@/parsers/json/include

zhttps_SOURCES = \
./server.cpp
