# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#

TEST_SOURCES = 	testBasicElementPlumbing.C \
		testFdbufs.C \
		testMarshal.C \
		testPel.C \
		testValues.C \
		testTables.C \
		testTable2.C \
		testIDs.C \
		testCsv.C \
		testLists.C


$(PROGRAMS): $(LDEPS) \
		$(TEST_SOURCES)

bin_PROGRAMS = p2Test

p2Test_SOURCES =	p2Test.C

p2Test_LDADD = \
	$(top_builddir)/overlog/.libs/liboverlog.a \
	$(top_builddir)/net/.libs/libp2net.a \
	$(top_builddir)/elements/.libs/libp2elements.a \
	$(top_builddir)/p2core/.libs/libp2core.a \
	$(top_builddir)/eventLoop/.libs/libeventLoop.a \
	-lboost_unit_test_framework


CLEANFILES = core *.core *~

EXTRA_DIST = .cvsignore $(p2Test_SOURCES) $(TEST_SOURCES)

INCLUDES =	@BOOST_INC@ \
		-I$(top_builddir)/p2core \
		-I$(top_builddir)/elements \
		-I$(top_builddir)/net \
		-I$(top_builddir)/overlog \
		-I$(top_builddir)/planner \
		-I$(top_builddir)/eventLoop

CXXFLAGS=-Wall -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG @CXXFLAGS@


LDFLAGS=@BOOST_LIB@ @LDFLAGS@

LIBS = @BOOST_LIB@

MAINTAINERCLEANFILES = aclocal.m4 install-sh mkinstalldirs \
	missing configure config.guess config.sub config.h.in \
	ltconfig ltmain.sh INSTALL COPYING Makefile.in stamp-h.in
