lib_LTLIBRARIES = libzapata-ontology-openapi.la

libzapata_ontology_openapi_la_LIBADD = -lpthread -lzapata-base				\
-lzapata-lex-json -lzapata-pipeline -lzapata-startup -lzapata-dom-processor
libzapata_ontology_openapi_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_ontology_openapi_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_ontology_openapi_la_SOURCES = \
./openapi.cpp \
./plugin.cpp

bin_PROGRAMS = zopenapi

zopenapi_LDADD = -lpthread -lzapata-base -lzapata-lex-json -lzapata-pipeline	\
-lzapata-startup -lzapata-dom-processor libzapata-ontology-openapi.la
zopenapi_LDFLAGS =
zopenapi_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zopenapi_SOURCES = \
./main.cpp
