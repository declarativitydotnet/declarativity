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
# DESCRIPTION: Makefile for the P2 tests
#

bin_PROGRAMS =			runOverLog \
				benchmark \
				udpMessage \
				udpListen \
				runStagedOverlog \
				p2nc


benchmark_SOURCES =		benchmark.C

benchmark_LDADD =		$(top_builddir)/p2core/libp2core.la \
				$(top_builddir)/lang/libp2lang.la \
				$(top_builddir)/lang/parse/libp2parse.la \
				$(top_builddir)/lang/eca/libp2eca.la \
				$(top_builddir)/lang/local/libp2local.la \
				$(top_builddir)/lang/rewrite/libp2rewrite.la \
				$(top_builddir)/lang/debug/libp2debug.la \
				$(top_builddir)/lang/planner/libp2planner.la \
				$(top_builddir)/lang/p2dl/libp2dl.la \
				$(top_builddir)/elements/libp2elements.la \
				$(top_builddir)/net/libp2net.la

runStagedOverlog_SOURCES  =	runStagedOverlog.C

runStagedOverlog_LDADD =	\
				@OPENSSL_LIB@ \
				$(top_builddir)/stages/libp2stages.la \
				$(top_builddir)/lang/p2dl/libp2dl.la \
				$(top_builddir)/elements/libp2elements.la \
				$(top_builddir)/p2core/libp2core.la \
				$(top_builddir)/lang/libp2lang.la \
				$(top_builddir)/lang/parse/libp2parse.la \
				$(top_builddir)/lang/eca/libp2eca.la \
				$(top_builddir)/lang/local/libp2local.la \
				$(top_builddir)/lang/rewrite/libp2rewrite.la \
				$(top_builddir)/lang/debug/libp2debug.la \
				$(top_builddir)/lang/planner/libp2planner.la \
				$(top_builddir)/net/libp2net.la \
				$(top_builddir)/functions/libp2functions.la \
				$(top_builddir)/aggregates/libp2aggregates.la \
				$(top_builddir)/debugger/libp2debugger.la 



runOverLog_SOURCES =		runOverLog.C

runOverLog_LDADD = 		\
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


p2nc_SOURCES =			p2nc.C

p2nc_LDADD =	 		\
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




udpMessage_SOURCES =		udpMessage.C

udpMessage_LDADD =		\
				@OPENSSL_LIB@ \
				$(top_builddir)/net/libp2net.la \
				$(top_builddir)/elements/libp2elements.la \
				$(top_builddir)/p2core/libp2core.la \
				$(top_builddir)/lang/libp2lang.la \
				$(top_builddir)/lang/parse/libp2parse.la \
				$(top_builddir)/lang/eca/libp2eca.la \
				$(top_builddir)/lang/local/libp2local.la \
				$(top_builddir)/lang/rewrite/libp2rewrite.la \
				$(top_builddir)/lang/debug/libp2debug.la \
				$(top_builddir)/lang/planner/libp2planner.la \
				$(top_builddir)/lang/p2dl/libp2dl.la


udpListen_SOURCES =		udpListen.C

udpListen_LDADD =		\
				@OPENSSL_LIB@ \
				$(top_builddir)/net/libp2net.la \
				$(top_builddir)/elements/libp2elements.la \
				$(top_builddir)/p2core/libp2core.la \
				$(top_builddir)/lang/libp2lang.la \
				$(top_builddir)/lang/parse/libp2parse.la \
				$(top_builddir)/lang/eca/libp2eca.la \
				$(top_builddir)/lang/local/libp2local.la \
				$(top_builddir)/lang/rewrite/libp2rewrite.la \
				$(top_builddir)/lang/debug/libp2debug.la \
				$(top_builddir)/lang/planner/libp2planner.la \
				$(top_builddir)/lang/p2dl/libp2dl.la


CLEANFILES = core *.core *~ .\#*


EXTRA_DIST = 			testerr.h \
				scripts/buildCDF.pl \
				scripts/plookups.py \
				scripts/pout.py \
				scripts/process_output.py \
				scripts/reset.sh \
				scripts/rnode.py \
				scripts/runLocalHostChords.pl \
				scripts/run_lookups.py \
				scripts/run_node.py \
				scripts/crunch/buildCDF.pl \
				scripts/crunch/consistency.pl \
				scripts/crunch/crunch.sh \
				scripts/crunch/hopDist.pl \
				scripts/crunch/messages.pl \
				scripts/crunch/stripResults.sh

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


