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
	pushprint.C \
	pullprint.C \
	memoryPull.C \
	print.C \
	timedSource.C \
	logger.C \
	discard.C \
	pelTransform.C \
	marshal.C \
	hexdump.C \
	csvparser.C \
	unmarshal.C

INCLUDES = @SFS_INC@ -I../p2core

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore

MAINTAINERCLEANFILES = Makefile.in

