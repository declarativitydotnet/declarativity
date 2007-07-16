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

lib_LTLIBRARIES =			libp2PythonElements.la

libp2PythonElements_la_SOURCES =	dataflowInstaller.C \
					dataflowInstaller.h

libp2PythonElements_la_CPPFLAGS =	@BOOST_INC@ \
					-I../p2core \
					-I../overlog \
					-I../net

libp2PythonElements_la_CXXFLAGS =	@PYTHON_CSPEC@ \
					-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

CLEANFILES = core *.core *~

MAINTAINERCLEANFILES = Makefile.in

