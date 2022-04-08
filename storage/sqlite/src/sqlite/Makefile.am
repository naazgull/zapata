lib_LTLIBRARIES = libzapata-storage-sqlite.la

libzapata_storage_sqlite_la_LIBADD = -lpthread -lzapata-base			\
-lzapata-common-lockfree -lzapata-lex-json -lzapata-storage-connector	\
-lsqlite3
libzapata_storage_sqlite_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs					\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs						\
-L@PROJECT_DIR@/storage/connector/build/src/connector/.libs
libzapata_storage_sqlite_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall			\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include											\
-I@PROJECT_DIR@/storage/connector/include
libzapata_storage_sqlite_la_SOURCES = \
./connector.cpp

bin_PROGRAMS = zsqlite

zsqlite_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-lex-json -lzapata-storage-connector -lsqlite3				\
libzapata-storage-sqlite.la
zsqlite_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs					\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs						\
-L@PROJECT_DIR@/storage/connector/build/src/connector/.libs
zsqlite_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@				\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include											\
-I@PROJECT_DIR@/storage/connector/include
zsqlite_SOURCES = \
./main.cpp
