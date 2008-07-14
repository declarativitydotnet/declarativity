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
# DESCRIPTION: Makefile for the P2 debugging library
#

lib_LTLIBRARIES = libp2debugger.la

libp2debugger_la_SOURCES = \
	tableTracer.C \
	ruleTracer.C \
	tap.C \
	execRecord.C \
	traceTuple.C

INCLUDES = @BOOST_INC@ -I../p2core -I../eventLoop -I../elements

CLEANFILES = core *.core *~

EXTRA_DIST = .cvsignore \
		$(libp2debugger_la_SOURCES:.C=.h)

MAINTAINERCLEANFILES = Makefile.in

CXXFLAGS=-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG @CXXFLAGS@ ${PYTHON_CPPFLAGS}

