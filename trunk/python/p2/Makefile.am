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

SUBDIRS=debugger eventLoop p2core elements net overlog

lib_LTLIBRARIES = libp2python.la
libp2python_la_SOURCES = p2python.cpp
libp2python_la_LIBADD = \
	$(top_builddir)/debugger/.libs/libdebugger.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a \
	$(top_builddir)/overlog/.libs/liboverlog.a \
	./debugger/.libs/libp2pythondebugger.a \
	./elements/.libs/libp2pythonelements.a \
	./p2core/.libs/libp2pythoncore.a \
	./net/.libs/libp2pythonnet.a \
	./eventLoop/.libs/libp2pythoneventloop.a \
	./overlog/.libs/libp2pythonoverlog.a \
       -lboost_python -lp2core -ldebugger -lp2elements -leventLoop -lp2net -loverlog

INCLUDES = $(PYTHON_CPPFLAGS) $(BOOST_INC)

LIBS = $(BOOST_LIB) $(PYTHON_LDFLAGS) \
       -L$(top_builddir)/elements -L$(top_builddir)/p2core \
       -L$(top_builddir)/eventLoop -L$(top_builddir)/net \
       -L$(top_builddir)/overlog -L$(top_builddir)/debugger \
       -L./elements -L./p2core -L./eventLoop -L./net -L./overlog -L./debugger

CLEANFILES = core *.core *~

EXTRA_DIST = .cvsignore README

MAINTAINERCLEANFILES = Makefile.in


