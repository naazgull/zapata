lib_LTLIBRARIES = libzapata-common-logging.la

libzapata_common_logging_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json -lzapata-io-streams
libzapata_common_logging_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_common_logging_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_common_logging_la_SOURCES = \
./logging.cpp

bin_PROGRAMS = zlogging

zlogging_LDADD = -lpthread libzapata-common-logging.la
zlogging_LDFLAGS =
zlogging_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zlogging_SOURCES = \
./main.cpp
