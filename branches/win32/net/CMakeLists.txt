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
# DESCRIPTION: CMakefile for the P2 net library
#

ADD_LIBRARY(p2net
            snetsim.C 
            skr.C 
            tman.C 
            plsensor.C 
            udp.C 
            udp2.C 
            tupleseq.C 
            basicAck.C 
            cumulativeAck.C 
            cct.C 
            rcct.C 
            rccr.C 
            rdelivery.C 
            odelivery.C 
            dupRemove.C 
            bw.C 
            frag.C 
            defrag.C 
            roundTripTimer.C
            netLoader.C
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

