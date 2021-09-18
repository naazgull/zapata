
AC_PREREQ(2.59)
AC_INIT(zapata-net-tcp, 2.0.0-1, n@zgul.me)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])

AC_LANG_CPLUSPLUS
AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(yes)

dnl Check for libraries
AC_HAVE_LIBRARY(dl)
AC_HAVE_LIBRARY(pthread)
AC_HAVE_LIBRARY(magic)
AC_HAVE_LIBRARY(z)
AC_HAVE_LIBRARY(ssl)
AC_HAVE_LIBRARY(crypto)


AC_CANONICAL_SYSTEM

dnl Create config.h header file
AH_TOP([#include <zapata/net/transport/tcp/unconfig.h>])
AC_CONFIG_HEADERS([include/zapata/net/transport/tcp/config.h])

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

AC_SUBST([CPP_STD_VERSION])
AC_SUBST([INCLUDE_DIR])
AC_SUBST([PROJECT_DIR])

AC_CONFIG_FILES(Makefile
		include/Makefile
		src/tcp/Makefile)
AC_OUTPUT
