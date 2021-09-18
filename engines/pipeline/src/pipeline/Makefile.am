lib_LTLIBRARIES = libzapata-pipeline.la

libzapata_pipeline_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json	\
-lzapata-lex-uri -lzapata-common-graph -lzapata-common-lockfree				\
-lzapata-common-events
libzapata_pipeline_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs			\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs	\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs		\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs			\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs			\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs				\
-L@PROJECT_DIR@/common/graph/build/src/graph/.libs			\
-L@PROJECT_DIR@/common/events/build/src/events/.libs		\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs	\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
libzapata_pipeline_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@	\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include -I@PROJECT_DIR@/common/graph/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/network/transport/include									\
-I@PROJECT_DIR@/engines/startup/include
libzapata_pipeline_la_SOURCES = \
./pipeline.cpp

bin_PROGRAMS = zpipeline

zpipeline_LDADD = -lpthread -lzapata-base -lzapata-lex-json		\
-lzapata-lex-uri -lzapata-common-graph -lzapata-common-lockfree	\
-lzapata-common-events libzapata-pipeline.la
zpipeline_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs				\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs						\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs						\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs							\
-L@PROJECT_DIR@/common/graph/build/src/graph/.libs						\
-L@PROJECT_DIR@/common/events/build/src/events/.libs					\
-L@PROJECT_DIR@/network/transport/build/src/transport/.libs				\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
zpipeline_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@				\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include -I@PROJECT_DIR@/common/graph/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/network/transport/include									\
-I@PROJECT_DIR@/engines/startup/include
zpipeline_SOURCES = \
./main.cpp
