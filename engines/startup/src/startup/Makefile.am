lib_LTLIBRARIES = libzapata-startup.la libzapata-plugin-example.la

libzapata_startup_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-uri	\
-lzapata-lex-json -lzapata-common-lockfree -lzapata-common-events		\
-lzapata-io-streams -lzapata-transport
libzapata_startup_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_startup_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_startup_la_SOURCES = \
./configuration.cpp \
./startup.cpp

libzapata_plugin_example_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-uri	\
-lzapata-lex-json -lzapata-io-streams -lzapata-transport libzapata-startup.la
libzapata_plugin_example_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_plugin_example_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_plugin_example_la_SOURCES = \
./plugin.cpp

bin_PROGRAMS = zpt

zpt_LDADD = -lpthread -lzapata-base -lzapata-lex-uri -lzapata-lex-json	\
-lzapata-common-lockfree -lzapata-common-events -lzapata-io-streams		\
-lzapata-transport libzapata-startup.la
zpt_LDFLAGS =
zpt_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zpt_SOURCES = \
./main.cpp
