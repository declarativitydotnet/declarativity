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
	slot.C \
	print.C \
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
	mux.C \
	duplicate.C \
	dupElim.C \
	filter.C \
	store.C \
	joiner.C \
	timedPullPush.C \
	strToSockaddr.C \
	queue.C \
	groupby.C \
	randomPushSource.C

INCLUDES = @SFS_INC@ -I../p2core

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore

MAINTAINERCLEANFILES = Makefile.in

