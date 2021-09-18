lib_LTLIBRARIES = libzapata-startup.la libzapata-plugin-example.la

libzapata_startup_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-uri	\
-lzapata-lex-json -lzapata-common-lockfree -lzapata-common-events		\
-lzapata-io-streams -lzapata-transport
libzapata_startup_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs			\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs	\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs			\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs				\
-L@PROJECT_DIR@/common/graph/build/src/graph/.libs			\
-L@PROJECT_DIR@/common/events/build/src/events/.libs		\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs
libzapata_startup_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@	\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/uri/include	\
-I@PROJECT_DIR@/common/graph/include -I@PROJECT_DIR@/common/events/include	\
-I@PROJECT_DIR@/network/transport/include


libzapata_startup_la_SOURCES = \
./configuration.cpp \
./startup.cpp

libzapata_plugin_example_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-uri	\
-lzapata-lex-json -lzapata-io-streams -lzapata-transport libzapata-startup.la
libzapata_plugin_example_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs						\
-L@PROJECT_DIR@/common/graph/build/src/graph/.libs					\
-L@PROJECT_DIR@/common/events/build/src/events/.libs				\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs
libzapata_plugin_example_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall			\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/uri/include	\
-I@PROJECT_DIR@/common/graph/include -I@PROJECT_DIR@/common/events/include	\
-I@PROJECT_DIR@/network/transport/include

libzapata_plugin_example_la_SOURCES = \
./plugin.cpp

bin_PROGRAMS = zpt

zpt_LDADD = -lpthread -lzapata-base -lzapata-lex-uri -lzapata-lex-json	\
-lzapata-common-lockfree -lzapata-common-events -lzapata-io-streams		\
-lzapata-transport libzapata-startup.la
zpt_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs		\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs				\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs				\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs					\
-L@PROJECT_DIR@/common/graph/build/src/graph/.libs				\
-L@PROJECT_DIR@/common/events/build/src/events/.libs			\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs
zpt_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@					\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/uri/include	\
-I@PROJECT_DIR@/common/graph/include -I@PROJECT_DIR@/common/events/include	\
-I@PROJECT_DIR@/network/transport/include

zpt_SOURCES = \
./main.cpp
