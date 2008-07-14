# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(sketches, 0.24b, mhadji@gmail.com)
AC_CANONICAL_BUILD

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AM_INIT_AUTOMAKE

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h,, [AC_MSG_ERROR([cannot find fcntl.h, bailing out])])
AC_CHECK_HEADERS(unistd.h,, [AC_MSG_ERROR([cannot find unistd.h, bailing out])])
AC_CHECK_HEADERS(sys/types.h,, [AC_MSG_ERROR([cannot find sys/types.h, bailing out])])
AC_CHECK_HEADERS(sys/stat.h,, [AC_MSG_ERROR([cannot find sys/stat.h, bailing out])])
AC_CHECK_HEADERS(pthread.h, [LIBS="$LIBS -lpthread"])

MH_CXX_HEADER_TOOLS

LIBS="$LIBS -ltools"

if test $build_cpu = i686; then
	AC_DEFINE(BUILD_CPU_I686)
fi

if test $build_os = cygwin; then
	AC_DEFINE(BUILD_OS_CYGWIN)
fi

AC_CONFIG_FILES([Makefile
				src/Makefile
				regressiontest/Makefile])

AC_OUTPUT

