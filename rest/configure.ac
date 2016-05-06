dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(zapata-rest, 0.9.1-1, n@zgul <naazgull@dfz.pt>)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])

dnl Check for libraries
AC_HAVE_LIBRARY(dl)
AC_HAVE_LIBRARY(pthread)
AC_HAVE_LIBRARY(magic)
AC_HAVE_LIBRARY(z)
AC_HAVE_LIBRARY(ssl)
AC_HAVE_LIBRARY(crypto)
AC_HAVE_LIBRARY(zmq)
AC_HAVE_LIBRARY(ossp-uuid++)

AC_CHECK_LIB([zapata-base], [zapata_base], [], [AC_MSG_ERROR([zapata-base not found])])
AC_CHECK_LIB([zapata-json], [zapata_json], [], [AC_MSG_ERROR([zapata-json not found])])
AC_CHECK_LIB([zapata-events], [zapata_events], [], [AC_MSG_ERROR([zapata-events not found])])
AC_CHECK_LIB([zapata-http], [zapata_http], [], [AC_MSG_ERROR([zapata-http not found])])

AC_CANONICAL_SYSTEM

dnl Create config.h header file
AH_TOP([#include <zapata/rest/unconfig.h>])
AC_CONFIG_HEADERS([include/zapata/rest/config.h])

dnl Initialize automake
AM_INIT_AUTOMAKE([subdir-objects tar-pax])

dnl this allows us specify individual liking flags for each target
AC_PROG_CXX
AM_PROG_CC_C_O 

dnl Initialize Libtool
LT_INIT

dnl Check if Libtool is present
dnl Libtool is used for building share libraries 
AC_PROG_LIBTOOL

AC_CONFIG_FILES(Makefile
        debian/Makefile
        etc/Makefile
        include/Makefile
        src/rest/Makefile)

AC_OUTPUT
