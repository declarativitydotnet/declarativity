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
#

lib_LTLIBRARIES = libp2python.la
libp2python_la_SOURCES = p2python.cpp
libp2python_la_LIBADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a \
       -lboost_python -lrt -lp2core -lp2elements -leventLoop

INCLUDES = $(PYTHON_CPPFLAGS) $(BOOST_INC) \
           -I$(top_builddir)/net -I$(top_builddir)/elements \
           -I$(top_builddir)/p2core -I$(top_builddir)/eventLoop

LIBS = $(BOOST_LIB) $(PYTHON_LDFLAGS) \
       -L$(top_builddir)/elements -L$(top_builddir)/p2core -L$(top_builddir)/eventLoop

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore

MAINTAINERCLEANFILES = Makefile.in
