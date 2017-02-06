lib_LTLIBRARIES = libzapata-json.la

libzapata_json_la_LIBADD = -lpthread -lmagic -lzapata-base
libzapata_json_la_LDFLAGS = -version-info 9:1:9 
libzapata_json_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_json_la_SOURCES = \
./json.cpp \
./JSONLexer.cpp \
./JSONObj.cpp \
./JSONParser.cpp \
./JSONTokenizer.cpp \
./JSONTokenizerLexer.cpp

bin_PROGRAMS = zat zpt-json zinit

zat_LDADD = -lpthread -lzapata-base -lzapata-json
zat_LDFLAGS= 
zat_CPPFLAGS = -std=c++14 -Wall -I../../include

zat_SOURCES = \
zat.cpp

zpt_json_LDADD = -lpthread -lzapata-base -lzapata-json
zpt_json_LDFLAGS= 
zpt_json_CPPFLAGS = -std=c++14 -Wall -I../../include

zpt_json_SOURCES = \
parser.cpp

zinit_LDADD = -lpthread -lzapata-base -lzapata-json
zinit_LDFLAGS= 
zinit_CPPFLAGS = -std=c++14 -Wall -I../../include

zinit_SOURCES = \
zinit.cpp
