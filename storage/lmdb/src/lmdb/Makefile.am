lib_LTLIBRARIES = libzapata-storage-lmdb.la

libzapata_storage_lmdb_la_LIBADD = -lpthread -lzapata-base				\
-lzapata-common-lockfree -lzapata-lex-json -lzapata-storage-connector	\
-llmdb
libzapata_storage_lmdb_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs				\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs		\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs				\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs				\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs					\
-L@PROJECT_DIR@/storage/connector/build/src/connector/.libs
libzapata_storage_lmdb_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall			\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include											\
-I@PROJECT_DIR@/storage/connector/include
libzapata_storage_lmdb_la_SOURCES = \
./connector.cpp

bin_PROGRAMS = zlmdb

zlmdb_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-lex-json -lzapata-storage-connector -llmdb				\
libzapata-storage-lmdb.la
zlmdb_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs					\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs						\
-L@PROJECT_DIR@/storage/connector/build/src/connector/.libs
zlmdb_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@					\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include											\
-I@PROJECT_DIR@/storage/connector/include
zlmdb_SOURCES = \
./main.cpp
