lib_LTLIBRARIES = libzapata-automaton.la libzapata-automaton-plugin.la

libzapata_automaton_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json			\
-lzapata-lex-uri -lzapata-common-graph -lzapata-common-lockfree					\
-lzapata-common-events -lzapata-net-socket -lzapata-transport -lzapata-pipeline
libzapata_automaton_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_automaton_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../../include -I../../include

libzapata_automaton_la_SOURCES = \
./automaton.cpp

libzapata_automaton_plugin_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json			\
-lzapata-lex-uri -lzapata-common-graph -lzapata-common-lockfree					\
-lzapata-common-events -lzapata-net-socket -lzapata-transport -lzapata-pipeline	\
libzapata-automaton.la
libzapata_automaton_plugin_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_automaton_plugin_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../../include -I../../include

libzapata_automaton_plugin_la_SOURCES = \
./plugin.cpp
