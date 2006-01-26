########################################################################
#
#  @(#)$Id$
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
#
# DESCRIPTION: Makefile for the P2 net library
#

lib_LTLIBRARIES = libp2net.la
libp2net_la_SOURCES = \
	snetsim.C \
	skr.C \
	tman.C \
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

INCLUDES = @BOOST_INC@ -I../p2core -I../eventLoop

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore

MAINTAINERCLEANFILES = Makefile.in

