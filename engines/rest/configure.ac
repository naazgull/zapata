
AC_PREREQ([2.71])
AC_INIT([zapata-rest],[2.0.0-1],[n@zgul.me])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])

AC_LANG([C++])
AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(yes)

dnl Check for libraries
AC_CHECK_LIB([dl],[main],[],[],[])ac_cv_lib_dl=ac_cv_lib_dl_main

AC_CHECK_LIB([pthread],[main],[],[],[])ac_cv_lib_pthread=ac_cv_lib_pthread_main

AC_CHECK_LIB([magic],[main],[],[],[])ac_cv_lib_magic=ac_cv_lib_magic_main

AC_CHECK_LIB([z],[main],[],[],[])ac_cv_lib_z=ac_cv_lib_z_main

AC_CHECK_LIB([ssl],[main],[],[],[])ac_cv_lib_ssl=ac_cv_lib_ssl_main

AC_CHECK_LIB([crypto],[main],[],[],[])ac_cv_lib_crypto=ac_cv_lib_crypto_main



AC_CANONICAL_TARGET

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
LT_INIT

AC_SUBST([CPP_STD_VERSION])
AC_SUBST([INCLUDE_DIR])
AC_SUBST([PROJECT_DIR])

AC_CONFIG_FILES(Makefile
		include/Makefile
		src/rest/Makefile)
AC_OUTPUT
