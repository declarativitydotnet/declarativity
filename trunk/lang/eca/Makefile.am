########################################################################
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

lib_LTLIBRARIES = libp2eca.la

libp2eca_la_SOURCES = ecaContext.C

INCLUDES = @BOOST_INC@ \
           -I.. -I../../p2core -I../../elements -I../../p2cpp

CLEANFILES = core *.core *~

EXTRA_DIST = $(libp2eca_la_SOURCES:.C=.h)

MAINTAINERCLEANFILES = Makefile.in
