dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(zapata, 0.0.8, pedro.figueiredo@gmail.com)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])

AC_ARG_ENABLE(mongodb, [  --enable-mongodb    Turn on MongoDB support],
	[ AC_DEFINE(HAVE_MONGO_DB, 1, "Turn on MongoDB support") ]
)

dnl Check for libraries
AC_HAVE_LIBRARY(dl)
AC_HAVE_LIBRARY(pthread)
AC_HAVE_LIBRARY(magic)
AC_HAVE_LIBRARY(z)
AC_HAVE_LIBRARY(zmq)
AC_HAVE_LIBRARY(ssl)
AC_HAVE_LIBRARY(crypto)

AC_CANONICAL_SYSTEM

dnl Create config.h header file
AC_CONFIG_HEADERS([include/zapata/config.h])

dnl Initialize automake
AM_INIT_AUTOMAKE([subdir-objects])

dnl this allows us specify individual liking flags for each target
AC_PROG_CXX
AM_PROG_CC_C_O 

dnl Initialize Libtool
LT_INIT

dnl Check if Libtool is present
dnl Libtool is used for building share libraries 
AC_PROG_LIBTOOL

AC_CONFIG_FILES(Makefile
                include/Makefile
                src/zapata-core/Makefile
                src/zapata-http/Makefile
                src/zapata-mongodb/Makefile
                src/zapata-net/Makefile
                src/zapata-rest/Makefile
                src/zapata-users/Makefile
                src/zapata-rest-server/Makefile)
AC_OUTPUT
