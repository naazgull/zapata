lib_LTLIBRARIES = libzapata-auth.la

libzapata_auth_la_LIBADD = -lpthread -lzapata-core -lzapata-http -lzapata-net
libzapata_auth_la_LDFLAGS= -L../zapata-core/.libs -L../zapata-http/.libs -L../zapata-net/.libs -version-info 9:1:9
libzapata_auth_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_auth_la_SOURCES = \
./AuthAgent.cpp \
./hawk/AuthAgent.cpp \
./oauth2/AuthAgent.cpp