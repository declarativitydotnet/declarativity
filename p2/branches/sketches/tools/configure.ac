# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(tools, 0.54b, mhadji@gmail.com)
AC_CANONICAL_BUILD

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AM_INIT_AUTOMAKE

# Checks for header files.
AC_CHECK_HEADERS(fcntl.h,, [AC_MSG_ERROR([cannot find fcntl.h, bailing out])])
AC_CHECK_HEADERS(unistd.h,, [AC_MSG_ERROR([cannot find unistd.h, bailing out])])
AC_CHECK_HEADERS(sys/types.h,, [AC_MSG_ERROR([cannot find sys/types.h, bailing out])])
AC_CHECK_HEADERS(sys/stat.h,, [AC_MSG_ERROR([cannot find sys/stat.h, bailing out])])
AC_CHECK_HEADERS(sys/resource.h,, [AC_MSG_ERROR([cannot find sys/resource.h, bailing out])])
AC_CHECK_HEADERS(stdint.h,, [AC_MSG_ERROR([cannot find stdint.h, bailing out])])
AC_CHECK_HEADERS(pthread.h, [LIBS="$LIBS -lpthread"])
AC_HEADER_TIME

# Checks for library functions.
AC_CHECK_FUNCS(gettimeofday)

echo $build_os

if test $build_cpu = i686; then
	AC_DEFINE(BUILD_CPU_I686)
fi

if test $build_os = cygwin; then
	AC_DEFINE(BUILD_OS_CYGWIN)
fi

AC_CONFIG_FILES([Makefile
				src/Makefile
				src/geometry/Makefile
				src/tools/Makefile
				regressiontest/Makefile])

AC_OUTPUT

