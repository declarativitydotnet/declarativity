/*
 * @(#)$Id$
 *
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: A chord dataflow.
 *
 */

#ifndef __RING_H__
#define __RING_H__


#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>
#include <stdlib.h>

#include "tuple.h"
#include "router.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_str.h"
#include "val_id.h"

#include "print.h"
#include "discard.h"
#include "pelTransform.h"
#include "duplicate.h"
#include "duplicateConservative.h"
#include "dupElim.h"
#include "filter.h"
#include "timedPullPush.h"
#include "udp.h"
#include "marshalField.h"
#include "unmarshalField.h"
#include "mux.h"
#include "roundRobin.h"
#include "demux.h"
#include "strToSockaddr.h"
#include "slot.h"
#include "timedPullSink.h"
#include "timestampSource.h"
#include "hexdump.h"
#include "table.h"
#include "lookup.h"
#include "aggregate.h"
#include "insert.h"
#include "scan.h"
#include "delete.h"
#include "pelScan.h"
#include "functorSource.h"
#include "tupleSource.h"
#include "queue.h"
#include "noNull.h"
#include "noNullField.h"


/** SU1: bestSuccessorDist@NI(NI,min<D>) :- node@NI(NI,N),
    successor@NI(NI,S,SI), D=f_dist(N,S)
*/
void ruleSU1(str name,
             Router::ConfigurationRef conf,
             TableRef nodeTable,
             TableRef successorTable,
             ElementSpecRef pushSuccessorIn,
             int pushSuccessorInPort,
             ElementSpecRef pullBSDOut,
             int pullBSDOutPort)
{
  // Join with node
  ElementSpecRef matchSuccessorIntoNodeS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("SuccessorInNode:") << name,
                                                    nodeTable,
                                                    1, // Match lookup.NI
                                                    1 // with node.NI
                                                    ));
  // Link it to the successor coming in. Pushes match already
  conf->hookUp(pushSuccessorIn, pushSuccessorInPort, matchSuccessorIntoNodeS, 0);
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));



  // Produce intermediate ephemeral result
  // res1(NI, N) from
  // <<Successor NI S SI><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 1 field pop /* output successor.NI */ \
                                                     $1 2 field pop /* output node.N */"));
  conf->hookUp(matchSuccessorIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  // Run aggregation over successor table
  // Agg bestSuccessorDist
  // NI, min<D>
  // (res1.NI == successor.NI,
  //  D = distance(N, S))
  ElementSpecRef findMinInSuccessorS =
    conf->addElement(New refcounted< PelScan >(str("bestSuccessorDist:") << name,
                                               successorTable, 1,
                                               str("$1 /* res1.NI */ \
                                                    $2 /* NI res1.N */ \
                                                    1 ->u32 ->id 0 ->u32 ->id distance /* NI N maxdist */"),
                                               str("2 peek /* NI */ \
                                                    $1 /* successor.NI */ \
                                                    ==s not /* res1.NI != successor.NI */ \
                                                    ifstop /* empty */ \
                                                    1 peek /* N */ \
                                                    $2  /* N successor.S */ \
                                                    distance --id dup /* dist(N,S) twice */ \
                                                    2 peek /* newDist newDist oldDist */ \
                                                    <id /* newDist (newDist<oldDist) */ \
                                                    swap /* (newDist<oldDist) newDist */ \
                                                    2 peek ifelse /* ((newDist<oldDist) ? newDist : oldDist) */ \
                                                    swap /* swap newMin in state where oldMin was */ \
                                                    drop /* only state remains */"),
                                               str("\"bestSuccessorDist\" pop 2 peek pop /* output NI */\
                                                   pop /* output minDistance */\
                                                   drop drop /* empty the stack */")));
  // Res1 must be pushed to second join
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name,
                                                     0));



  // Link the join to the aggregation
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, findMinInSuccessorS, 0);
  conf->hookUp(findMinInSuccessorS, 0, pullBSDOut, pullBSDOutPort);
}



/** SU2: bestSuccessor@NI(NI,S,SI) :- node@NI(NI,N),
    bestSuccessorDist@NI(NI,D), successor@NI(NI,S,SI), D=f_dist(N,S)
*/
void ruleSU2(str name,
             Router::ConfigurationRef conf,
             TableRef nodeTable,
             TableRef successorTable,
             TableRef bestSuccessorTable,
             ElementSpecRef pushBSDIn,
             int pushBSDInPort,
             ElementSpecRef pullBestSuccessorOut,
             int pullBestSuccessorOutPort)
{
  // Join with node
  ElementSpecRef matchBSDIntoNodeS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("BSDInNode:") << name,
                                                    nodeTable,
                                                    1, // Match bSD.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  // Link it to the BSD coming in. Pushes match already
  conf->hookUp(pushBSDIn, pushBSDInPort, matchBSDIntoNodeS, 0);



  // Produce intermediate ephemeral result
  // res1(NI, N, D) from
  // <<BSD NI D><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 1 field pop /* output BSD.NI */ \
                                                     $1 2 field pop /* output node.N */ \
                                                     $0 2 field pop /* output BSD.D */ \
                                                     "));
  conf->hookUp(matchBSDIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  // Join with successor table
  ElementSpecRef matchRes1IntoSuccessorS =
    conf->addElement(New refcounted< MultLookup >(strbuf("Res1InSuccessor:") << name,
                                                  successorTable,
                                                  1, // Match res1.NI
                                                  1 // with successor.NI
                                                  ));
  ElementSpecRef noNull2S = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull2:") << name, 1));

  // Res1 must be pushed to second join
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name,
                                                     0));
  // Link the two joins together
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, matchRes1IntoSuccessorS, 0);

  // Select out the outcomes that match the select clause
  // (res1.D == distance(res1.N, successor.S))
  // from <<Res1 NI N D><Successor NI S SI>>
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("SelectRes1:").cat(name),
                                                    "$0 2 field /* res1.N */\
                                                     $1 2 field /* D successor.S */\
                                                     distance --id /* dist(N,S) */\
                                                     $0 3 field /* dist(N,S) res1.D */\
                                                     ==id /* (dist==D) */\
                                                     not ifstop /* drop if not equal */\
                                                     $0 pop $1 pop /* pass through */"));
  conf->hookUp(matchRes1IntoSuccessorS, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, selectS, 0);

  // Turn into bestSuccessor
  // from <<Res1...><Successor...>>
  ElementSpecRef makeBSS =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenSuccessor:").cat(name),
                                                    "$1 unbox drop /* replace with BestSuccessor */\
                                                     \"bestSuccessor\" pop /* new literal */\
                                                     pop pop pop /* Remaining fields */"));
  conf->hookUp(selectS, 0, makeBSS, 0);
  conf->hookUp(makeBSS, 0, pullBestSuccessorOut, pullBestSuccessorOutPort);
}


/** SR1: successorCount(NI, count<>) :- successor(NI, S, SI)
*/
void ruleSR1(str name,
             Router::ConfigurationRef conf,
             Table::MultAggregate aggregate,
             ElementSpecRef pullSuccessorCountOut,
             int pullSuccessorCountOutPort)
{
  // Create the agg element
  ElementSpecRef successorCountAggS =
    conf->addElement(New refcounted< Aggregate >(strbuf("CountSuccessors:") << name,
                                                 aggregate));


  // Produce result
  // successorCount(NI, C) from
  // <NI C>
  ElementSpecRef makeSuccessorCountS =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenSuccessorCount:").cat(name),
                                                    "\"successorCount\" pop \
                                                     $0 pop /* output NI */ \
                                                     $1 pop /* output C */ \
                                                     "));
  conf->hookUp(successorCountAggS, 0, makeSuccessorCountS, 0);
  conf->hookUp(makeSuccessorCountS, 0, pullSuccessorCountOut, pullSuccessorCountOutPort);
}

/**
   rule SR2 evictSuccessor@NI(NI) :- successorCount@NI(NI,C),
   C>successor.size.
*/
void ruleSR2(str name,
             Router::ConfigurationRef conf,
             unsigned successorSize,
             ElementSpecRef pushSuccessorCountIn,
             int pushSuccessorCountInPort,
             ElementSpecRef pullEvictOut,
             int pullEvictOutPort)
{
  // Join with node
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("SelectSuccCount:").cat(name),
                                                    strbuf() << successorSize
                                                    << " $2 >i /* successor.Size < C*/\
                                                        ifstop /* drop if less than max */\
                                                        \"evictSuccessor\" pop\
                                                        $1 pop /* NI */"));
  conf->hookUp(pushSuccessorCountIn, pushSuccessorCountInPort, selectS, 0);

  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:") << name));
  conf->hookUp(selectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullEvictOut, pullEvictOutPort);
}

/** rule SR3 maxSuccessorDist@NI(NI,max<D>) :- successor@NI(NI,S,SI),
	node@NI(NI,N), D = f_dist(N,S), evictSuccessor@NI(NI).
*/
void ruleSR3(str name,
             Router::ConfigurationRef conf,
             TableRef nodeTable,
             TableRef successorTable,
             ElementSpecRef pushEvictSuccessorIn,
             int pushEvictSuccessorInPort,
             ElementSpecRef pullMSDOut,
             int pullMSDOutPort)
{
  // Join evictSuccessor with node
  ElementSpecRef matchEvictSuccessorIntoNodeS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("evictSuccessorInNode:") << name,
                                                    nodeTable,
                                                    1, // Match EvictSuccessor.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  // Link it to the evictSuccessor coming in. Pushes match already
  conf->hookUp(pushEvictSuccessorIn, pushEvictSuccessorInPort, matchEvictSuccessorIntoNodeS, 0);



  // Produce intermediate ephemeral result
  // res1(NI, N) from
  // <<evictSuccessor NI><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 1 field pop /* output evictSuccessor.NI */ \
                                                     $1 2 field pop /* output node.N */"));
  conf->hookUp(matchEvictSuccessorIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  // Run aggregation over successor table
  // Agg bestSuccessorDist
  // NI, max<D>
  // (res1.NI == successor.NI,
  //  D = distance(N, S))
  ElementSpecRef findMaxInSuccessorS =
    conf->addElement(New refcounted< PelScan >(str("bestSuccessorDist:") << name,
                                               successorTable, 1,
                                               str("$1 /* res1.NI */ \
                                                    $2 /* NI res1.N */ \
                                                    0 ->u32 ->id /* NI N mindist */"),
                                               str("2 peek /* NI */ \
                                                    $1 /* successor.NI */ \
                                                    ==s not /* res1.NI != successor.NI */ \
                                                    ifstop /* empty */ \
                                                    1 peek /* N */ \
                                                    $2  /* N successor.S */ \
                                                    distance --id dup /* dist(N,S) twice */ \
                                                    2 peek /* newDist newDist oldDist */ \
                                                    >id /* newDist (newDist>oldDist) */ \
                                                    swap /* (newDist>oldDist) newDist */ \
                                                    2 peek ifelse /* ((newDist>oldDist) ? newDist : oldDist) */ \
                                                    swap /* swap newMax in state where oldMax was */ \
                                                    drop /* only state remains */"),
                                               str("\"maxSuccessorDist\" pop /* output name */\
                                                    $1 pop /* output NI */\
                                                    pop /* output maxDistance */\
                                                    drop drop /* empty the stack */")));
  // Res1 must be pushed to second join
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name,
                                                     0));



  // Link the join to the aggregation
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, findMaxInSuccessorS, 0);
  conf->hookUp(findMaxInSuccessorS, 0, pullMSDOut, pullMSDOutPort);
}

/** SR4: maxSuccessor(NI, S, SI) :- successor(NI, S, SI),
    maxSuccessorDist(NI, D), D=dist(N, S)
*/
void ruleSR4(str name,
             Router::ConfigurationRef conf,
             TableRef nodeTable,
             TableRef successorTable,
             ElementSpecRef pushMSDIn,
             int pushMSDInPort)
{
  // Join with node
  ElementSpecRef matchMSDIntoNodeS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("MSDInNode:") << name,
                                                    nodeTable,
                                                    1, // Match bSD.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  // Link it to the MSD coming in. Pushes match already
  conf->hookUp(pushMSDIn, pushMSDInPort, matchMSDIntoNodeS, 0);


  // Produce intermediate ephemeral result
  // res1(NI, N, D) from
  // <<MSD NI D><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 1 field pop /* output MSD.NI */ \
                                                     $1 2 field pop /* output node.N */ \
                                                     $0 2 field pop /* output MSD.D */ \
                                                     "));
  conf->hookUp(matchMSDIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  // Join with successor table
  ElementSpecRef matchRes1IntoSuccessorS =
    conf->addElement(New refcounted< MultLookup >(strbuf("Res1InSuccessor:") << name,
                                                  successorTable,
                                                  1, // Match res1.NI
                                                  1 // with successor.NI
                                                  ));
  ElementSpecRef noNull2S = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull2:") << name, 1));

  // Res1 must be pushed to second join
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name,
                                                     0));
  // Link the two joins together
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, matchRes1IntoSuccessorS, 0);

  // Select out the outcomes that match the select clause
  // (res1.D == distance(res1.N, successor.S))
  // from <<Res1 NI N D><Successor NI S SI>>
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("SelectRes1:").cat(name),
                                                    "$0 2 field /* res1.N */\
                                                     $1 2 field /* D successor.S */\
                                                     distance --id /* dist(N,S) */\
                                                     $0 3 field /* dist(N,S) res1.D */\
                                                     ==id /* (dist==D) */\
                                                     not ifstop /* drop if not equal */\
                                                     $0 pop $1 pop /* pass through */"));
  conf->hookUp(matchRes1IntoSuccessorS, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, selectS, 0);

  // Turn into res2
  // from <<Res1...><Successor...>>
  ElementSpecRef makeMSS =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenRes2:").cat(name),
                                                    "$1 unbox drop /* replace with Res2 */\
                                                     \"Res2\" pop /* new literal */\
                                                     pop pop pop /* Remaining fields */"));
  conf->hookUp(selectS, 0, makeMSS, 0);

  // And send it for deletion
  ElementSpecRef deleteSuccessorS =
    conf->addElement(New refcounted< Delete >(strbuf("DeleteSuccessor:") << name,
                                              successorTable,
                                              2,
                                              2));
  ElementSpecRef pushRes2S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes2:") << name,
                                                     0));
  // Link the two joins together
  conf->hookUp(makeMSS, 0, pushRes2S, 0);
  conf->hookUp(pushRes2S, 0, deleteSuccessorS, 0);
}


/** rule S0 stabilize@NI(NI, E) :- periodic@NI(TTL * 0.5), E=f_rand(),
    NI=ni. */
void ruleS0(str name,
            Router::ConfigurationRef conf,
            str localAddress,
            double fingerTTL,
            ElementSpecRef pullStabilizeOut,
            int pullStabilizeOutPort)
{
  // My stabilize tuple
  TupleRef stabilizeTuple = Tuple::mk();
  stabilizeTuple->append(Val_Str::mk("stabilizeEvent"));
  stabilizeTuple->append(Val_Str::mk(localAddress));
  stabilizeTuple->freeze();

  ElementSpecRef sourceS =
    conf->addElement(New refcounted< TupleSource >(str("StabilizeSource:") << name,
                                                   stabilizeTuple));
  
  // The timed pusher
  ElementSpecRef pushS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("StabilizePush:") << name,
                                                     fingerTTL * 0.5));
  
  // And a slot from which to pull
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("StabilizeSlot:") << name));

  // Link everything
  conf->hookUp(sourceS, 0, pushS, 0);
  conf->hookUp(pushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullStabilizeOut, pullStabilizeOutPort);
}


/** rule S0a stabilize@NI(NI, E) :- stabilizeEvent@NI(NITTL * 0.5),
    E=f_rand(), NI=ni.
*/
void ruleS0a(str name,
             Router::ConfigurationRef conf,
             ElementSpecRef pushStabilizeEventIn,
             int pushStabilizeEventInPort,
             ElementSpecRef pullStabilizeOut,
             int pullStabilizeOutPort)
{
  // Project onto stabilize(NI, E)
  // from
  // stabilizeEvent(NI)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"stabilize\" pop \
                                                     $1 pop /* out sE.NI */\
                                                     $1 \":\" strcat rand strcat pop /* out random */\
                                                     "));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:") << name));
  conf->hookUp(pushStabilizeEventIn, pushStabilizeEventInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullStabilizeOut, pullStabilizeOutPort);
}


/** rule S0b stabilizeRecord@NI(NI, E) :- stabilize@NI(NI, E).
 */
void ruleS0b(str name,
             Router::ConfigurationRef conf,
             ElementSpecRef pushStabilizeIn,
             int pushStabilizeInPort,
             ElementSpecRef pullStabilizeRecordOut,
             int pullStabilizeRecordOutPort)
{
  // Project onto stabilizeRecord(NI, E)
  // from
  // stabilize(NI, E)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"stabilizeRecord\" pop \
                                                     $1 pop /* out s.NI */\
                                                     $2 pop /* out s.E */\
                                                     "));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:") << name));
  conf->hookUp(pushStabilizeIn, pushStabilizeInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullStabilizeRecordOut, pullStabilizeRecordOutPort);
}

/**
   rule S1 stabilizeRequest@SI(SI,NI,E) :- stabilize@NI(NI,E),
   bestSuccessor@NI(NI,S,SI),
 */
void ruleS1(str name,
            Router::ConfigurationRef conf,
            TableRef bestSuccessorTable,
            ElementSpecRef pushStabilizeIn,
            int pushStabilizeInPort,
            ElementSpecRef pullStabilizeRequestOut,
            int pullStabilizeRequestOutPort)
{
  // Join with best successor
  ElementSpecRef joinS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("StabilizeInBestSuccessor:") << name,
                                                    bestSuccessorTable,
                                                    1, // Match stabilize.NI
                                                    1 // with bestSuccessor.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));
  // Link it to the stabilize coming in. Pushes match already
  conf->hookUp(pushStabilizeIn, pushStabilizeInPort, joinS, 0);
  conf->hookUp(joinS, 0, noNullS, 0);



  // Produce result
  // stabilizeRequest(SI, NI, E) from
  // stabilize(NI, E), bestSuccesssor(NI,S, SI)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("Project:").cat(name),
                                                    "\"stabilizeRequest\" pop \
                                                     $1 3 field pop /* SI */\
                                                     $0 1 field pop /* NI */\
                                                     $0 2 field pop /* E */\
                                                     "));
  conf->hookUp(noNullS, 0, projectS, 0);
  conf->hookUp(projectS, 0, pullStabilizeRequestOut, pullStabilizeRequestOutPort);
}

/** 
    rule S2 sendPredecessor@PI1(PI1,P,PI,E) :-
    stabilizeRequest@NI(NI,PI1,E), predecessor@NI(NI,P,PI), PI != null.
*/
void ruleS2(str name,
            Router::ConfigurationRef conf,
            TableRef predecessorTable,
            ElementSpecRef pushStabilizeRequestIn,
            int pushStabilizeRequestInPort,
            ElementSpecRef pullSendPredecessorOut,
            int pullSendPredecessorOutPort)
{
  // StabilizeRequest stabilizeRequest with landmark table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("stabilizeRequestIntoPredecessor:") << name,
                                                    predecessorTable,
                                                    1, // Match stabilizeRequest.NI
                                                    1 // with predecessor.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  conf->hookUp(pushStabilizeRequestIn, pushStabilizeRequestInPort, join1S, 0);
  conf->hookUp(join1S, 0, noNullS, 0);


  // Select predecessor.PI != null
  // stabilizeRequest(NI, PI1, E),predecessor(NI, P, PI)>
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("select:") << name,
                                                    "$1 3 field /* predecessor.PI */\
                                                     \"-\" ==s ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     "));
  conf->hookUp(noNullS, 0, selectS, 0);


  // Now project onto the result
  // sendPredecessor(PI1, P, PI, E)
  // from
  // stabilizeRequest(NI, PI1, E),predecessor(NI, P, PI)>
  ElementSpecRef project1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"sendPredecessor\" pop \
                                                     $0 2 field pop /* out stabilizeRequest.PI1 */\
                                                     $1 2 field pop /* out predecessor.P */\
                                                     $1 3 field pop /* out predecessor.PI */\
                                                     $0 3 field pop /* out stabilizeRequest.E */\
                                                     "));
  conf->hookUp(selectS, 0, project1S, 0);
  conf->hookUp(project1S, 0, pullSendPredecessorOut, pullSendPredecessorOutPort);
}


/** 
    rule S3 successor@NI(NI,P,PI) :- node(NI,N),
    sendPredecessor@NI(NI,P,PI,E), bestSuccessor@NI(NI,S,SI), P in
    (N,S), stabilizeRecord@NI(NI, E).
*/
void ruleS3(str name,
            Router::ConfigurationRef conf,
            TableRef stabilizeRecordTable,
            TableRef nodeTable,
            TableRef bestSuccessorTable,
            ElementSpecRef pushSendPredecessorIn,
            int pushSendPredecessorInPort,
            ElementSpecRef pullSuccessorOut,
            int pullSuccessorOutPort)
{
  // SendPredecessor sendPredecessor with landmark table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("sendPredecessorIntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match sendPredecessor.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));
  conf->hookUp(pushSendPredecessorIn, pushSendPredecessorInPort, join1S, 0);
  conf->hookUp(join1S, 0, noNullS, 0);

  // from
  // sendPredecessor(NI, P, PI, E), node(NI, N)>
  ElementSpecRef project1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("Project:").cat(name),
                                                    "\"res1\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field pop /* out P */\
                                                     $0 3 field pop /* out PI */\
                                                     $0 4 field pop /* out E */\
                                                     $1 2 field pop /* out node.N */\
                                                     "));
  conf->hookUp(noNullS, 0, project1S, 0);

  
  // Now join res1 with stabilize record
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name, 0));
  ElementSpecRef join2S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("res1IntoStabilizeRecord:") << name,
                                                    stabilizeRecordTable,
                                                    1, // Match res1.NI
                                                    1 // with stabilizeRecord.NI
                                                    ));
  ElementSpecRef noNull2S = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull2:") << name, 1));
  conf->hookUp(project1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, join2S, 0);
  conf->hookUp(join2S, 0, noNull2S, 0);
  
  // Select res1.E == stabilizeRecord.E
  // from
  // res1(NI, P, PI, E, N), stabilizeRecord(NI, E)
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("select:") << name,
                                                    "$0 4 field /* res1.E */\
                                                     $1 2 field /* res1.E sR.E */\
                                                     ==s not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     "));
  conf->hookUp(noNull2S, 0, selectS, 0);

  // Project back to res1.
  // from
  // res1()..., stabilizeRecord(...)
  ElementSpecRef project2S =
    conf->addElement(New refcounted< PelTransform >(strbuf("Project2:").cat(name),
                                                    "$0 unboxPop\
                                                     "));
  conf->hookUp(selectS, 0, project2S, 0);


  // Finally join res1 with bestSuccessorTable
  ElementSpecRef push2S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("Push2:") << name, 0));
  ElementSpecRef join3S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("res1IntoBestSuccessor:") << name,
                                                    bestSuccessorTable,
                                                    1, // Match res1.NI
                                                    1 // with bestSuccessor.NI
                                                    ));
  ElementSpecRef noNull3S = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull3:") << name, 1));
  conf->hookUp(project2S, 0, push2S, 0);
  conf->hookUp(push2S, 0, join3S, 0);
  conf->hookUp(join3S, 0, noNull3S, 0);
  

  // Select P in (N, S)
  // from
  // res1(NI, P, PI, E, N), bestSuccessor(NI, S, SI)
  ElementSpecRef select2S =
    conf->addElement(New refcounted< PelTransform >(strbuf("select2:") << name,
                                                    "$0 2 field /* P */\
                                                     $0 5 field /* P N */\
                                                     $1 2 field /* P N S */\
                                                     ()id not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     "));
  conf->hookUp(noNull3S, 0, select2S, 0);


  // Finally project onto the result
  // successor(NI, P, PI)
  // from
  // res1(NI, P, PI, E, N), bestSuccessor(NI, S, SI)
  ElementSpecRef project3S =
    conf->addElement(New refcounted< PelTransform >(strbuf("project3:").cat(name),
                                                    "\"successor\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field pop /* out P */\
                                                     $0 3 field pop /* out PI */\
                                                     "));
  conf->hookUp(select2S, 0, project3S, 0);
  conf->hookUp(project3S, 0, pullSuccessorOut, pullSuccessorOutPort);
}


/**
   rule S4 sendSuccessors@SI(SI,NI,E) :- stabilize@NI(NI,E),
   successor@NI(NI,S,SI).
 */
void ruleS4(str name,
            Router::ConfigurationRef conf,
            TableRef successorTable,
            ElementSpecRef pushStabilizeIn,
            int pushStabilizeInPort,
            ElementSpecRef pullSendSuccessorsOut,
            int pullSendSuccessorsOutPort)
{
  // Join with successor
  ElementSpecRef joinS =
    conf->addElement(New refcounted< MultLookup >(strbuf("StabilizeInSuccessor:") << name,
                                                  successorTable,
                                                  1, // Match stabilize.NI
                                                  1 // with successor.NI
                                                  ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));
  // Link it to the stabilize coming in. Pushes match already
  conf->hookUp(pushStabilizeIn, pushStabilizeInPort, joinS, 0);
  conf->hookUp(joinS, 0, noNullS, 0);



  // Produce result
  // sendSuccessors(SI, NI, E) from
  // stabilize(NI, E), successsor(NI,S, SI)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("Project:").cat(name),
                                                    "\"sendSuccessors\" pop \
                                                     $1 3 field pop /* SI */\
                                                     $0 1 field pop /* NI */\
                                                     $0 2 field pop /* E */\
                                                     "));
  conf->hookUp(noNullS, 0, projectS, 0);
  conf->hookUp(projectS, 0, pullSendSuccessorsOut, pullSendSuccessorsOutPort);
}



/**
   rule S5 returnSuccessor@PI(PI,S,SI,E) :- sendSuccessors@NI(NI,PI,E),
   successor@NI(NI,S,SI).
 */
void ruleS5(str name,
            Router::ConfigurationRef conf,
            TableRef successorTable,
            ElementSpecRef pushSendSuccessorsIn,
            int pushSendSuccessorsInPort,
            ElementSpecRef pullReturnSuccessorOut,
            int pullReturnSuccessorOutPort)
{
  // Join with successor
  ElementSpecRef joinS =
    conf->addElement(New refcounted< MultLookup >(strbuf("SendSuccessorsInSuccessor:") << name,
                                                  successorTable,
                                                  1, // Match sendSuccessors.NI
                                                  1 // with successor.NI
                                                  ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));
  // Link it to the sendSuccessors coming in. Pushes match already
  conf->hookUp(pushSendSuccessorsIn, pushSendSuccessorsInPort, joinS, 0);
  conf->hookUp(joinS, 0, noNullS, 0);



  // Produce result
  // returnSuccessor(PI, S, SI, E) from
  // sendSuccessors(NI,PI,E), successsor(NI,S, SI)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("Project:").cat(name),
                                                    "\"returnSuccessor\" pop \
                                                     $0 2 field pop /* PI */\
                                                     $1 2 field pop /* S */\
                                                     $1 3 field pop /* SI */\
                                                     $0 3 field pop /* E */\
                                                     "));
  conf->hookUp(noNullS, 0, projectS, 0);
  conf->hookUp(projectS, 0, pullReturnSuccessorOut, pullReturnSuccessorOutPort);
}


/** 
    rule S5a successor@NI(NI, S, SI) :- returnSuccessor@NI(NI,S,SI,E),
    stabilizeRecord@NI(NI, E).
*/
void ruleS5a(str name,
             Router::ConfigurationRef conf,
             TableRef stabilizeRecordTable,
             ElementSpecRef pushReturnSuccessorIn,
             int pushReturnSuccessorInPort,
             ElementSpecRef pullSuccessorOut,
             int pullSuccessorOutPort)
{
  // returnSuccessor with stabilizerecord table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("returnSuccessorIntoStabilizeRecord:") << name,
                                                    stabilizeRecordTable,
                                                    1, // Match returnSuccessor.NI
                                                    1 // with stabilizeRecord.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));
  conf->hookUp(pushReturnSuccessorIn, pushReturnSuccessorInPort, join1S, 0);
  conf->hookUp(join1S, 0, noNullS, 0);

  // Select rS.E == stabilizeRecord.E
  // from
  // rS(NI, S, SI, E), stabilizeRecord(NI, E)
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("select:") << name,
                                                    "$0 4 field /* rS.E */\
                                                     $1 2 field /* rS1.E sR.E */\
                                                     ==s not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     "));
  conf->hookUp(noNullS, 0, selectS, 0);

  // Finally project onto the result
  // successor(NI, S, SI)
  // from
  // rS(NI, S, SI, E), stabilizeRecord(NI, E)
  ElementSpecRef project3S =
    conf->addElement(New refcounted< PelTransform >(strbuf("project3:").cat(name),
                                                    "\"successor\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field pop /* out S */\
                                                     $0 3 field pop /* out SI */\
                                                     "));
  conf->hookUp(selectS, 0, project3S, 0);
  conf->hookUp(project3S, 0, pullSuccessorOut, pullSuccessorOutPort);
}


/** rule S6a notify@NI(NI) :- periodic@NI(TTL * 0.5), NI=ni. */
void ruleS6a(str name,
             Router::ConfigurationRef conf,
             str localAddress,
             double fingerTTL,
             ElementSpecRef pullNotifyOut,
             int pullNotifyOutPort)
{
  // My notify tuple
  TupleRef notifyTuple = Tuple::mk();
  notifyTuple->append(Val_Str::mk("notify"));
  notifyTuple->append(Val_Str::mk(localAddress));
  notifyTuple->freeze();

  ElementSpecRef sourceS =
    conf->addElement(New refcounted< TupleSource >(str("NotifySource:") << name,
                                                   notifyTuple));
  
  // The timed pusher
  ElementSpecRef pushS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("NotifyPush:") << name,
                                                     fingerTTL * 0.5));
  
  // And a slot from which to pull
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("NotifySlot:") << name));

  // Link everything
  conf->hookUp(sourceS, 0, pushS, 0);
  conf->hookUp(pushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullNotifyOut, pullNotifyOutPort);
}


/** 
    rule S6 notifyPredecessor@SI(SI,N,NI) :- notify@NI(NI),
    node@NI(NI,N), successor@NI(NI,S,SI).
*/
void ruleS6(str name,
            Router::ConfigurationRef conf,
            TableRef nodeTable,
            TableRef successorTable,
            ElementSpecRef pushNotifyIn,
            int pushNotifyInPort,
            ElementSpecRef pullNotifyPredecessorOut,
            int pullNotifyPredecessorOutPort)
{
  // Join notify with node
  ElementSpecRef join1S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("notifyIntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match notify.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));
  conf->hookUp(pushNotifyIn, pushNotifyInPort, join1S, 0);
  conf->hookUp(join1S, 0, noNullS, 0);

  // Project res1(NI, N)
  // from
  // notify(NI), node(NI, N)>
  ElementSpecRef project1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("Project:").cat(name),
                                                    "\"res1\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $1 2 field pop /* out node.N */\
                                                     "));
  conf->hookUp(noNullS, 0, project1S, 0);

  
  // Now join res1 with successor
  ElementSpecRef push1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("Push1:") << name, 0));
  ElementSpecRef join2S =
    conf->addElement(New refcounted< MultLookup >(strbuf("res1IntoSuccessor:") << name,
                                                    successorTable,
                                                    1, // Match res1.NI
                                                    1 // with successor.NI
                                                    ));
  ElementSpecRef noNull2S = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull2:") << name, 1));
  conf->hookUp(project1S, 0, push1S, 0);
  conf->hookUp(push1S, 0, join2S, 0);
  conf->hookUp(join2S, 0, noNull2S, 0);
  
  // Project to notifyPredecessor(SI, N, NI)
  // from
  // res1(NI, N), successor(NI, S, SI)
  ElementSpecRef project2S =
    conf->addElement(New refcounted< PelTransform >(strbuf("Project2:").cat(name),
                                                    "\"notifyPredecessor\" pop \
                                                     $1 3 field pop /* out SI */\
                                                     $0 2 field pop /* out N */\
                                                     $0 1 field pop /* out NI */\
                                                     "));
  conf->hookUp(noNull2S, 0, project2S, 0);

  conf->hookUp(project2S, 0, pullNotifyPredecessorOut, pullNotifyPredecessorOutPort);
}


/** 
    rule S7 predecessor@NI(NI,P,PI) :- node@NI(NI,N),
    notifyPredecessor@NI(NI,P,PI), predecessor@NI(NI,P1,PI1), ((PI1 ==
    "") || (P in (P1, N))).
*/
void ruleS7(str name,
            Router::ConfigurationRef conf,
            TableRef nodeTable,
            TableRef predecessorTable,
            ElementSpecRef pushNotifyPredecessorIn,
            int pushNotifyPredecessorInPort,
            ElementSpecRef pullPredecessorOut,
            int pullPredecessorOutPort)
{
  // Join notifyPredecessor with table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("notifyPredecessorIntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match notifyPredecessor.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));
  conf->hookUp(pushNotifyPredecessorIn, pushNotifyPredecessorInPort, join1S, 0);
  conf->hookUp(join1S, 0, noNullS, 0);

  // Project res1(NI, P, PI, N)
  // from
  // notifyPredecessor(NI, P, PI), node(NI, N)>
  ElementSpecRef project1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("Project:").cat(name),
                                                    "\"res1\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field pop /* out P */\
                                                     $0 3 field pop /* out PI */\
                                                     $1 2 field pop /* out node.N */\
                                                     "));
  conf->hookUp(noNullS, 0, project1S, 0);

  
  // Finally join res1 with predecessorTable
  ElementSpecRef push2S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("Push2:") << name, 0));
  ElementSpecRef join3S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("res1IntoPredecessor:") << name,
                                                    predecessorTable,
                                                    1, // Match res1.NI
                                                    1 // with predecessor.NI
                                                    ));
  ElementSpecRef noNull3S = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull3:") << name, 1));
  conf->hookUp(project1S, 0, push2S, 0);
  conf->hookUp(push2S, 0, join3S, 0);
  conf->hookUp(join3S, 0, noNull3S, 0);
  

  // Select (PI1 == "" || (P in (P1, N))).
  // from
  // res1(NI, P, PI, N), predecessor(NI, P1, PI1)
  ElementSpecRef select2S =
    conf->addElement(New refcounted< PelTransform >(strbuf("select2:") << name,
                                                    "$0 2 field /* P */\
                                                     $1 2 field /* P P1 */\
                                                     $0 4 field /* P P1 N */\
                                                     ()id /* (P in(P1,N)) */\
                                                     $1 3 field /* (P in(P1,N)) PI1 */\
                                                     \"-\" ==s or not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     "));
  conf->hookUp(noNull3S, 0, select2S, 0);


  // Finally project onto the result
  // predecessor(NI, P, PI)
  // from
  // res1(NI, P, PI, N), predecessor(NI, S, SI)
  ElementSpecRef project3S =
    conf->addElement(New refcounted< PelTransform >(strbuf("project3:").cat(name),
                                                    "\"predecessor\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field pop /* out P */\
                                                     $0 3 field pop /* out PI */\
                                                     "));
  conf->hookUp(select2S, 0, project3S, 0);
  conf->hookUp(project3S, 0, pullPredecessorOut, pullPredecessorOutPort);
}



/*
 * End of file 
 */
#endif /* __RING_H_ */
