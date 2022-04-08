lib_LTLIBRARIES = libzapata-storage-connector.la

libzapata_storage_connector_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-lex-json -lzapata-lex-functional
libzapata_storage_connector_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs						\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs				\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs						\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs						\
-L@PROJECT_DIR@/parsers/functional/build/src/functional/.libs			\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs
libzapata_storage_connector_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include						\
-I@PROJECT_DIR@/common/lockfree/include									\
-I@PROJECT_DIR@/common/globals/include									\
-I@PROJECT_DIR@/parsers/json/include									\
-I@PROJECT_DIR@/parsers/functional/include								\
-I@PROJECT_DIR@/parsers/uri/include
libzapata_storage_connector_la_SOURCES = \
./connector.cpp

bin_PROGRAMS = zconnector

zconnector_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-lex-json libzapata-storage-connector.la
zconnector_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs				\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs					\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs						\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs						\
-L@PROJECT_DIR@/parsers/functional/build/src/functional/.libs			\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs
zconnector_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@				\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/functional/include									\
-I@PROJECT_DIR@/parsers/uri/include
zconnector_SOURCES = \
./main.cpp
