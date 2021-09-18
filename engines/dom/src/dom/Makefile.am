lib_LTLIBRARIES = libzapata-dom-processor.la

libzapata_dom_processor_la_LIBADD = -lpthread -lzapata-base			\
-lzapata-lex-json -lzapata-lex-uri -lzapata-common-graph			\
-lzapata-common-lockfree -lzapata-common-events -lzapata-pipeline
libzapata_dom_processor_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs					\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs						\
-L@PROJECT_DIR@/common/graph/build/src/graph/.libs					\
-L@PROJECT_DIR@/common/events/build/src/events/.libs				\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs				\
-L@PROJECT_DIR@/engines/pipeline/build/src/pipeline/.libs
libzapata_dom_processor_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall			\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include -I@PROJECT_DIR@/common/graph/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/engines/startup/include										\
-I@PROJECT_DIR@/engines/pipeline/include
libzapata_dom_processor_la_SOURCES = \
./dom.cpp

bin_PROGRAMS = zdom

zdom_LDADD = -lpthread -lzapata-base -lzapata-lex-json -lzapata-lex-uri	\
-lzapata-common-graph -lzapata-common-lockfree -lzapata-common-events	\
-lzapata-pipeline libzapata-dom-processor.la
zdom_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs		\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs				\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs				\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs					\
-L@PROJECT_DIR@/common/graph/build/src/graph/.libs				\
-L@PROJECT_DIR@/common/events/build/src/events/.libs			\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs			\
-L@PROJECT_DIR@/engines/pipeline/build/src/pipeline/.libs
zdom_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@					\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include -I@PROJECT_DIR@/common/graph/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/engines/startup/include										\
-I@PROJECT_DIR@/engines/pipeline/include
zdom_SOURCES = \
./main.cpp
