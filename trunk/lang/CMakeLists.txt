########################################################################
#
#  @(#)$Id:  $
#
# Copyright (c) 2007 Intel Corporation. All rights reserved.
#
# This file is distributed under the terms in the attached INTEL-LICENSE file.
# If you do not find these files, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
#
# DESCRIPTION: CMakefile for the P2 planner
#

ADD_LIBRARY(p2lang
            compileContext.C
            langLoader.C
)

SUBDIRS(snlog parse eca debug planner p2dl compound rewrite0 rewrite1 rewrite2 secure)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/olg DESTINATION lang USE_SOURCE_PERMISSIONS)

INSTALL(TARGETS p2lang LIBRARY DESTINATION lib)
