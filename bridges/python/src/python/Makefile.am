lib_LTLIBRARIES = libzapata-bridge-python.la libzapata-bridge-python-plugin.la

libzapata_bridge_python_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-lex-json -lzapata-bridge-base -lpython@PYTHON_VERSION@
libzapata_bridge_python_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_bridge_python_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall			\
-I../../../include -I../../include -I/usr/include/python@PYTHON_VERSION@

libzapata_bridge_python_la_SOURCES = \
./python.cpp

libzapata_bridge_python_plugin_la_LIBADD = -lpthread -lzapata-base	\
-lzapata-lex-json -lzapata-bridge-base -lpython@PYTHON_VERSION@		\
libzapata-bridge-python.la
libzapata_bridge_python_plugin_la_LDFLAGS = -version-info 2:0:0 -shared
libzapata_bridge_python_plugin_la_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall	\
-I../../../include -I../../include -I/usr/include/python@PYTHON_VERSION@

libzapata_bridge_python_plugin_la_SOURCES = \
./plugin.cpp

bin_PROGRAMS = zpython

zpython_LDADD = -lpthread -lzapata-base -lzapata-lex-json		\
-lzapata-bridge-base -lzapata-lex-json -lpython@PYTHON_VERSION@	\
libzapata-bridge-python.la
zpython_LDFLAGS =
zpython_CPPFLAGS = -O3 @CPP_STD_VERSION@ -Wall -I../../../include	\
-I../../include -I/usr/include/python@PYTHON_VERSION@

zpython_SOURCES = \
./main.cpp
