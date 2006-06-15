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
# DESCRIPTION: Makefile for the P2 core elements python library
#

noinst_LTLIBRARIES = libp2pythonelements.la

libp2pythonelements_la_SOURCES = \
	aggregate.cpp \
	aggwrap.cpp \
	csvparser.cpp \
	dDuplicateConservative.cpp \
	dRoundRobin.cpp \
	dataflowInstaller.cpp \
	ddemux.cpp \
	delete.cpp \
	demux.cpp \
	discard.cpp \
	dupElim.cpp \
	duplicate.cpp \
	duplicateConservative.cpp \
	filter.cpp \
	functorSource.cpp \
	hexdump.cpp \
	insert.cpp \
	logger.cpp \
	lookup2.cpp \
	marshal.cpp \
	marshalField.cpp \
	mux.cpp \
	noNull.cpp \
	noNullField.cpp \
	pelTransform.cpp \
	print.cpp \
	printTime.cpp \
	printWatch.cpp \
	queue.cpp \
	roundRobin.cpp \
	slot.cpp \
	strToSockaddr.cpp \
	timedPullPush.cpp \
	timedPullSink.cpp \
	timedPushSource.cpp \
	timestampSource.cpp \
	tupleSource.cpp \
	unboxField.cpp \
	unmarshal.cpp \
	unmarshalField.cpp \
	tupleListener.cpp \
	tupleSourceInterface.cpp \
	update.cpp

INCLUDES = @BOOST_INC@ -I$(top_builddir)/p2core -I$(top_builddir)/elements -I$(top_builddir)/eventLoop

CLEANFILES = core *.core *~

MAINTAINERCLEANFILES = Makefile.in

CXXFLAGS= -static -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG  @CXXFLAGS@ ${PYTHON_CPPFLAGS}
