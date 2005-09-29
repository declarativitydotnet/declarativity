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
	ddemux.C \
	demux.C \
	mux.C \
	roundRobin.C \
	duplicate.C \
	duplicateConservative.C \
	dupElim.C \
	filter.C \
	store.C \
	insert.C \
	delete.C \
	joiner.C \
	timedPullPush.C \
	strToSockaddr.C \
	queue.C \
	groupby.C \
	randomPushSource.C \
	ping.C \
	pong.C \
	pelScan.C \
	functorSource.C \
	tupleSource.C \
	noNull.C \
	noNullField.C\
	unboxField.C \
	printWatch.C \
	aggregate.C \
	scan.C


INCLUDES = @SFS_INC@ -I../p2core

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore

MAINTAINERCLEANFILES = Makefile.in

