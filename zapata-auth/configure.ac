dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(zapata, 0.9.1-1, n@zgul <naazgull@dfz.pt>)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])

dnl Check for libraries
AC_HAVE_LIBRARY(dl)
AC_HAVE_LIBRARY(pthread)
AC_HAVE_LIBRARY(magic)
AC_HAVE_LIBRARY(z)
AC_HAVE_LIBRARY(ssl)
AC_HAVE_LIBRARY(crypto)

AC_CANONICAL_SYSTEM

dnl Create config.h header file
AH_TOP([#include <zapata/unconfig.h>])
AC_CONFIG_HEADERS([include/zapata/config.h])

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
        src/zapata-core/Makefile
        src/zapata-http/Makefile
        src/zapata-mongodb/Makefile
        src/zapata-net/Makefile
        src/zapata-rest/Makefile
        src/zapata-auth/Makefile
        src/zapata-users/Makefile
        src/zapata-rest-server/Makefile)

AC_OUTPUT
