$(PROGRAMS): $(LDEPS)

bin_PROGRAMS 	= tuples basicelements basicElementPlumbing
# noinst_HEADERS	= 
tuples_SOURCES	= tuples.C
tuples_LDADD = $(top_builddir)/p2core/.libs/libp2core.a $(LDADD)

basicElementPlumbing_SOURCES = basicElementPlumbing.C
basicElementPlumbing_LDADD =	$(top_builddir)/p2core/.libs/libp2core.a \
				$(top_builddir)/elements/.libs/libp2elements.a \
				$(LDADD)

basicelements_SOURCES	= basicelements.C
basicelements_LDADD = \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(LDADD)

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore
INCLUDES = -I$(top_builddir)/p2core -I$(top_builddir)/elements

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
