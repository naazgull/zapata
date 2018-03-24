lib_LTLIBRARIES = libzapata-gen.la

libzapata_gen_la_LIBADD = -lpthread -lzapata-base -lzapata-json
libzapata_gen_la_LDFLAGS = -version-info 1:0:0
libzapata_gen_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_gen_la_SOURCES = \
./Generator.cpp

bin_PROGRAMS = zgen

zgen_LDADD = -lpthread -lmagic -lzapata-json -lzapata-gen
zgen_LDFLAGS =  -L../gen/.libs
zgen_CPPFLAGS = -O3 -std=c++11 -I../../include

zgen_SOURCES = \
./main.cpp
