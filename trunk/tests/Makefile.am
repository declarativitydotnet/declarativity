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
			logger \
			testQueue \
			udpMessage \
			udpListen

tuples_SOURCES	= tuples.C
tuples_LDADD = $(top_builddir)/p2core/libp2core.la \
		$(top_builddir)/eventLoop/libp2eventLoop.la

benchmark_SOURCES	= benchmark.C
benchmark_LDADD = $(top_builddir)/p2core/libp2core.la \
		  $(top_builddir)/eventLoop/libp2eventLoop.la


logger_SOURCES  = logger.C
logger_LDADD = -lboost_python \
		$(top_builddir)/elements/libp2elements.la \
		$(top_builddir)/p2core/libp2core.la \
		$(top_builddir)/eventLoop/libp2eventLoop.la


scheduling_SOURCES = scheduling.C
scheduling_LDADD = -lboost_python \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la

demultiplex_SOURCES = demultiplex.C
demultiplex_LDADD = -lboost_python \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la

runOverLog_SOURCES = runOverLog.C
runOverLog_LDADD = -lboost_python \
	$(top_builddir)/planner/libp2planner.la \
	$(top_builddir)/overlog/libp2overlog.la \
	$(top_builddir)/net/libp2net.la \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la \
	$(top_builddir)/debugger/libp2debugger.la

runChord_SOURCES = runChord.C
runChord_LDADD = -lboost_python \
	$(top_builddir)/planner/libp2planner.la \
	$(top_builddir)/debugger/libp2debugger.la \
	$(top_builddir)/overlog/libp2overlog.la \
	$(top_builddir)/net/libp2net.la \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la

runOverLog2_SOURCES = runOverLog2.C
runOverLog2_LDADD = -lboost_python \
	$(top_builddir)/debugger/libp2debugger.la \
	$(top_builddir)/p2cpp/libp2cpp.la \
	$(top_builddir)/overlog/libp2overlog.la \
	$(top_builddir)/net/libp2net.la \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la


runPingPong_SOURCES = runPingPong.C
runPingPong_LDADD = -lboost_python \
	$(top_builddir)/debugger/libp2debugger.la \
	$(top_builddir)/p2cpp/libp2cpp.la \
	$(top_builddir)/overlog/libp2overlog.la \
	$(top_builddir)/net/libp2net.la \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la 


overlog_SOURCES	= overlog.C
overlog_LDADD = -lboost_python \
	$(top_builddir)/planner/libp2planner.la \
	$(top_builddir)/overlog/libp2overlog.la \
	$(top_builddir)/net/libp2net.la \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la \
	$(top_builddir)/debugger/libp2debugger.la

simple_lookup_SOURCES = simple_lookup.C
simple_lookup_LDADD = -lboost_python \
	$(top_builddir)/net/libp2net.la \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la 

lookupGenerator_SOURCES = lookupGenerator.C
lookupGenerator_LDADD = -lboost_python \
	$(top_builddir)/net/libp2net.la \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la 

basicelements_SOURCES	= basicelements.C
basicelements_LDADD = -lboost_python \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la

testQueue_SOURCES	= testQueue.C
testQueue_LDADD = -lboost_python \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la

udpMessage_SOURCES	= udpMessage.C
udpMessage_LDADD = -lboost_python \
	$(top_builddir)/net/libp2net.la \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la

udpListen_SOURCES	= udpListen.C
udpListen_LDADD = -lboost_python \
	$(top_builddir)/net/libp2net.la \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la

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
       -L$(top_builddir)/p2cpp -lcrypto

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
