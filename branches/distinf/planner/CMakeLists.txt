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
# DESCRIPTION: Makefile for the P2 planner (SIGMOD 2006 version)
#

ADD_LIBRARY(p2planner 
            tableStore.C 
            planContext.C 
            localize_context.C 
            eca_context.C 
            ruleStrand.C 
            netPlanner.C 
            planner.C 
            stageStrand.C
            )
            
# need to explicitly mention dependencies on ol_lexer.h since CMake can't 
# figure it out.
ADD_DEPENDENCIES(p2planner p2overlog p2core)        

TARGET_LINK_LIBRARIES(p2planner p2core)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${P2_BINARY_DIR}/overlog ${P2_BINARY_DIR}/p2core)

INSTALL(TARGETS p2planner LIBRARY DESTINATION lib
			  ARCHIVE DESTINATION lib)
