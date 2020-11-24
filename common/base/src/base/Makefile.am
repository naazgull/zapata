lib_LTLIBRARIES = libzapata-base.la

libzapata_base_la_LIBADD = -lpthread -lmagic -lossp-uuid++ -lcrypto++
libzapata_base_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_base_la_CPPFLAGS = -O3 -std=c++17 -I../../../include -I../../include

libzapata_base_la_SOURCES = \
./expect.cpp \
./ascii.cpp \
./ExpectationException.cpp \
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
./NoMoreElementsException.cpp \
./ParserEOF.cpp \
./send.cpp \
./SyntaxErrorException.cpp \
./url.cpp \
./usage.cpp \
./net.cpp \
./utf8.cpp

bin_PROGRAMS = zoc

zoc_LDADD = -lpthread libzapata-base.la
zoc_LDFLAGS=
zoc_CPPFLAGS = -std=c++17 -Wall -I../../../include -I../../include

zoc_SOURCES = \
zoc.cpp
