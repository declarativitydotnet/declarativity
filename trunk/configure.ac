#####################################################################
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
# 
# DESCRIPTION: Configure script for P2
# 
#   $Id$
# 
# Process this file with autoconf to produce a configure script.

AC_INIT(p2,0.1)
AC_CONFIG_SRCDIR(p2core/element.h)
# AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(p2,0.7.2)


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
AM_PATH_PYTHON(2.3)
## We have given up on SWIG! -- JMH 2/26/06
#AC_PROG_SWIG(1.3.21)
#SWIG_ENABLE_CXX
#SWIG_PYTHON

# Checks for libraries.
P2_BOOSTLIB
AC_PYTHON_DEVEL(2.3)

# Checks for header files.
AC_HEADER_STDC
P2_BOOSTINC

AC_LANG_PUSH(C++)
AC_CHECK_HEADERS([rpc/rpc.h])
AC_CHECK_HEADERS([rpc/xdr.h], [], [],
[# include <rpc/rpc.h>])
AC_CHECK_FUNCS([xdr_uint32_t xdr_u_int32_t],[],[],[#include <rpc/xdr.h>])
AC_CHECK_FUNCS([exp10],[],[],[#include <math.h>])
# Check Boost's vector and matrix insertion, which changed at v. 1_33
## XXX This hard-earned test, while accurate, is no longer used since
## we don't need to call the insert methods for vector and matrix.
## I'll comment it out for now to speed builds, but save it for future
## use in case it's needed.  -- JMH
## save_CPPFLAGS="$CPPFLAGS"
## CPPFLAGS="$CPPFLAGS $BOOST_INC"
## AC_COMPILE_IFELSE([
##   AC_LANG_PROGRAM([#include <boost/numeric/ublas/vector.hpp>],
##    [[boost::numeric::ublas::vector<int> v(3);  
##      v.insert_element(0, 0);
##   ]]) ], [AC_DEFINE(BOOST_VEC_INSERT,33,[Boost vector insert version])],
##         [AC_DEFINE(BOOST_VEC_INSERT,32,[Boost vector insert version])]
## )
## AC_COMPILE_IFELSE([
##   AC_LANG_PROGRAM([#include <boost/numeric/ublas/matrix.hpp>],
##    [[boost::numeric::ublas::matrix<int> m(3,3);  
##      m.insert_element(0, 0, 0);
##   ]]) ], [AC_DEFINE(BOOST_MAT_INSERT,33,[Boost matrix insert version])],
##         [AC_DEFINE(BOOST_MAT_INSERT,32,[Boost matrix insert version])]
## )
## CPPFLAGS="$save_CPPFLAGS"
AC_LANG_POP(C++)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_TYPE([socklen_t],[],[],[#include <sys/socket.h>])
AC_CHECK_SOCKADDR_IN_SIN_LEN

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([dup2 strchr strerror strstr])

AC_CONFIG_FILES([
	Makefile 
	eventLoop/Makefile
	p2core/Makefile 
	p2cpp/Makefile 
	python/Makefile
	python/p2/Makefile
	python/p2/debugger/Makefile
	python/p2/elements/Makefile
	python/p2/eventLoop/Makefile
	python/p2/p2core/Makefile
	python/p2/net/Makefile
	python/p2/overlog/Makefile
	python/p2/p2cpp/Makefile
	python/dfparser/Makefile
	python/dfparser/yapps/Makefile
	python/scripts/Makefile
	elements/Makefile 
	net/Makefile 
	tests/Makefile 
	unitTests/Makefile 
	overlog/Makefile 
	debugger/Makefile ])

AC_OUTPUT
