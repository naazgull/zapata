lib_LTLIBRARIES = libzapata-dom-processor.la

libzapata_dom_processor_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json	\
-lzapata-lex-uri -lzapata-common-graph -lzapata-common-lockfree					\
-lzapata-common-events -lzapata-pipeline
libzapata_dom_processor_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_dom_processor_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

libzapata_dom_processor_la_SOURCES = \
./dom.cpp

bin_PROGRAMS = zdom

zdom_LDADD = -lpthread -lzapata-base -lzapata-lex-json -lzapata-lex-uri	\
-lzapata-common-graph -lzapata-common-lockfree -lzapata-common-events	\
-lzapata-pipeline libzapata-dom-processor.la
zdom_LDFLAGS =
zdom_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

zdom_SOURCES = \
./main.cpp
