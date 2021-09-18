lib_LTLIBRARIES = libzapata-bridge-prolog.la libzapata-bridge-prolog-plugin.la

libzapata_bridge_prolog_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-lex-json -lzapata-bridge-base
libzapata_bridge_prolog_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/bridges/base/build/src/bridge/.libs					\
-L@PROJECT_DIR@/common/events/build/src/events/.libs				\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
libzapata_bridge_prolog_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall			\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/bridges/base/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/engines/startup/include

libzapata_bridge_prolog_la_SOURCES = \
./prolog.cpp

libzapata_bridge_prolog_plugin_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-lex-json -lzapata-bridge-base libzapata-bridge-prolog.la
libzapata_bridge_prolog_plugin_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs						\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs				\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs						\
-L@PROJECT_DIR@/bridges/base/build/src/bridge/.libs						\
-L@PROJECT_DIR@/common/events/build/src/events/.libs					\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
libzapata_bridge_prolog_plugin_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/bridges/base/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/engines/startup/include

libzapata_bridge_prolog_plugin_la_SOURCES = \
./plugin.cpp

bin_PROGRAMS = zprolog

zprolog_LDADD = -lpthread -lzapata-base -lzapata-lex-json			\
-lzapata-bridge-base -lzapata-lex-json libzapata-bridge-prolog.la
zprolog_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/bridges/base/build/src/bridge/.libs					\
-L@PROJECT_DIR@/common/events/build/src/events/.libs				\
-L@PROJECT_DIR@/engines/startup/build/src/startup/.libs
zprolog_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@				\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/bridges/base/include	\
-I@PROJECT_DIR@/common/events/include										\
-I@PROJECT_DIR@/engines/startup/include

zprolog_SOURCES = \
./main.cpp
