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

SUBDIRS=parse eca local rewrite debug planner p2dl

lib_LTLIBRARIES = libp2lang.la
libp2lang_la_SOURCES = \
			compileContext.C \
			langLoader.C



INCLUDES = @BOOST_INC@ \
		-I../p2cpp \
		-I./parse \
		-I./eca \
		-I./planner \
		-I./p2dl \
		-I./debug \
		-I./local \
		-I./rewrite \
		-I../p2core \
		-I../elements \
		-I../net \
		-I../overlog \
		-I../debugger

CLEANFILES = core *.core *~

EXTRA_DIST = $(libp2lang_la_SOURCES:.C=.h) \
             compileContext.h

MAINTAINERCLEANFILES = Makefile.in

CXXFLAGS= -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG  @CXXFLAGS@

LDFLAGS=@BOOST_LIB@ @LDFLAGS@
