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
# DESCRIPTION: Makefile for the P2 event loop library
#

lib_LTLIBRARIES = libp2cpp.la
libp2cpp_la_SOURCES = \
	p2.C


INCLUDES = @BOOST_INC@ -I../p2core -I../elements \
           -I../eventLoop -I../overlog -I../net -I../debugger

CLEANFILES = core *.core *~

EXTRA_DIST = .cvsignore README \
		$(libp2cpp_la_SOURCES:.C=.h)

MAINTAINERCLEANFILES = Makefile.in

CXXFLAGS= -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG @CXXFLAGS@ ${PYTHON_CPPFLAGS}

# End of File
############################################################

