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
#
# DESCRIPTION: Makefile for the P2 core elements library
#
ADD_LIBRARY(p2elements
            aggregate.C 
            aggwrap2.C
            aggwrap2Callback.C
            commitBuf.C
            compileStage.C
            programLoader.C 
            dDuplicateConservative.C 
            dRoundRobin.C 
            ddemux.C 
            delete.C 
            delete2.C
            demux.C
            demuxConservative.C 
            discard.C 
            dupElim.C 
            duplicate.C 
            duplicateConservative.C 
            elementLoader.C
	    elementRegistry.C
            hexdump.C 
            identity.C
            insert.C 
            insert2.C
            logger.C 
            lookup2.C 
            marshal.C 
            marshalField.C 
            mux.C 
            noNull.C 
            noNullField.C 
            onlyNullField.C
            noNullSignal.C
            pelTransform.C 
            print.C 
            tupleCounter.C 
            printTime.C 
            printWatch.C 
            pullPush.C
            queue.C 
            rangeLookup.C
            refresh.C 
            removed.C
            roundRobin.C 
            slot.C 
            stage.C
            stageRegistry.C
            staticTupleSource.C
            strToSockaddr.C 
            switch.C
            timedPullPush.C 
            tupleInjector.C
            tupleListener.C
            tupleSource.C 
            unboxField.C 
            unmarshal.C 
            unmarshalField.C 
            update.C)



ADD_DEPENDENCIES(p2elements p2core)
INCLUDE_DIRECTORIES(${P2_BINARY_DIR}/p2core)

INSTALL(TARGETS p2elements LIBRARY DESTINATION lib
			   ARCHIVE DESTINATION lib)
