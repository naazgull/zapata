lib_LTLIBRARIES = libzapata-bridge-base.la

libzapata_bridge_base_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json
libzapata_bridge_base_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs				\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs
libzapata_bridge_base_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include				\
-I@PROJECT_DIR@/parsers/json/include

libzapata_bridge_base_la_SOURCES = \
./bridge.cpp

bin_PROGRAMS = zbridge

zbridge_LDADD = -lpthread -lzapata-base -lzapata-lex-json libzapata-bridge-base.la
zbridge_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs
zbridge_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@				\
-I@PROJECT_DIR@/common/base/include -I@PROJECT_DIR@/parsers/json/include

zbridge_SOURCES = \
./main.cpp
