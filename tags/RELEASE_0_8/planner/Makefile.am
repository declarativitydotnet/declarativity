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
# DESCRIPTION: Makefile for the P2 planner (SIGMOD 2006 version)
#


lib_LTLIBRARIES =		libp2planner.la

SOURCE_FILE_ROOTS =		tableStore \
				planContext \
				localize_context \
				eca_context \
				ruleStrand \
				netPlanner \
				planner \
				stageStrand 


libp2planner_la_SOURCES =	\
				$(SOURCE_FILE_ROOTS:=.h) \
				$(SOURCE_FILE_ROOTS:=.C)

libp2planner_la_CXXFLAGS =	-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

libp2planner_la_CPPFLAGS =	\
				@BOOST_INC@ \
				-I../p2core \
				-I../elements \
				-I../net \
				-I../overlog \
				-I../eventLoop \
				-I../debugger

CLEANFILES =			core *.core *~

EXTRA_DIST =			rulePlanner.C \
				rulePel.C

MAINTAINERCLEANFILES =		Makefile.in

