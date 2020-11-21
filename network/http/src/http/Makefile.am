lib_LTLIBRARIES = libzapata-net-http.la libzapata-net-http-plugin.la

libzapata_net_http_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json	\
-lzapata-lex-http -lzapata-lex-uri -lzapata-net-socket -lzapata-transport
libzapata_net_http_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_net_http_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_net_http_la_SOURCES = \
./http.cpp

libzapata_net_http_plugin_la_LIBADD = -lpthread -lzapata-base -lzapata-lex-json		\
-lzapata-lex-http -lzapata-lex-uri -lzapata-net-socket -lzapata-transport libzapata-net-http.la
libzapata_net_http_plugin_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_net_http_plugin_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_net_http_plugin_la_SOURCES = \
./plugin.cpp
