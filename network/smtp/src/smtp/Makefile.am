
lib_LTLIBRARIES = libzapata-smtp.la

libzapata_smtp_la_LIBADD = -lpthread -letpan -lzapata-base -lzapata-lex-json
libzapata_smtp_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_smtp_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_smtp_la_SOURCES = \
./SMTP.cpp
