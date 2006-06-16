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

noinst_PROGRAMS 	=  	tuples \
			benchmark \
			overlog \
			runChord \
			runOverLog \
			runOverLog2 \
			runPingPong \
			simple_lookup \
			lookupGenerator \
			basicelements \
			scheduling \
			demultiplex \
			net \
			netrcc \
			netcc2 \
			perfectChord \
			logger \
			testQueue \
			udpMessage \
			udpListen


tuples_SOURCES	= tuples.C
tuples_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
		$(top_builddir)/eventLoop/.libs/libp2eventLoop.a

benchmark_SOURCES	= benchmark.C
benchmark_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
		  $(top_builddir)/eventLoop/.libs/libp2eventLoop.a


logger_SOURCES  = logger.C
logger_LDADD = $(top_builddir)/elements/.libs/libp2elements.a \
		$(top_builddir)/p2core/.libs/libp2core.a \
		$(top_builddir)/eventLoop/.libs/libp2eventLoop.a


scheduling_SOURCES = scheduling.C
scheduling_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a

demultiplex_SOURCES = demultiplex.C
demultiplex_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a

runOverLog_SOURCES = runOverLog.C
runOverLog_LDADD = \
	$(top_builddir)/overlog/.libs/libp2overlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a \
	$(top_builddir)/debugger/.libs/libp2debugger.a

runChord_SOURCES = runChord.C
runChord_LDADD = \
	$(top_builddir)/overlog/.libs/libp2overlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a \
	$(top_builddir)/debugger/.libs/libp2debugger.a

runOverLog2_SOURCES = runOverLog2.C
runOverLog2_LDADD = \
	-lboost_python -lp2core -lp2debugger -lp2elements \
	-lp2eventLoop -lp2net -lp2overlog -lp2cpp \
	$(top_builddir)/overlog/.libs/libp2overlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a \
	$(top_builddir)/p2cpp/.libs/libp2cpp.a


runPingPong_SOURCES = runPingPong.C
runPingPong_LDADD = \
	-lboost_python -lp2core -lp2debugger -lp2elements \
	-lp2eventLoop -lp2net -lp2overlog -lp2cpp \
	$(top_builddir)/overlog/.libs/libp2overlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a \
	$(top_builddir)/p2cpp/.libs/libp2cpp.a


overlog_SOURCES	= overlog.C
overlog_LDADD = \
	$(top_builddir)/overlog/.libs/libp2overlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a \
	$(top_builddir)/debugger/.libs/libp2debugger.a

simple_lookup_SOURCES = simple_lookup.C
simple_lookup_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a 

lookupGenerator_SOURCES = lookupGenerator.C
lookupGenerator_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a 

net_SOURCES = net.C
net_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a

netrcc_SOURCES = netrcc.C
netrcc_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a \
	$(top_builddir)/p2core/.libs/libp2core.a

netcc2_SOURCES = netcc2.C
netcc2_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a

perfectChord_SOURCES = perfectChord.C
perfectChord_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a

basicelements_SOURCES	= basicelements.C
basicelements_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a

testQueue_SOURCES	= testQueue.C
testQueue_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a

udpMessage_SOURCES	= udpMessage.C
udpMessage_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a

udpListen_SOURCES	= udpListen.C
udpListen_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libp2eventLoop.a

CLEANFILES = core *.core *~

EXTRA_DIST = .cvsignore \
		testerr.h \
		chordDatalog.C \
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




INCLUDES = @BOOST_INC@ -I$(top_builddir)/p2core -I$(top_builddir)/elements \
		-I$(top_builddir)/net -I$(top_builddir)/overlog \
		-I$(top_builddir)/planner -I$(top_builddir)/eventLoop \
		-I$(top_builddir)/debugger -I$(top_builddir)/p2cpp

CXXFLAGS= -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG  @CXXFLAGS@ ${PYTHON_CPPFLAGS}

LDFLAGS=@BOOST_LIB@ @LDFLAGS@ @PYTHON_LDFLAGS@

LIBS = @BOOST_LIB@ \
       -L$(top_builddir)/elements -L$(top_builddir)/p2core \
       -L$(top_builddir)/eventLoop -L$(top_builddir)/net \
       -L$(top_builddir)/overlog -L$(top_builddir)/debugger \
       -L$(top_builddir)/p2cpp

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
