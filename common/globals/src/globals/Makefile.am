bin_PROGRAMS = zglobals

zglobals_LDADD = -lpthread -lzapata-base -lzapata-common-lockfree
zglobals_LDFLAGS = -L@PROJECT_DIR@/common/base/build/src/base/.libs	\
-L@PROJECT_DIR@/common/lockfree/build/src/lockfree/.libs
zglobals_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@	\
-I@PROJECT_DIR@/common/base/include								\
-I@PROJECT_DIR@/common/lockfree/include

zglobals_SOURCES = \
./main.cpp
