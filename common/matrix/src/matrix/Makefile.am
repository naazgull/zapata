lib_LTLIBRARIES = libzapata-common-matrix.la

libzapata_common_matrix_la_LIBADD = -lpthread -lzapata-base
libzapata_common_matrix_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_common_matrix_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_common_matrix_la_SOURCES = \
./XXX.cpp

bin_PROGRAMS = zmatrix

zmatrix_LDADD = -lpthread libzapata-common-matrix.la
zmatrix_LDFLAGS =
zmatrix_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zmatrix_SOURCES = \
./main.cpp
