########################################################################
#
#  @(#)$Id: $
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

ADD_LIBRARY(p2cpp p2.C)
INCLUDE_DIRECTORIES(${P2_BINARY_DIR}/p2core ${P2_BINARY_DIR}/overlog)

ADD_DEPENDENCIES(p2cpp p2overlog)

