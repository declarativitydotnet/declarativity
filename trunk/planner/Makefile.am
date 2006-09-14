########################################################################
#
#  @(#)$Id$
#
# Copyright (c) 2005 Intel Corporation. All rights reserved.
#
# This file is distributed under the terms in the attached INTEL-LICENSE file.
# If you do not find these files, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
#
# DESCRIPTION: Makefile for the P2 planner
#


lib_LTLIBRARIES = libp2planner.la
libp2planner_la_SOURCES = tableStore.C \
			planContext.C \
			localize_context.C \
			eca_context.C \
			ruleStrand.C \
			netPlanner.C \
			planner.C 


INCLUDES = @BOOST_INC@ -I../p2core -I../elements -I../net -I../overlog -I../eventLoop -I../debugger

CLEANFILES = core *.core *~

EXTRA_DIST = $(libp2core_la_SOURCES:.C=.h) \
		rulePlanner.C \
		rulePel.C

MAINTAINERCLEANFILES = Makefile.in

CXXFLAGS= -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG  @CXXFLAGS@
