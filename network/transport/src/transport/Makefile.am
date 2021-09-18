lib_LTLIBRARIES = libzapata-transport.la

libzapata_transport_la_LIBADD = -lpthread -lzapata-base							\
-lzapata-common-lockfree -lzapata-io-streams -lzapata-lex-json -lzapata-lex-uri
libzapata_transport_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs				\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs		\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs				\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs				\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs
libzapata_transport_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall				\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/uri/include

libzapata_transport_la_SOURCES = \
./transport.cpp

bin_PROGRAMS = ztransport

ztransport_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree			\
-lzapata-io-streams -lzapata-lex-json -lzapata-lex-http -lzapata-lex-uri	\
-lzapata-net-socket libzapata-transport.la
ztransport_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs				\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs						\
-L@PROJECT_DIR@/io/socket/build/src/socket/.libs						\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs						\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs						\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs
ztransport_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@				\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/io/socket/include -I@PROJECT_DIR@/parsers/json/include		\
-I@PROJECT_DIR@/parsers/http/include -I@PROJECT_DIR@/parsers/uri/include


ztransport_SOURCES = \
./main.cpp
