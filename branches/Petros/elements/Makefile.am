########################################################################
#
#  @(#)$Id$
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#  
#
# DESCRIPTION: Makefile for the P2 core elements library
#

lib_LTLIBRARIES =		libp2elements.la

SOURCE_FILE_ROOTS = 		elementRegistry \
				elementLoader \
				aggwrap2 \
				aggwrap2Callback \
				slot \
				print \
				printTime \
				discard \
				logger \
				pelTransform \
				marshal \
				marshalField \
				hexdump \
				unmarshal \
				unmarshalField \
				demuxConservative \
				ddemux \
				demux \
				mux \
				roundRobin \
				dRoundRobin \
				duplicate \
				duplicateConservative \
				dDuplicateConservative \
				dupElim \
				insert \
				lookup2 \
				delete \
				timedPullPush \
				strToSockaddr \
				queue \
				tupleSource \
				staticTupleSource \
				tupleInjector \
				noNull \
				noNullSignal \
				noNullField \
				unboxField \
				printWatch \
				aggregate \
				update \
				refresh \
				tupleListener \
				compileStage \
				stage \
				stageRegistry \
				compileTerminal \
				identity \
				removed \
				pullPush \
				switch \
				rangeLookup \
				commitBuf \
				insert2 \
				delete2

libp2elements_la_SOURCES =	$(SOURCE_FILE_ROOTS:=.h) $(SOURCE_FILE_ROOTS:=.C)

libp2elements_la_CPPFLAGS =	@BOOST_INC@ \
				-I../p2core

libp2elements_la_CXXFLAGS =	-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

CLEANFILES = core *.core *~

MAINTAINERCLEANFILES = Makefile.in


