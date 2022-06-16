lib_LTLIBRARIES = libzapata-common-events.la

libzapata_common_events_la_LIBADD = -lpthread -lmagic -lzapata-base	\
-lzapata-common-lockfree -lzapata-lex-json
libzapata_common_events_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs
libzapata_common_events_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ @INCLUDE_DIR@	\
-I@PROJECT_DIR@/common/base/include -I@PROJECT_DIR@/common/globals/include	\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/parsers/json/include

libzapata_common_events_la_SOURCES = \
./dispatcher.cpp

bin_PROGRAMS = zevents

zevents_LDADD = -lpthread -lmagic -lzapata-base -lzapata-common-lockfree	\
-lzapata-lex-json libzapata-common-events.la
zevents_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs			\
-L@PROJECT_DIR@/parsers/json/build/src/json/.libs
zevents_CPPFLAGS = -O3 @CPP_STD_VERSION@ @INCLUDE_DIR@						\
-I@PROJECT_DIR@/common/base/include -I@PROJECT_DIR@/common/globals/include	\
-I@PROJECT_DIR@/common/lockfree/include										\
-I@PROJECT_DIR@/parsers/json/include

zevents_SOURCES = \
./main.cpp
