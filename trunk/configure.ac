#####################################################################
#
# Copyright (c) 2003 Intel Corporation
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: 
# 
#    # Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#       
#    # Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#       
#    # Neither the name of the Intel Corporation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#       
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
# 
# EXPORT LAWS: THIS LICENSE ADDS NO RESTRICTIONS TO THE EXPORT LAWS OF
# YOUR JURISDICTION. It is licensee's responsibility to comply with any
# export regulations applicable in licensee's jurisdiction. Under
# CURRENT (May 2000) U.S. export regulations this software is eligible
# for export from the U.S. and can be downloaded by or otherwise
# exported or reexported worldwide EXCEPT to U.S. embargoed destinations
# which include Cuba, Iraq, Libya, North Korea, Iran, Syria, Sudan,
# Afghanistan and any other country to which the U.S. has embargoed
# goods and services.
# 
# DESCRIPTION: Configure script for asynchronous sensor server
# 
#   $Id$
# 
#

# Process this file with autoconf to produce a configure script.

AC_INIT(p2,0.1)
AC_CONFIG_SRCDIR(p2core/element.h)
# AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(p2,0.4)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
AM_PATH_PYTHON(2.3)
AC_PROG_SWIG(1.3.21)
SWIG_ENABLE_CXX
SWIG_PYTHON

# Checks for libraries.
P2_SFSLIB

# Checks for header files.
AC_HEADER_STDC
P2_SFSINC

AC_LANG_PUSH(C++)
CPPFLAGS="$SFS_INC"
AC_CHECK_TYPES([tcpconn_t *, tcpconnect_t *],[],[],[#include <async.h>])
AC_LANG_POP(C++)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Python!
# AZ_PYTHON_DEFAULT( )
AZ_PYTHON_ENABLE( )             # Optional
AZ_PYTHON_WITH( )               # Optional
AZ_PYTHON_PATH( )               # or AZ_PYTHON_INSIST( )
# if $PYTHON is not defined, then the following do nothing.
AZ_PYTHON_VERSION_ENSURE( [2.2] )

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([dup2 strchr strerror strstr])

AC_CONFIG_FILES([
	Makefile 
	eventLoop/Makefile
	p2core/Makefile 
	swig/Makefile
	elements/Makefile 
	net/Makefile 
	tests/Makefile 
	phid/Makefile
	overlog/Makefile 
	planner/Makefile ])
AC_OUTPUT
