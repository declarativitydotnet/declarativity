# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#


# Libraries

lib_LTLIBRARIES = libp2unitTests.la

SOURCE_FILE_ROOTS =		\
				testTable2 \
				testRefTable \
				testBasicElementPlumbing \
				testValues \
				testIDs \
				testFdbufs \
				testCsv \
				testMarshal \
				testPel \
				testLists \
				testAggwrap

libp2unitTests_la_SOURCES =	$(SOURCE_FILE_ROOTS:=.h) $(SOURCE_FILE_ROOTS:=.C)

libp2unitTests_la_CXXFLAGS =	-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

libp2unitTests_la_CPPFLAGS =	@BOOST_INC@ \
				-I$(top_builddir)/p2core \
				-I$(top_builddir)/elements \
				-I$(top_builddir)/net \
				-I$(top_builddir)/overlog \
				-I$(top_builddir)/planner \
				-I$(top_builddir)/eventLoop



# Programs

bin_PROGRAMS =			p2TestTable2 \
				p2Test \
				p2TestAggwrap

p2Test_SOURCES =		p2Test.C

p2TestTable2_SOURCES =		p2TestTable2.C

p2TestAggwrap_SOURCES =		p2TestAggwrap.C

CLEANFILES = core *.core *~

AM_LDFLAGS =			 \
				-lboost_unit_test_framework \
				-lcrypto \
				@BOOST_LIB@

AM_CXXFLAGS =			-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

AM_CPPFLAGS =			-I$(top_builddir)/p2core

p2Test_DEPENDENCIES =		$(lib_LTLIBRARIES)

p2TestTable2_DEPENDENCIES =	$(lib_LTLIBRARIES)

p2TestAggwrap_DEPENDENCIES =	$(lib_LTLIBRARIES)

UNITLIBRARYDEPS =		\
				$(top_builddir)/unitTests/libp2unitTests.la \
				$(top_builddir)/overlog/libp2overlog.la \
				$(top_builddir)/net/libp2net.la \
				$(top_builddir)/elements/libp2elements.la \
				$(top_builddir)/debugger/libp2debugger.la \
				$(top_builddir)/p2core/libp2core.la \
				$(top_builddir)/eventLoop/libp2eventLoop.la

LDADD =				$(UNITLIBRARYDEPS) \
				$(lib_LTLIBRARIES)

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
