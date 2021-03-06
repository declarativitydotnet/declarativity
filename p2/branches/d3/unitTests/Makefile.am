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
				testValues \
				testIDs \
				testFdbufs \
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
				-I$(top_builddir)/stages \
				-I$(top_builddir)/aggregates \
				-I$(top_builddir)/net \
				-I$(top_builddir)/overlog \
				-I$(top_builddir)/planner

CLEANFILES =			core *.core *~

AM_LDFLAGS =			\
				-lboost_unit_test_framework \
				-lcrypto \
				@BOOST_LIB@

AM_CXXFLAGS =			-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

LDADD =		\
				$(top_builddir)/overlog/libp2overlog.la \
				$(top_builddir)/net/libp2net.la \
				$(top_builddir)/stages/libp2stages.la \
				$(top_builddir)/aggregates/libp2aggregates.la \
				$(top_builddir)/elements/libp2elements.la \
				$(top_builddir)/debugger/libp2debugger.la \
				$(top_builddir)/p2core/libp2core.la \
				$(top_builddir)/lang/libp2lang.la \
				$(top_builddir)/lang/parse/libp2parse.la \
				$(top_builddir)/lang/rewrite/libp2rewrite.la \
				$(top_builddir)/lang/debug/libp2debug.la \
				$(top_builddir)/lang/local/libp2local.la \
				$(top_builddir)/lang/eca/libp2eca.la \
				$(top_builddir)/lang/planner/libp2planner.la \
				$(top_builddir)/lang/p2dl/libp2dl.la

