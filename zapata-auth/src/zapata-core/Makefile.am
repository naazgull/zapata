lib_LTLIBRARIES = libzapata-core.la

libzapata_core_la_LIBADD = -lpthread -lmagic
libzapata_core_la_LDFLAGS = -version-info 9:1:9
libzapata_core_la_CPPFLAGS = -O3 -std=c++11 -I../../include

libzapata_core_la_SOURCES = \
./base/assert.cpp \
./log/log.cpp \
./thread/Job.cpp \
./thread/TimerJob.cpp \
./mem/usage.cpp \
./conf/load.cpp \
./exceptions/InterruptedException.cpp \
./exceptions/CastException.cpp \
./exceptions/ParserEOF.cpp \
./exceptions/AssertionException.cpp \
./exceptions/NoAttributeNameException.cpp \
./exceptions/SyntaxErrorException.cpp \
./parsers/JSONLexer.cpp \
./parsers/json.cpp \
./parsers/JSONTokenizer.cpp \
./parsers/JSONTokenizerLexer.cpp \
./parsers/JSONParser.cpp \
./file/info.cpp \
./file/filesystem.cpp \
./mail/send.cpp \
./text/ascii.cpp \
./text/html.cpp \
./text/utf8.cpp \
./text/manip.cpp \
./text/url.cpp \
./text/convert.cpp \
./text/base64.cpp \
./json/JSONObj.cpp 

bin_PROGRAMS = zat

zat_LDADD = -lpthread -lzapata-core
zat_LDFLAGS= 
zat_CPPFLAGS = -O3 -std=c++11 -Wall -I../../include

zat_SOURCES = \
./log/zat.cpp
