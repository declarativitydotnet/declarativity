$(PROGRAMS): $(LDEPS)

bin_PROGRAMS 	= tuples pel basicelements basicElementPlumbing 

tuples_SOURCES	= tuples.C
tuples_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

pel_SOURCES	= pel.C
pel_LDADD = $(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

basicElementPlumbing_SOURCES = basicElementPlumbing.C
basicElementPlumbing_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

basicelements_SOURCES	= basicelements.C
basicelements_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	@SFS_LIBASYNC@ @SFS_LIBARPC@

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore
INCLUDES = @SFS_INC@ -I$(top_builddir)/p2core -I$(top_builddir)/elements

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
