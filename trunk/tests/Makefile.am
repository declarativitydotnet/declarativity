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
# DESCRIPTION: Makefile for the P2 tests
#

$(PROGRAMS): $(LDEPS)

bin_PROGRAMS 	= 	values \
			tuples \
			pel \
			basicelements \
			basicElementPlumbing \
			scheduling \
			demultiplex \
			net \
			logger \
			csv

values_SOURCES	= values.C
values_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

tuples_SOURCES	= tuples.C
tuples_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

pel_SOURCES	= pel.C
pel_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

logger_SOURCES  = logger.C
logger_LDADD = $(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

basicElementPlumbing_SOURCES = basicElementPlumbing.C
basicElementPlumbing_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

scheduling_SOURCES = scheduling.C
scheduling_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

demultiplex_SOURCES = demultiplex.C
demultiplex_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

net_SOURCES = net.C
net_LDADD = \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

basicelements_SOURCES	= basicelements.C
basicelements_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

csv_SOURCES	= csv.C
csv_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@


CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore
INCLUDES = @SFS_INC@ -I$(top_builddir)/p2core -I$(top_builddir)/elements -I$(top_builddir)/net

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
