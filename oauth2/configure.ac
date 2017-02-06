dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(zapata-oauth2, 0.9.1-4, n@zgul <naazgull@dfz.pt>)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])

dnl Check for libraries
AC_HAVE_LIBRARY(dl)
AC_HAVE_LIBRARY(magic)
AC_HAVE_LIBRARY(z)
AC_HAVE_LIBRARY(ssl)
AC_HAVE_LIBRARY(crypto)
AC_HAVE_LIBRARY(zmq)
AC_HAVE_LIBRARY(ossp-uuid++)
AC_HAVE_LIBRARY(zapata-base)
AC_HAVE_LIBRARY(zapata-json)
AC_HAVE_LIBRARY(zapata-events)
AC_HAVE_LIBRARY(zapata-zmq)
AC_HAVE_LIBRARY(zapata-http)
AC_HAVE_LIBRARY(zapata-rest)

AC_CANONICAL_SYSTEM

dnl Create config.h header file
AH_TOP([#include <zapata/oauth2/unconfig.h>])
AC_CONFIG_HEADERS([include/zapata/oauth2/config.h])

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
        include/Makefile
        src/api/Makefile)

AC_OUTPUT
