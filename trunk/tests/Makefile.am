$(PROGRAMS): $(LDEPS)

bin_PROGRAMS 	= tuples
# noinst_HEADERS	= 
tuples_SOURCES	= tuples.C
tuples_LDADD = $(top_builddir)/p2core/.libs/libp2core.a $(LDADD)

CLEANFILES = core *.core *~
EXTRA_DIST = .cvsignore
INCLUDES = -I$(top_builddir)/phid

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
