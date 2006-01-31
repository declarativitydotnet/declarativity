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

$(PROGRAMS): $(LDEPS)

bin_PROGRAMS 	=  	tuples \
			benchmark \
			overlog \
			tables \
			IDs \
			runManyChords \
			runChord \
			runOverLog \
			runSymphony \
			runSymphonicChord \
			simple_lookup \
			lookupGenerator \
			basicelements \
			scheduling \
			demultiplex \
			join \
			net \
			netcc \
			netrcc \
			netcc2 \
			perfectChord \
			logger \
			testQueue \
			udpMessage \
			udpListen


tuples_SOURCES	= tuples.C
tuples_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
		$(top_builddir)/eventLoop/.libs/libeventLoop.a

benchmark_SOURCES	= benchmark.C
benchmark_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
		  $(top_builddir)/eventLoop/.libs/libeventLoop.a

tables_SOURCES	= tables.C
tables_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
		$(top_builddir)/eventLoop/.libs/libeventLoop.a


logger_SOURCES  = logger.C
logger_LDADD = $(top_builddir)/elements/.libs/libp2elements.a \
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

udpMessage_SOURCES	= udpMessage.C
udpMessage_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

udpListen_SOURCES	= udpListen.C
udpListen_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore
INCLUDES = @BOOST_INC@ -I$(top_builddir)/p2core -I$(top_builddir)/elements \
		-I$(top_builddir)/net -I$(top_builddir)/overlog \
		-I$(top_builddir)/planner -I$(top_builddir)/eventLoop

LIBS = @BOOST_LIB@ -lrt

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
