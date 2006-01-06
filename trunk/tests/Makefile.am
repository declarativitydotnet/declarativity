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
# DESCRIPTION: Makefile for the P2 tests
#

$(PROGRAMS): $(LDEPS)

bin_PROGRAMS 	= 	values \
			tuples \
			benchmark \
			fdbufs \
			overlog \
			tables \
			pel \
			IDs \
			runManyChords \
			runChord \
			runOverLog \
			runSymphony \
			runSymphonicChord \
			simple_lookup \
			lookupGenerator \
			basicelements \
			basicElementPlumbing \
			scheduling \
			demultiplex \
			join \
			net \
			netcc \
			netrcc \
			netcc2 \
			perfectChord \
			logger \
			csv \
			testQueue \
			agg \
			pingpong \
			paths \
			testPlanner


values_SOURCES	= values.C
values_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
		$(top_builddir)/eventLoop/.libs/libeventLoop.a

tuples_SOURCES	= tuples.C
tuples_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
		$(top_builddir)/eventLoop/.libs/libeventLoop.a

benchmark_SOURCES	= benchmark.C
benchmark_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
		  $(top_builddir)/eventLoop/.libs/libeventLoop.a

fdbufs_SOURCES	= fdbufs.C
fdbufs_LDADD = $(top_builddir)/p2core/.libs/libp2core.a

tables_SOURCES	= tables.C
tables_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
		$(top_builddir)/eventLoop/.libs/libeventLoop.a

pel_SOURCES	= pel.C
pel_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
		$(top_builddir)/eventLoop/.libs/libeventLoop.a


logger_SOURCES  = logger.C
logger_LDADD = $(top_builddir)/elements/.libs/libp2elements.a \
		$(top_builddir)/p2core/.libs/libp2core.a \
		$(top_builddir)/eventLoop/.libs/libeventLoop.a


basicElementPlumbing_SOURCES = basicElementPlumbing.C
basicElementPlumbing_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

scheduling_SOURCES = scheduling.C
scheduling_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

demultiplex_SOURCES = demultiplex.C
demultiplex_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

join_SOURCES = join.C
join_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

paths_SOURCES = paths.C
paths_LDADD = \
	$(top_builddir)/planner/.libs/libp2planner.a \
	$(top_builddir)/overlog/.libs/liboverlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

runOverLog_SOURCES = runOverLog.C
runOverLog_LDADD = \
	$(top_builddir)/overlog/.libs/liboverlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

runSymphony_SOURCES = runSymphony.C
runSymphony_LDADD = \
	$(top_builddir)/overlog/.libs/liboverlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

runSymphonicChord_SOURCES = runSymphonicChord.C
runSymphonicChord_LDADD = \
	$(top_builddir)/overlog/.libs/liboverlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

runChord_SOURCES = runChord.C
runChord_LDADD = \
	$(top_builddir)/overlog/.libs/liboverlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

runManyChords_SOURCES = runManyChords.C
runManyChords_LDADD = \
	$(top_builddir)/overlog/.libs/liboverlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

overlog_SOURCES	= overlog.C
overlog_LDADD = \
	$(top_builddir)/overlog/.libs/liboverlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a 

simple_lookup_SOURCES = simple_lookup.C
simple_lookup_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a 

lookupGenerator_SOURCES = lookupGenerator.C
lookupGenerator_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a 

#chordDatalog_SOURCES = chordDatalog.C
#chordDatalog_LDADD = \
#	$(top_builddir)/overlog/.libs/liboverlog.a \
#	$(top_builddir)/net/.libs/libp2net.a \
#	$(top_builddir)/elements/.libs/libp2elements.a \
#	$(top_builddir)/p2core/.libs/libp2core.a 

IDs_SOURCES = IDs.C
IDs_LDADD = \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a \
	$(top_builddir)/p2core/.libs/libp2core.a

net_SOURCES = net.C
net_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

netcc_SOURCES = netcc.C
netcc_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a \
	$(top_builddir)/p2core/.libs/libp2core.a

netrcc_SOURCES = netrcc.C
netrcc_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a \
	$(top_builddir)/p2core/.libs/libp2core.a

netcc2_SOURCES = netcc2.C
netcc2_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a

perfectChord_SOURCES = perfectChord.C
perfectChord_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a

basicelements_SOURCES	= basicelements.C
basicelements_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

testQueue_SOURCES	= testQueue.C
testQueue_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

agg_SOURCES	= agg.C
agg_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a \
	$(top_builddir)/p2core/.libs/libp2core.a

csv_SOURCES	= csv.C
csv_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

pingpong_SOURCES	= pingpong.C
pingpong_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

testPlanner_SOURCES	= testPlanner.C
testPlanner_LDADD = \
	$(top_builddir)/planner/.libs/libp2planner.a \
	$(top_builddir)/overlog/.libs/liboverlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore
INCLUDES = -I$(top_builddir)/p2core -I$(top_builddir)/elements \
		-I$(top_builddir)/net -I$(top_builddir)/overlog \
		-I$(top_builddir)/planner -I$(top_builddir)/eventLoop

LIBS = -lboost_regex -lrt

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
