lib_LTLIBRARIES = libzapata-storage-mysqlx.la

libzapata_storage_mysqlx_la_LIBADD = -lpthread -lzapata-base			\
-lzapata-common-lockfree -lzapata-lex-json -lzapata-storage-connector	\
-lmysqlcppconn8
libzapata_storage_mysqlx_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs					\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs						\
-L@PROJECT_DIR@/storage/connector/build/src/connector/.libs
libzapata_storage_mysqlx_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall			\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include											\
-I@PROJECT_DIR@/storage/connector/include
libzapata_storage_mysqlx_la_SOURCES = \
./translate.cpp \
./connector.cpp

bin_PROGRAMS = zmysqlx

zmysqlx_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree	\
-lzapata-lex-json -lzapata-storage-connector -lmysqlcppconn8		\
libzapata-storage-mysqlx.la
zmysqlx_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs				\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs					\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs					\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs						\
-L@PROJECT_DIR@/storage/connector/build/src/connector/.libs
zmysqlx_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@				\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include											\
-I@PROJECT_DIR@/storage/connector/include
zmysqlx_SOURCES = \
./main.cpp
