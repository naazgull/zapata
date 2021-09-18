lib_LTLIBRARIES = libzapata-fsm.la

libzapata_fsm_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json	\
-lzapata-lex-uri -lzapata-common-lockfree -lzapata-common-events
libzapata_fsm_la_LDFLAGS = -version-info 2:0:0 -shared		\
-L@PROJECT_DIR@/common/base/build/src/base/.libs			\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs	\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs		\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs			\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs			\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs				\
-L@PROJECT_DIR@/common/graph/build/src/graph/.libs			\
-L@PROJECT_DIR@/common/events/build/src/events/.libs
libzapata_fsm_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@		\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include -I@PROJECT_DIR@/common/graph/include	\
-I@PROJECT_DIR@/common/events/include
libzapata_fsm_la_SOURCES = \
./fsm.cpp

bin_PROGRAMS = zfsm

zfsm_LDADD = -lpthread -lzapata-base -lzapata-lex-json				\
-lzapata-common-lockfree -lzapata-common-events libzapata-fsm.la
zfsm_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs		\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs				\
-L@PROJECT_DIR@/parsers/http/build/src/http/.libs				\
-L@PROJECT_DIR@/parsers/uri/build/src/uri/.libs					\
-L@PROJECT_DIR@/common/graph/build/src/graph/.libs				\
-L@PROJECT_DIR@/common/events/build/src/events/.libs
zfsm_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@					\
-I@PROJECT_DIR@/common/base/include											\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/common/globals/include										\
-I@PROJECT_DIR@/parsers/json/include -I@PROJECT_DIR@/parsers/http/include	\
-I@PROJECT_DIR@/parsers/uri/include -I@PROJECT_DIR@/common/graph/include	\
-I@PROJECT_DIR@/common/events/include
zfsm_SOURCES = \
./main.cpp
