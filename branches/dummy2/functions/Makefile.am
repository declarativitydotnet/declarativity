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
# DESCRIPTION: Makefile for the P2 aggregates
#

lib_LTLIBRARIES =		libp2functions.la

SOURCE_FILE_ROOTS =		\
				functionLoader

libp2functions_la_SOURCES =	$(SOURCE_FILE_ROOTS:=.h) $(SOURCE_FILE_ROOTS:=.C)

libp2functions_la_CPPFLAGS =	@BOOST_INC@ \
				-I../p2core \
				-I../elements

libp2functions_la_CXXFLAGS =	-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

CLEANFILES = core *.core *~

MAINTAINERCLEANFILES = Makefile.in


