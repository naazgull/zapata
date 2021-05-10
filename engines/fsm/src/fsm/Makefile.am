lib_LTLIBRARIES = libzapata-fsm.la

libzapata_fsm_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json	\
-lzapata-lex-uri -lzapata-common-lockfree -lzapata-common-events
libzapata_fsm_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_fsm_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

libzapata_fsm_la_SOURCES = \
./fsm.cpp

bin_PROGRAMS = zfsm

zfsm_LDADD = -lpthread -lzapata-base -lzapata-lex-json				\
-lzapata-common-lockfree -lzapata-common-events libzapata-fsm.la
zfsm_LDFLAGS =
zfsm_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

zfsm_SOURCES = \
./main.cpp
