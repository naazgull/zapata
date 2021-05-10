lib_LTLIBRARIES = libzapata-pipeline.la

libzapata_pipeline_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json -lzapata-lex-uri -lzapata-common-graph -lzapata-common-lockfree -lzapata-common-events -lzapata-net-socket
libzapata_pipeline_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_pipeline_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

libzapata_pipeline_la_SOURCES = \
./pipeline.cpp

bin_PROGRAMS = zpipeline

zpipeline_LDADD = -lpthread -lzapata-base -lzapata-lex-json -lzapata-lex-uri -lzapata-common-graph -lzapata-common-lockfree -lzapata-common-events -lzapata-net-socket libzapata-pipeline.la
zpipeline_LDFLAGS =
zpipeline_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include -I../../include

zpipeline_SOURCES = \
./main.cpp
