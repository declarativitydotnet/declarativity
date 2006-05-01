########################################################################
#
#  @(#)$Id$
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#  
#
# DESCRIPTION: Makefile for the P2 core elements python library
#

lib_LTLIBRARIES = libp2pythonoverlog.la

libp2pythonoverlog_la_SOURCES = \
	overlogCompiler.cpp

INCLUDES = @BOOST_INC@ -I$(top_builddir)/p2core \
                       -I$(top_builddir)/elements \
                       -I$(top_builddir)/eventLoop \
                       -I$(top_builddir)/overlog 

CLEANFILES = core *.core *~

MAINTAINERCLEANFILES = Makefile.in

CXXFLAGS= -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG  @CXXFLAGS@ ${PYTHON_CPPFLAGS}
