
lib_LTLIBRARIES = libzapata-smtp.la

libzapata_smtp_la_LIBADD = -lpthread -letpan -lzapata-base -lzapata-lex-json
libzapata_smtp_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_smtp_la_CPPFLAGS = -O3 @CPP_STD_VERSION@

libzapata_smtp_la_SOURCES = \
./SMTP.cpp
