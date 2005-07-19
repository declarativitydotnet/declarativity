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
# DESCRIPTION: Makefile for the P2 net library
#

lib_LTLIBRARIES = libp2net.la
libp2net_la_SOURCES = \
	udpsuio.C \
	snetsim.C \
	plsensor.C \
	udp.C \
	tupleseq.C \
	cc.C \
	cct.C \
	ccr.C \
	rcct.C \
	rccr.C \
	rdelivery.C \
	bw.C \
	frag.C \
	defrag.C

INCLUDES = @SFS_INC@ -I../p2core

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore

MAINTAINERCLEANFILES = Makefile.in

