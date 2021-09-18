lib_LTLIBRARIES = libzapata-common-graph.la

libzapata_common_graph_la_LIBADD = -lpthread -lzapata-base
libzapata_common_graph_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs				\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs
libzapata_common_graph_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include					\
-I@PROJECT_DIR@/common/lockfree/include

libzapata_common_graph_la_SOURCES = \
./graph.cpp

bin_PROGRAMS = zgraph

zgraph_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-lex-json -lzapata-lex-uri libzapata-common-graph.la
zgraph_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs
zgraph_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@					\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/uri/include

zgraph_SOURCES = \
./main.cpp
