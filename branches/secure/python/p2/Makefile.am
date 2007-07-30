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

SUBDIRS =			debugger \
				eventLoop \
				p2core \
				elements \
				pythonElements \
				net \
				overlog \
				p2cpp







lib_LTLIBRARIES =		libp2python.la

libp2python_la_SOURCES =	p2python.cpp

libp2python_la_LIBADD =		\
				-lcrypto \
				-lboost_python \
				@PYTHON_LSPEC@ \
				$(top_builddir)/python/p2/debugger/libp2pythondebugger.la \
				$(top_builddir)/python/p2/elements/libp2pythonelements.la \
				$(top_builddir)/python/p2/pythonElements/libp2pythonPythonelements.la \
				$(top_builddir)/python/p2/p2core/libp2pythoncore.la \
				$(top_builddir)/python/p2/net/libp2pythonnet.la \
				$(top_builddir)/python/p2/eventLoop/libp2pythoneventloop.la \
				$(top_builddir)/python/p2/overlog/libp2pythonoverlog.la \
				$(top_builddir)/python/p2/p2cpp/libp2pythoncpp.la \
				$(top_builddir)/p2core/libp2core.la \
				$(top_builddir)/debugger/libp2debugger.la \
				$(top_builddir)/elements/libp2elements.la \
				$(top_builddir)/pythonElements/libp2PythonElements.la \
				$(top_builddir)/eventLoop/libp2eventLoop.la \
				$(top_builddir)/net/libp2net.la \
				$(top_builddir)/overlog/libp2overlog.la \
				$(top_builddir)/p2cpp/libp2cpp.la

libp2python_la_CPPFLAGS =	@BOOST_INC@ \
				@PYTHON_CSPEC@

CLEANFILES =			core *.core *~

EXTRA_DIST =			README

MAINTAINERCLEANFILES =		Makefile.in


