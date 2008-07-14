# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#


noinst_LTLIBRARIES = libp2unitTests.la

libp2unitTests_la_SOURCES = \
	testTable2.C \
	testRefTable.C \
	testBasicElementPlumbing.C \
	testValues.C \
	testIDs.C \
	testFdbufs.C \
	testCsv.C \
	testMarshal.C \
	testPel.C \
	testLists.C


$(noinst_PROGRAMS): $(LDEPS) $(noinst_LTLIBRARIES)

noinst_PROGRAMS = p2Test p2TestTable2

p2Test_SOURCES =	p2Test.C

p2Test_LDADD = \
	$(top_builddir)/unitTests/libp2unitTests.la \
	$(top_builddir)/overlog/libp2overlog.la \
	$(top_builddir)/net/libp2net.la \
	$(top_builddir)/elements/libp2elements.la \
	$(top_builddir)/p2core/libp2core.la \
	$(top_builddir)/eventLoop/libp2eventLoop.la \
	-lboost_unit_test_framework

p2TestTable2_SOURCES =	p2TestTable2.C

p2TestTable2_LDADD = $(p2Test_LDADD)

$(p2Test_LDADD) : $(noinst_LTLIBRARIES)

CLEANFILES = core *.core *~

EXTRA_DIST = .cvsignore $(p2Test_SOURCES) \
		$(libp2unitTests_la_SOURCES:.C=.h)


INCLUDES =	@BOOST_INC@ \
		-I$(top_builddir)/p2core \
		-I$(top_builddir)/elements \
		-I$(top_builddir)/net \
		-I$(top_builddir)/overlog \
		-I$(top_builddir)/planner \
		-I$(top_builddir)/eventLoop

CXXFLAGS=-Wall -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG @CXXFLAGS@


LDFLAGS=@BOOST_LIB@ @LDFLAGS@ -static

LIBS = @BOOST_LIB@ -lcrypto

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
