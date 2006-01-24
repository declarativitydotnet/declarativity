bin_PROGRAMS = p2Test

p2Test_SOURCES =	p2Test.C \
			testMarshal.C \
			testBasicElementPlumbing.C \
			testFdbufs.C

p2Test_LDADD =		-lboost_unit_test_framework

p2Test_LDFLAGS = \
	$(top_builddir)/overlog/.libs/liboverlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a

CLEANFILES = core *.core *~

EXTRA_DIST = .cvsignore

INCLUDES =	@BOOST_INC@ \
		-I$(top_builddir)/p2core \
		-I$(top_builddir)/elements \
		-I$(top_builddir)/net \
		-I$(top_builddir)/overlog \
		-I$(top_builddir)/planner \
		-I$(top_builddir)/eventLoop

LIBS = @BOOST_LIB@ -lrt

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
