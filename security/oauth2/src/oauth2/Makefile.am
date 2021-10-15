lib_LTLIBRARIES = libzapata-oauth2.la libzapata-oauth2-plugin.la

libzapata_oauth2_la_LIBADD = -lpthread -lmagic -lzapata-lex-json -lzapata-rest
libzapata_oauth2_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs			\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs	\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs		\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs			\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs			\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs				\
-L@PROJECT_DIR@/common/graph/build/src/graph/.libs			\
-L@PROJECT_DIR@/common/events/build/src/events/.libs		\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs	\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs		\
-L@PROJECT_DIR@/engines/pipeline/build/src/pipeline/.libs	\
-L@PROJECT_DIR@/engines/rest/build/src/rest/.libs
libzapata_oauth2_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ @INCLUDE_DIR@			\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include -I@PROJECT_DIR@/common/graph/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/network/transport/include									\
-I@PROJECT_DIR@/engines/startup/include										\
-I@PROJECT_DIR@/engines/pipeline/include									\
-I@PROJECT_DIR@/engines/rest/include
libzapata_oauth2_la_SOURCES = \
./oauth2.cpp \
./listeners.cpp

libzapata_oauth2_plugin_la_LIBADD = -lpthread -lzapata-rest libzapata-oauth2.la
libzapata_oauth2_plugin_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/io/stream/build/src/streams/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs					\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs						\
-L@PROJECT_DIR@/common/graph/build/src/graph/.libs					\
-L@PROJECT_DIR@/common/events/build/src/events/.libs				\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs			\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs				\
-L@PROJECT_DIR@/engines/pipeline/build/src/pipeline/.libs			\
-L@PROJECT_DIR@/engines/rest/build/src/rest/.libs
libzapata_oauth2_plugin_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall			\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include -I@PROJECT_DIR@/io/stream/include	\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include -I@PROJECT_DIR@/common/graph/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/network/transport/include									\
-I@PROJECT_DIR@/engines/startup/include										\
-I@PROJECT_DIR@/engines/pipeline/include									\
-I@PROJECT_DIR@/engines/rest/include
libzapata_oauth2_plugin_la_SOURCES = \
./plugin.cpp
