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
# DESCRIPTION: Makefile for the P2 tutorial C++ programs
#

bin_PROGRAMS =			ringControl \
				ponger


ringControl_SOURCES =		ringControl.C

ringControl_LDADD = 		\
				@OPENSSL_LIB@ \
				$(top_builddir)/p2cpp/libp2cpp.la \
				$(top_builddir)/planner/libp2planner.la \
				$(top_builddir)/overlog/libp2overlog.la \
				$(top_builddir)/net/libp2net.la \
				$(top_builddir)/stages/libp2stages.la \
				$(top_builddir)/functions/libp2functions.la \
				$(top_builddir)/aggregates/libp2aggregates.la \
				$(top_builddir)/elements/libp2elements.la \
				$(top_builddir)/p2core/libp2core.la \
				$(top_builddir)/debugger/libp2debugger.la



ponger_SOURCES =		ponger.C

ponger_LDADD =	 		\
				@OPENSSL_LIB@ \
				$(top_builddir)/p2cpp/libp2cpp.la \
				$(top_builddir)/planner/libp2planner.la \
				$(top_builddir)/overlog/libp2overlog.la \
				$(top_builddir)/net/libp2net.la \
				$(top_builddir)/stages/libp2stages.la \
				$(top_builddir)/functions/libp2functions.la \
				$(top_builddir)/aggregates/libp2aggregates.la \
				$(top_builddir)/elements/libp2elements.la \
				$(top_builddir)/p2core/libp2core.la \
				$(top_builddir)/debugger/libp2debugger.la



CLEANFILES = core *.core *~ .\#*


AM_CPPFLAGS =			@BOOST_INC@ \
				-I$(top_builddir)/p2core \
				-I$(top_builddir)/aggregates \
				-I$(top_builddir)/stages \
				-I$(top_builddir)/functions \
				-I$(top_builddir)/elements \
				-I$(top_builddir)/pythonElements \
				-I$(top_builddir)/net \
				-I$(top_builddir)/overlog \
				-I$(top_builddir)/planner \
				-I$(top_builddir)/lang \
				-I$(top_builddir)/lang/parse \
				-I$(top_builddir)/lang/eca \
				-I$(top_builddir)/lang/local \
				-I$(top_builddir)/lang/rewrite \
				-I$(top_builddir)/lang/debug \
				-I$(top_builddir)/lang/planner \
				-I$(top_builddir)/lang/p2dl \
				-I$(top_builddir)/debugger \
				-I$(top_builddir)/p2cpp \
				-I$(top_builddir)/lang 

AM_CXXFLAGS =			-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG


MAINTAINERCLEANFILES =		aclocal.m4 install-sh mkinstalldirs \
				missing configure config.guess config.sub config.h.in \
				ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in


