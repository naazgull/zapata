lib_LTLIBRARIES = libzapata-common-lockfree.la

libzapata_common_lockfree_la_LIBADD = -lpthread -lzapata-base
libzapata_common_lockfree_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs					\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs
libzapata_common_lockfree_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include					\
-I@PROJECT_DIR@/common/globals/include

libzapata_common_lockfree_la_SOURCES = \
./lockfree.cpp

bin_PROGRAMS = zlf

zlf_LDADD = -lpthread -lzapata-base libzapata-common-lockfree.la
zlf_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/globals/build/src/globals/.libs
zlf_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@	\
-I@PROJECT_DIR@/common/base/include							\
-I@PROJECT_DIR@/common/globals/include

zlf_SOURCES = \
./main.cpp
