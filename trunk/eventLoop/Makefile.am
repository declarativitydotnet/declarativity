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

lib_LTLIBRARIES = libeventLoop.la
libeventLoop_la_SOURCES = \
	loop.C \
	p2Time.C

INCLUDES = @BOOST_INC@ -I$(top_builddir)/p2core

CLEANFILES = core *.core *~ *.gen.h *.gen.txt pel_lexer.C

EXTRA_DIST = .cvsignore \
		$(libeventLoop_la_SOURCES:.C=.h)

MAINTAINERCLEANFILES = Makefile.in

CXXFLAGS="-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG" @CXXFLAGS@

# End of File
############################################################

