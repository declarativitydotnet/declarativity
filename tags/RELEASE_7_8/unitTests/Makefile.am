# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#

SUBDIRS =			olg

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

bin_PROGRAMS =			p2Test

p2Test_SOURCES =		p2Test.C \
				$(SOURCE_FILE_ROOTS:=.h) $(SOURCE_FILE_ROOTS:=.C)

AM_CPPFLAGS =			@BOOST_INC@ \
				-I$(top_builddir)/p2core \
				-I$(top_builddir)/elements \
				-I$(top_builddir)/net \
				-I$(top_builddir)/overlog \
				-I$(top_builddir)/planner \
				-I$(top_builddir)/eventLoop

CLEANFILES =			core *.core *~

AM_LDFLAGS =			\
				-lboost_unit_test_framework \
				-lcrypto \
				@BOOST_LIB@

AM_CXXFLAGS =			-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

LDADD =		\
				$(top_builddir)/overlog/libp2overlog.la \
				$(top_builddir)/net/libp2net.la \
				$(top_builddir)/elements/libp2elements.la \
				$(top_builddir)/debugger/libp2debugger.la \
				$(top_builddir)/p2core/libp2core.la \
				$(top_builddir)/eventLoop/libp2eventLoop.la

