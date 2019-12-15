lib_LTLIBRARIES = libzapata-kernel-matrix.la

libzapata_kernel_matrix_la_LIBADD = -lpthread -lzapata-base
libzapata_kernel_matrix_la_LDFLAGS = -version-info 1:0:0
libzapata_kernel_matrix_la_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

libzapata_kernel_matrix_la_SOURCES = \
./XXX.cpp

bin_PROGRAMS = zmatrix

zmatrix_LDADD = -lpthread -lzapata-kernel-matrix
zmatrix_LDFLAGS =
zmatrix_CPPFLAGS = -O3 -std=c++17 -Wall -I../../include

zmatrix_SOURCES = \
./main.cpp
