lib_LTLIBRARIES = libzapata-base.la

libzapata_base_la_LIBADD = -lpthread -lmagic -lossp-uuid++ -lcrypto++
libzapata_base_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_base_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ @INCLUDE_DIR@

libzapata_base_la_SOURCES = \
./performative.cpp \
./expect.cpp \
./ascii.cpp \
./Exception.cpp \
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
./utf8.cpp \
./sha1.cpp \
./sha256.cpp \
./sha512.cpp

bin_PROGRAMS = zoc

zoc_LDADD = -lpthread libzapata-base.la
zoc_LDFLAGS =
zoc_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall @INCLUDE_DIR@

zoc_SOURCES = \
zoc.cpp
