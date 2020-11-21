lib_LTLIBRARIES = libzapata-common-events.la

libzapata_common_events_la_LIBADD = -lpthread -lmagic -lzapata-base -lzapata-lex-json
libzapata_common_events_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_common_events_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_common_events_la_SOURCES = \
./dispatcher.cpp

bin_PROGRAMS = zevents

zevents_LDADD = -lpthread -lmagic -lzapata-base -lzapata-lex-json libzapata-common-events.la
zevents_LDFLAGS =
zevents_CPPFLAGS = -O3 -std=c++17 -I../../include

zevents_SOURCES = \
./main.cpp
