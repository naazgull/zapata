dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(zapata-users, 0.9.1-1, n@zgul <naazgull@dfz.pt>)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])

dnl Check for libraries
AC_HAVE_LIBRARY(dl)
AC_HAVE_LIBRARY(pthread)
AC_HAVE_LIBRARY(magic)
AC_HAVE_LIBRARY(z)
AC_HAVE_LIBRARY(ssl)
AC_HAVE_LIBRARY(crypto)
AC_HAVE_LIBRARY(mongoclient)
AC_HAVE_LIBRARY(boost_system)
AC_HAVE_LIBRARY(boost_program_options)
AC_HAVE_LIBRARY(boost_thread)
AC_HAVE_LIBRARY(boost_filesystem)
AC_HAVE_LIBRARY(zmq)
AC_HAVE_LIBRARY(ossp-uuid++)

AC_CHECK_LIB([zapata-base], [zapata_base], [], [AC_MSG_ERROR([zapata-base not found])])
AC_CHECK_LIB([zapata-json], [zapata_json], [], [AC_MSG_ERROR([zapata-json not found])])
AC_CHECK_LIB([zapata-0mq], [zapata_0mq], [], [AC_MSG_ERROR([zapata-0mq not found])])
AC_CHECK_LIB([zapata-http], [zapata_http], [], [AC_MSG_ERROR([zapata-http not found])])
AC_CHECK_LIB([zapata-rest], [zapata_rest], [], [AC_MSG_ERROR([zapata-rest not found])])

AC_CANONICAL_SYSTEM

dnl Create config.h header file
AH_TOP([#include <zapata/users/unconfig.h>])
AC_CONFIG_HEADERS([include/zapata/users/config.h])

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
        src/api/Makefile)

AC_OUTPUT
