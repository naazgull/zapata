lib_LTLIBRARIES = libzapata-common-lockfree.la

libzapata_common_lockfree_la_LIBADD = -lpthread -lzapata-base
libzapata_common_lockfree_la_LDFLAGS = -version-info 2:0:0 -shared	\
-L@PROJECT_DIR@/common/base/build/src/base/.libs
libzapata_common_lockfree_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
@INCLUDE_DIR@ -I@PROJECT_DIR@/common/base/include

libzapata_common_lockfree_la_SOURCES = \
./spin_lock.cpp

bin_PROGRAMS = zlf

zlf_LDADD = -lpthread -lzapata-base libzapata-common-lockfree.la
zlf_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs
zlf_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@	\
-I@PROJECT_DIR@/common/base/include

zlf_SOURCES = \
./main.cpp
