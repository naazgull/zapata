lib_LTLIBRARIES = libzapata-base.la

libzapata_base_la_LIBADD = -lpthread -lmagic -lossp-uuid++ -lcrypto++ 
libzapata_base_la_LDFLAGS = -version-info 9:1:9
libzapata_base_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_base_la_SOURCES = \
./assert.cpp \
./ascii.cpp \
./AssertionException.cpp \
./base64.cpp \
./CastException.cpp \
./ClosedException.cpp \
./convert.cpp \
./filesystem.cpp \
./html.cpp \
./info.cpp \
./InterruptedException.cpp \
./log.cpp \
./Makefile.am \
./manip.cpp \
./NoAttributeNameException.cpp \
./ParserEOF.cpp \
./send.cpp \
./SyntaxErrorException.cpp \
./url.cpp \
./usage.cpp \
./utf8.cpp

bin_PROGRAMS = zoc

zoc_LDADD = -lpthread -lzapata-base
zoc_LDFLAGS= 
zoc_CPPFLAGS = -std=c++11 -Wall -I../../include

zoc_SOURCES = \
zoc.cpp
