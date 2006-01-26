########################################################################
#
#  @(#)$Id$
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
#
# DESCRIPTION: Makefile for the P2 event loop library
#

lib_LTLIBRARIES = libeventLoop.la
libeventLoop_la_SOURCES = \
	loop.C \
	p2Time.C

INCLUDES = @BOOST_INC@

CLEANFILES = core *.core *~ *.gen.h *.gen.txt pel_lexer.C

EXTRA_DIST = .cvsignore

MAINTAINERCLEANFILES = Makefile.in

# End of File
############################################################

