#########################################################################
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
# DESCRIPTION: Makefile
#

$(PROGRAMS): $(LDEPS)

bin_PROGRAMS 	= phid
noinst_HEADERS	= phid.h
phid_SOURCES	= phid.C
phid_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/elements/.libs/libp2elements.a

LIBS = @BOOST_LIB@

CXXFLAGS= -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG @CXXFLAGS@
LDFLAGS=@BOOST_LIB@ @LDFLAGS@

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore

INCLUDES = @BOOST_INC@ -I$(top_builddir)/p2core \
	-I$(top_builddir)/elements \
	-I$(top_builddir)/net

