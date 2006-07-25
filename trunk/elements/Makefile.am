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
#
# DESCRIPTION: Makefile for the P2 core elements library
#

lib_LTLIBRARIES = libp2elements.la

libp2elements_la_SOURCES = \
	aggwrap.C \
	slot.C \
	print.C \
	printTime.C \
	timedPushSource.C \
	timestampSource.C \
	timedPullSink.C \
	discard.C \
	logger.C \
	pelTransform.C \
	marshal.C \
	marshalField.C \
	hexdump.C \
	csvparser.C \
	unmarshal.C \
	unmarshalField.C \
	route.C \
	demux.C \
	ddemux.C \
	mux.C \
	roundRobin.C \
	dRoundRobin.C \
	duplicate.C \
	duplicateConservative.C \
	dDuplicateConservative.C \
	dupElim.C \
	filter.C \
	insert.C \
	lookup2.C \
	delete.C \
	timedPullPush.C \
	strToSockaddr.C \
	queue.C \
	functorSource.C \
	tupleSource.C \
	noNull.C \
	noNullField.C\
	unboxField.C \
	printWatch.C \
	aggregate.C \
	update.C \
	tupleListener.C \
	tupleSourceInterface.C \
	removed.C \
	dataflowInstaller.C

libp2elements_la_LIBADD = -lboost_python

INCLUDES = @BOOST_INC@ -I../p2core -I../eventLoop -I../overlog -I../net

CLEANFILES = core *.core *~

EXTRA_DIST = .cvsignore \
		$(libp2elements_la_SOURCES:.C=.h)

MAINTAINERCLEANFILES = Makefile.in

CXXFLAGS=-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG @CXXFLAGS@ ${PYTHON_CPPFLAGS}

