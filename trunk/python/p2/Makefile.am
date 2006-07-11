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

SUBDIRS=debugger eventLoop p2core elements net overlog p2cpp

lib_LTLIBRARIES = libp2python.la

libp2python_la_SOURCES = p2python.cpp

libp2python_la_LIBADD = \
#    -lboost_python \
	-lp2pythondebugger \
	-lp2pythonelements \
	-lp2pythoncore \
	-lp2pythonnet \
	-lp2pythoneventloop \
	-lp2pythonoverlog \
	-lp2pythoncpp \
       -lp2core -lp2debugger -lp2elements -lp2eventLoop -lp2net -lp2overlog -lp2cpp

INCLUDES = $(PYTHON_CPPFLAGS) $(BOOST_INC)

LIBS = $(BOOST_LIB) $(PYTHON_LDFLAGS) \
       -L$(top_builddir)/elements -L$(top_builddir)/p2core \
       -L$(top_builddir)/eventLoop -L$(top_builddir)/net \
       -L$(top_builddir)/overlog -L$(top_builddir)/debugger -L$(top_builddir)/p2cpp \
       -L./elements -L./p2core -L./eventLoop -L./net -L./overlog -L./debugger -L./p2cpp

CLEANFILES = core *.core *~

EXTRA_DIST = .cvsignore README \
		$(libp2python_la_SOURCES)

MAINTAINERCLEANFILES = Makefile.in
