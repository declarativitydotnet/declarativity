/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: A chord dataflow.
 *
 */

#ifndef __RING_H__
#define __RING_H__


#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <stdlib.h>

#include "tuple.h"
#include "plumber.h"
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
void ruleSU1(string name,
             Plumber::ConfigurationPtr conf,
             TablePtr nodeTable,
             TablePtr successorTable,
             ElementSpecPtr pushSuccessorIn,
             int pushSuccessorInPort,
             ElementSpecPtr pullBSDOut,
             int pullBSDOutPort)
{
  // Join with node
  ElementSpecPtr matchSuccessorIntoNodeS =
    conf->addElement(ElementPtr(new UniqueLookup(string("SuccessorInNode:") + name,
                                                    nodeTable,
                                                    1, // Match lookup.NI
                                                    1 // with node.NI
                                                    )));
  // Link it to the successor coming in. Pushes match already
  conf->hookUp(pushSuccessorIn, pushSuccessorInPort, matchSuccessorIntoNodeS, 0);
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));



  // Produce intermediate ephemeral result
  // res1(NI, N) from
  // <<Successor NI S SI><node NI N>>
  ElementSpecPtr makeRes1S =
    conf->addElement(ElementPtr(new PelTransform(string("FlattenRes1:").append(name),
                                                    "\"Res1\" pop \
                                                     $0 1 field pop /* output successor.NI */ \
                                                     $1 2 field pop /* output node.N */")));
  conf->hookUp(matchSuccessorIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  // Run aggregation over successor table
  // Agg bestSuccessorDist
  // NI, min<D>
  // (res1.NI == successor.NI,
  //  D = distance(N, S))
  ElementSpecPtr findMinInSuccessorS =
    conf->addElement(ElementPtr(new PelScan(string("bestSuccessorDist:") + name,
                                               successorTable, 1,
                                               string("$1 /* res1.NI */ \
                                                    $2 /* NI res1.N */ \
                                                    1 ->u32 ->id 0 ->u32 ->id distance /* NI N maxdist */"),
                                               string("2 peek /* NI */ \
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
                                               string("\"bestSuccessorDist\" pop 2 peek pop /* output NI */\
                                                   pop /* output minDistance */\
                                                   drop drop /* empty the stack */"))));
  // Res1 must be pushed to second join
  ElementSpecPtr pushRes1S =
    conf->addElement(ElementPtr(new TimedPullPush(string("PushRes1:") + name,
                                                     0)));



  // Link the join to the aggregation
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, findMinInSuccessorS, 0);
  conf->hookUp(findMinInSuccessorS, 0, pullBSDOut, pullBSDOutPort);
}



/** SU2: bestSuccessor@NI(NI,S,SI) :- node@NI(NI,N),
    bestSuccessorDist@NI(NI,D), successor@NI(NI,S,SI), D=f_dist(N,S)
*/
void ruleSU2(string name,
             Plumber::ConfigurationPtr conf,
             TablePtr nodeTable,
             TablePtr successorTable,
             TablePtr bestSuccessorTable,
             ElementSpecPtr pushBSDIn,
             int pushBSDInPort,
             ElementSpecPtr pullBestSuccessorOut,
             int pullBestSuccessorOutPort)
{
  // Join with node
  ElementSpecPtr matchBSDIntoNodeS =
    conf->addElement(ElementPtr(new UniqueLookup(string("BSDInNode:") + name,
                                                    nodeTable,
                                                    1, // Match bSD.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));

  // Link it to the BSD coming in. Pushes match already
  conf->hookUp(pushBSDIn, pushBSDInPort, matchBSDIntoNodeS, 0);



  // Produce intermediate ephemeral result
  // res1(NI, N, D) from
  // <<BSD NI D><node NI N>>
  ElementSpecPtr makeRes1S =
    conf->addElement(ElementPtr(new PelTransform(string("FlattenRes1:").append(name),
                                                    "\"Res1\" pop \
                                                     $0 1 field pop /* output BSD.NI */ \
                                                     $1 2 field pop /* output node.N */ \
                                                     $0 2 field pop /* output BSD.D */ \
                                                     ")));
  conf->hookUp(matchBSDIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  // Join with successor table
  ElementSpecPtr matchRes1IntoSuccessorS =
    conf->addElement(ElementPtr(new MultLookup(string("Res1InSuccessor:") + name,
                                                  successorTable,
                                                  1, // Match res1.NI
                                                  1 // with successor.NI
                                                  )));
  ElementSpecPtr noNull2S = conf->addElement(ElementPtr(new NoNullField(string("NoNull2:") + name, 1)));

  // Res1 must be pushed to second join
  ElementSpecPtr pushRes1S =
    conf->addElement(ElementPtr(new TimedPullPush(string("PushRes1:") + name, 0)));
  // Link the two joins together
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, matchRes1IntoSuccessorS, 0);

  // Select out the outcomes that match the select clause
  // (res1.D == distance(res1.N, successor.S))
  // from <<Res1 NI N D><Successor NI S SI>>
  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(string("SelectRes1:").append(name),
                                                    "$0 2 field /* res1.N */\
                                                     $1 2 field /* D successor.S */\
                                                     distance --id /* dist(N,S) */\
                                                     $0 3 field /* dist(N,S) res1.D */\
                                                     ==id /* (dist==D) */\
                                                     not ifstop /* drop if not equal */\
                                                     $0 pop $1 pop /* pass through */")));
  conf->hookUp(matchRes1IntoSuccessorS, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, selectS, 0);

  // Turn into bestSuccessor
  // from <<Res1...><Successor...>>
  ElementSpecPtr makeBSS =
    conf->addElement(ElementPtr(new PelTransform(string("FlattenSuccessor:").append(name),
                                                    "$1 unbox drop /* replace with BestSuccessor */\
                                                     \"bestSuccessor\" pop /* new literal */\
                                                     pop pop pop /* Remaining fields */")));
  conf->hookUp(selectS, 0, makeBSS, 0);
  conf->hookUp(makeBSS, 0, pullBestSuccessorOut, pullBestSuccessorOutPort);
}


/** SR1: successorCount(NI, count<>) :- successor(NI, S, SI)
*/
void ruleSR1(string name,
             Plumber::ConfigurationPtr conf,
             Table::MultAggregate aggregate,
             ElementSpecPtr pullSuccessorCountOut,
             int pullSuccessorCountOutPort)
{
  // Create the agg element
  ElementSpecPtr successorCountAggS =
    conf->addElement(ElementPtr(new Aggregate(string("CountSuccessors:") + name,
                                                 aggregate)));


  // Produce result
  // successorCount(NI, C) from
  // <NI C>
  ElementSpecPtr makeSuccessorCountS =
    conf->addElement(ElementPtr(new PelTransform(string("FlattenSuccessorCount:").append(name),
                                                    "\"successorCount\" pop \
                                                     $0 pop /* output NI */ \
                                                     $1 pop /* output C */ \
                                                     ")));
  conf->hookUp(successorCountAggS, 0, makeSuccessorCountS, 0);
  conf->hookUp(makeSuccessorCountS, 0, pullSuccessorCountOut, pullSuccessorCountOutPort);
}

/**
   rule SR2 evictSuccessor@NI(NI) :- successorCount@NI(NI,C),
   C>successor.size.
*/
void ruleSR2(string name,
             Plumber::ConfigurationPtr conf,
             unsigned successorSize,
             ElementSpecPtr pushSuccessorCountIn,
             int pushSuccessorCountInPort,
             ElementSpecPtr pullEvictOut,
             int pullEvictOutPort)
{
  // Join with node
  ostringstream oss;
  oss << successorSize << " $2 >i /* successor.Size < C*/\
                          ifstop /* drop if less than max */\
                          \"evictSuccessor\" pop\
                          $1 pop /* NI */";
  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(string("SelectSuccCount:").append(name),
                                                 oss.str())));
  conf->hookUp(pushSuccessorCountIn, pushSuccessorCountInPort, selectS, 0);

  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("Slot:") + name)));
  conf->hookUp(selectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullEvictOut, pullEvictOutPort);
}

/** rule SR3 maxSuccessorDist@NI(NI,max<D>) :- successor@NI(NI,S,SI),
	node@NI(NI,N), D = f_dist(N,S), evictSuccessor@NI(NI).
*/
void ruleSR3(string name,
             Plumber::ConfigurationPtr conf,
             TablePtr nodeTable,
             TablePtr successorTable,
             ElementSpecPtr pushEvictSuccessorIn,
             int pushEvictSuccessorInPort,
             ElementSpecPtr pullMSDOut,
             int pullMSDOutPort)
{
  // Join evictSuccessor with node
  ElementSpecPtr matchEvictSuccessorIntoNodeS =
    conf->addElement(ElementPtr(new UniqueLookup(string("evictSuccessorInNode:") + name,
                                                    nodeTable,
                                                    1, // Match EvictSuccessor.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));

  // Link it to the evictSuccessor coming in. Pushes match already
  conf->hookUp(pushEvictSuccessorIn, pushEvictSuccessorInPort, matchEvictSuccessorIntoNodeS, 0);



  // Produce intermediate ephemeral result
  // res1(NI, N) from
  // <<evictSuccessor NI><node NI N>>
  ElementSpecPtr makeRes1S =
    conf->addElement(ElementPtr(new PelTransform(string("FlattenRes1:").append(name),
                                                    "\"Res1\" pop \
                                                     $0 1 field pop /* output evictSuccessor.NI */ \
                                                     $1 2 field pop /* output node.N */")));
  conf->hookUp(matchEvictSuccessorIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  // Run aggregation over successor table
  // Agg bestSuccessorDist
  // NI, max<D>
  // (res1.NI == successor.NI,
  //  D = distance(N, S))
  ElementSpecPtr findMaxInSuccessorS =
    conf->addElement(ElementPtr(new PelScan(string("bestSuccessorDist:") + name,
                                               successorTable, 1,
                                               string("$1 /* res1.NI */ \
                                                    $2 /* NI res1.N */ \
                                                    0 ->u32 ->id /* NI N mindist */"),
                                               string("2 peek /* NI */ \
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
                                               string("\"maxSuccessorDist\" pop /* output name */\
                                                    $1 pop /* output NI */\
                                                    pop /* output maxDistance */\
                                                    drop drop /* empty the stack */"))));
  // Res1 must be pushed to second join
  ElementSpecPtr pushRes1S =
    conf->addElement(ElementPtr(new TimedPullPush(string("PushRes1:") + name, 0)));



  // Link the join to the aggregation
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, findMaxInSuccessorS, 0);
  conf->hookUp(findMaxInSuccessorS, 0, pullMSDOut, pullMSDOutPort);
}

/** SR4: maxSuccessor(NI, S, SI) :- successor(NI, S, SI),
    maxSuccessorDist(NI, D), D=dist(N, S)
*/
void ruleSR4(string name,
             Plumber::ConfigurationPtr conf,
             TablePtr nodeTable,
             TablePtr successorTable,
             ElementSpecPtr pushMSDIn,
             int pushMSDInPort)
{
  // Join with node
  ElementSpecPtr matchMSDIntoNodeS =
    conf->addElement(ElementPtr(new UniqueLookup(string("MSDInNode:") + name,
                                                    nodeTable,
                                                    1, // Match bSD.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));

  // Link it to the MSD coming in. Pushes match already
  conf->hookUp(pushMSDIn, pushMSDInPort, matchMSDIntoNodeS, 0);


  // Produce intermediate ephemeral result
  // res1(NI, N, D) from
  // <<MSD NI D><node NI N>>
  ElementSpecPtr makeRes1S =
    conf->addElement(ElementPtr(new PelTransform(string("FlattenRes1:").append(name),
                                                    "\"Res1\" pop \
                                                     $0 1 field pop /* output MSD.NI */ \
                                                     $1 2 field pop /* output node.N */ \
                                                     $0 2 field pop /* output MSD.D */ \
                                                     ")));
  conf->hookUp(matchMSDIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  // Join with successor table
  ElementSpecPtr matchRes1IntoSuccessorS =
    conf->addElement(ElementPtr(new MultLookup(string("Res1InSuccessor:") + name,
                                                  successorTable,
                                                  1, // Match res1.NI
                                                  1 // with successor.NI
                                                  )));
  ElementSpecPtr noNull2S = conf->addElement(ElementPtr(new NoNullField(string("NoNull2:") + name, 1)));

  // Res1 must be pushed to second join
  ElementSpecPtr pushRes1S =
    conf->addElement(ElementPtr(new TimedPullPush(string("PushRes1:") + name, 0)));
  // Link the two joins together
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, matchRes1IntoSuccessorS, 0);

  // Select out the outcomes that match the select clause
  // (res1.D == distance(res1.N, successor.S))
  // from <<Res1 NI N D><Successor NI S SI>>
  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(string("SelectRes1:").append(name),
                                                    "$0 2 field /* res1.N */\
                                                     $1 2 field /* D successor.S */\
                                                     distance --id /* dist(N,S) */\
                                                     $0 3 field /* dist(N,S) res1.D */\
                                                     ==id /* (dist==D) */\
                                                     not ifstop /* drop if not equal */\
                                                     $0 pop $1 pop /* pass through */")));
  conf->hookUp(matchRes1IntoSuccessorS, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, selectS, 0);

  // Turn into res2
  // from <<Res1...><Successor...>>
  ElementSpecPtr makeMSS =
    conf->addElement(ElementPtr(new PelTransform(string("FlattenRes2:").append(name),
                                                    "$1 unbox drop /* replace with Res2 */\
                                                     \"Res2\" pop /* new literal */\
                                                     pop pop pop /* Remaining fields */")));
  conf->hookUp(selectS, 0, makeMSS, 0);

  // And send it for deletion
  ElementSpecPtr deleteSuccessorS =
    conf->addElement(ElementPtr(new Delete(string("DeleteSuccessor:") + name,
                                              successorTable,
                                              2,
                                              2)));
  ElementSpecPtr pushRes2S =
    conf->addElement(ElementPtr(new TimedPullPush(string("PushRes2:") + name, 0)));
  // Link the two joins together
  conf->hookUp(makeMSS, 0, pushRes2S, 0);
  conf->hookUp(pushRes2S, 0, deleteSuccessorS, 0);
}


/** rule S0 stabilize@NI(NI, E) :- periodic@NI(TTL * 0.5), E=f_rand(),
    NI=ni. */
void ruleS0(string name,
            Plumber::ConfigurationPtr conf,
            string localAddress,
            double fingerTTL,
            ElementSpecPtr pullStabilizeOut,
            int pullStabilizeOutPort)
{
  // My stabilize tuple
  TuplePtr stabilizeTuple = Tuple::mk();
  stabilizeTuple->append(Val_Str::mk("stabilizeEvent"));
  stabilizeTuple->append(Val_Str::mk(localAddress));
  stabilizeTuple->freeze();

  ElementSpecPtr sourceS =
    conf->addElement(ElementPtr(new TupleSource(string("StabilizeSource:") + name,
                                                   stabilizeTuple)));
  
  // The timed pusher
  ElementSpecPtr pushS =
    conf->addElement(ElementPtr(new TimedPullPush(string("StabilizePush:") + name,
                                                     fingerTTL * 0.5)));
  
  // And a slot from which to pull
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("StabilizeSlot:") + name)));

  // Link everything
  conf->hookUp(sourceS, 0, pushS, 0);
  conf->hookUp(pushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullStabilizeOut, pullStabilizeOutPort);
}


/** rule S0a stabilize@NI(NI, E) :- stabilizeEvent@NI(NITTL * 0.5),
    E=f_rand(), NI=ni.
*/
void ruleS0a(string name,
             Plumber::ConfigurationPtr conf,
             ElementSpecPtr pushStabilizeEventIn,
             int pushStabilizeEventInPort,
             ElementSpecPtr pullStabilizeOut,
             int pullStabilizeOutPort)
{
  // Project onto stabilize(NI, E)
  // from
  // stabilizeEvent(NI)
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(string("project:").append(name),
                                                    "\"stabilize\" pop \
                                                     $1 pop /* out sE.NI */\
                                                     $1 \":\" strcat rand strcat pop /* out random */\
                                                     ")));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("Slot:") + name)));
  conf->hookUp(pushStabilizeEventIn, pushStabilizeEventInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullStabilizeOut, pullStabilizeOutPort);
}


/** rule S0b stabilizeRecord@NI(NI, E) :- stabilize@NI(NI, E).
 */
void ruleS0b(string name,
             Plumber::ConfigurationPtr conf,
             ElementSpecPtr pushStabilizeIn,
             int pushStabilizeInPort,
             ElementSpecPtr pullStabilizeRecordOut,
             int pullStabilizeRecordOutPort)
{
  // Project onto stabilizeRecord(NI, E)
  // from
  // stabilize(NI, E)
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(string("project:").append(name),
                                                    "\"stabilizeRecord\" pop \
                                                     $1 pop /* out s.NI */\
                                                     $2 pop /* out s.E */\
                                                     ")));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("Slot:") + name)));
  conf->hookUp(pushStabilizeIn, pushStabilizeInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullStabilizeRecordOut, pullStabilizeRecordOutPort);
}

/**
   rule S1 stabilizeRequest@SI(SI,NI,E) :- stabilize@NI(NI,E),
   bestSuccessor@NI(NI,S,SI),
 */
void ruleS1(string name,
            Plumber::ConfigurationPtr conf,
            TablePtr bestSuccessorTable,
            ElementSpecPtr pushStabilizeIn,
            int pushStabilizeInPort,
            ElementSpecPtr pullStabilizeRequestOut,
            int pullStabilizeRequestOutPort)
{
  // Join with best successor
  ElementSpecPtr joinS =
    conf->addElement(ElementPtr(new UniqueLookup(string("StabilizeInBestSuccessor:") + name,
                                                    bestSuccessorTable,
                                                    1, // Match stabilize.NI
                                                    1 // with bestSuccessor.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));
  // Link it to the stabilize coming in. Pushes match already
  conf->hookUp(pushStabilizeIn, pushStabilizeInPort, joinS, 0);
  conf->hookUp(joinS, 0, noNullS, 0);



  // Produce result
  // stabilizeRequest(SI, NI, E) from
  // stabilize(NI, E), bestSuccesssor(NI,S, SI)
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(string("Project:").append(name),
                                                    "\"stabilizeRequest\" pop \
                                                     $1 3 field pop /* SI */\
                                                     $0 1 field pop /* NI */\
                                                     $0 2 field pop /* E */\
                                                     ")));
  conf->hookUp(noNullS, 0, projectS, 0);
  conf->hookUp(projectS, 0, pullStabilizeRequestOut, pullStabilizeRequestOutPort);
}

/** 
    rule S2 sendPredecessor@PI1(PI1,P,PI,E) :-
    stabilizeRequest@NI(NI,PI1,E), predecessor@NI(NI,P,PI), PI != null.
*/
void ruleS2(string name,
            Plumber::ConfigurationPtr conf,
            TablePtr predecessorTable,
            ElementSpecPtr pushStabilizeRequestIn,
            int pushStabilizeRequestInPort,
            ElementSpecPtr pullSendPredecessorOut,
            int pullSendPredecessorOutPort)
{
  // StabilizeRequest stabilizeRequest with landmark table
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new UniqueLookup(string("stabilizeRequestIntoPredecessor:") + name,
                                                    predecessorTable,
                                                    1, // Match stabilizeRequest.NI
                                                    1 // with predecessor.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));

  conf->hookUp(pushStabilizeRequestIn, pushStabilizeRequestInPort, join1S, 0);
  conf->hookUp(join1S, 0, noNullS, 0);


  // Select predecessor.PI != null
  // stabilizeRequest(NI, PI1, E),predecessor(NI, P, PI)>
  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(string("select:") + name,
                                                    "$1 3 field /* predecessor.PI */\
                                                     \"-\" ==s ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     ")));
  conf->hookUp(noNullS, 0, selectS, 0);


  // Now project onto the result
  // sendPredecessor(PI1, P, PI, E)
  // from
  // stabilizeRequest(NI, PI1, E),predecessor(NI, P, PI)>
  ElementSpecPtr project1S =
    conf->addElement(ElementPtr(new PelTransform(string("project:").append(name),
                                                    "\"sendPredecessor\" pop \
                                                     $0 2 field pop /* out stabilizeRequest.PI1 */\
                                                     $1 2 field pop /* out predecessor.P */\
                                                     $1 3 field pop /* out predecessor.PI */\
                                                     $0 3 field pop /* out stabilizeRequest.E */\
                                                     ")));
  conf->hookUp(selectS, 0, project1S, 0);
  conf->hookUp(project1S, 0, pullSendPredecessorOut, pullSendPredecessorOutPort);
}


/** 
    rule S3 successor@NI(NI,P,PI) :- node(NI,N),
    sendPredecessor@NI(NI,P,PI,E), bestSuccessor@NI(NI,S,SI), P in
    (N,S), stabilizeRecord@NI(NI, E).
*/
void ruleS3(string name,
            Plumber::ConfigurationPtr conf,
            TablePtr stabilizeRecordTable,
            TablePtr nodeTable,
            TablePtr bestSuccessorTable,
            ElementSpecPtr pushSendPredecessorIn,
            int pushSendPredecessorInPort,
            ElementSpecPtr pullSuccessorOut,
            int pullSuccessorOutPort)
{
  // SendPredecessor sendPredecessor with landmark table
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new UniqueLookup(string("sendPredecessorIntoNode:") + name,
                                                    nodeTable,
                                                    1, // Match sendPredecessor.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));
  conf->hookUp(pushSendPredecessorIn, pushSendPredecessorInPort, join1S, 0);
  conf->hookUp(join1S, 0, noNullS, 0);

  // from
  // sendPredecessor(NI, P, PI, E), node(NI, N)>
  ElementSpecPtr project1S =
    conf->addElement(ElementPtr(new PelTransform(string("Project:").append(name),
                                                    "\"res1\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field pop /* out P */\
                                                     $0 3 field pop /* out PI */\
                                                     $0 4 field pop /* out E */\
                                                     $1 2 field pop /* out node.N */\
                                                     ")));
  conf->hookUp(noNullS, 0, project1S, 0);

  
  // Now join res1 with stabilize record
  ElementSpecPtr pushRes1S =
    conf->addElement(ElementPtr(new TimedPullPush(string("PushRes1:") + name, 0)));
  ElementSpecPtr join2S =
    conf->addElement(ElementPtr(new UniqueLookup(string("res1IntoStabilizeRecord:") + name,
                                                    stabilizeRecordTable,
                                                    1, // Match res1.NI
                                                    1 // with stabilizeRecord.NI
                                                    )));
  ElementSpecPtr noNull2S = conf->addElement(ElementPtr(new NoNullField(string("NoNull2:") + name, 1)));
  conf->hookUp(project1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, join2S, 0);
  conf->hookUp(join2S, 0, noNull2S, 0);
  
  // Select res1.E == stabilizeRecord.E
  // from
  // res1(NI, P, PI, E, N), stabilizeRecord(NI, E)
  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(string("select:") + name,
                                                    "$0 4 field /* res1.E */\
                                                     $1 2 field /* res1.E sR.E */\
                                                     ==s not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     ")));
  conf->hookUp(noNull2S, 0, selectS, 0);

  // Project back to res1.
  // from
  // res1()..., stabilizeRecord(...)
  ElementSpecPtr project2S =
    conf->addElement(ElementPtr(new PelTransform(string("Project2:").append(name),
                                                    "$0 unboxPop\
                                                     ")));
  conf->hookUp(selectS, 0, project2S, 0);


  // Finally join res1 with bestSuccessorTable
  ElementSpecPtr push2S =
    conf->addElement(ElementPtr(new TimedPullPush(string("Push2:") + name, 0)));
  ElementSpecPtr join3S =
    conf->addElement(ElementPtr(new UniqueLookup(string("res1IntoBestSuccessor:") + name,
                                                    bestSuccessorTable,
                                                    1, // Match res1.NI
                                                    1 // with bestSuccessor.NI
                                                    )));
  ElementSpecPtr noNull3S = conf->addElement(ElementPtr(new NoNullField(string("NoNull3:") + name, 1)));
  conf->hookUp(project2S, 0, push2S, 0);
  conf->hookUp(push2S, 0, join3S, 0);
  conf->hookUp(join3S, 0, noNull3S, 0);
  

  // Select P in (N, S)
  // from
  // res1(NI, P, PI, E, N), bestSuccessor(NI, S, SI)
  ElementSpecPtr select2S =
    conf->addElement(ElementPtr(new PelTransform(string("select2:") + name,
                                                    "$0 2 field /* P */\
                                                     $0 5 field /* P N */\
                                                     $1 2 field /* P N S */\
                                                     ()id not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     ")));
  conf->hookUp(noNull3S, 0, select2S, 0);


  // Finally project onto the result
  // successor(NI, P, PI)
  // from
  // res1(NI, P, PI, E, N), bestSuccessor(NI, S, SI)
  ElementSpecPtr project3S =
    conf->addElement(ElementPtr(new PelTransform(string("project3:").append(name),
                                                    "\"successor\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field pop /* out P */\
                                                     $0 3 field pop /* out PI */\
                                                     ")));
  conf->hookUp(select2S, 0, project3S, 0);
  conf->hookUp(project3S, 0, pullSuccessorOut, pullSuccessorOutPort);
}


/**
   rule S4 sendSuccessors@SI(SI,NI,E) :- stabilize@NI(NI,E),
   successor@NI(NI,S,SI).
 */
void ruleS4(string name,
            Plumber::ConfigurationPtr conf,
            TablePtr successorTable,
            ElementSpecPtr pushStabilizeIn,
            int pushStabilizeInPort,
            ElementSpecPtr pullSendSuccessorsOut,
            int pullSendSuccessorsOutPort)
{
  // Join with successor
  ElementSpecPtr joinS =
    conf->addElement(ElementPtr(new MultLookup(string("StabilizeInSuccessor:") + name,
                                                  successorTable,
                                                  1, // Match stabilize.NI
                                                  1 // with successor.NI
                                                  )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));
  // Link it to the stabilize coming in. Pushes match already
  conf->hookUp(pushStabilizeIn, pushStabilizeInPort, joinS, 0);
  conf->hookUp(joinS, 0, noNullS, 0);



  // Produce result
  // sendSuccessors(SI, NI, E) from
  // stabilize(NI, E), successsor(NI,S, SI)
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(string("Project:").append(name),
                                                    "\"sendSuccessors\" pop \
                                                     $1 3 field pop /* SI */\
                                                     $0 1 field pop /* NI */\
                                                     $0 2 field pop /* E */\
                                                     ")));
  conf->hookUp(noNullS, 0, projectS, 0);
  conf->hookUp(projectS, 0, pullSendSuccessorsOut, pullSendSuccessorsOutPort);
}



/**
   rule S5 returnSuccessor@PI(PI,S,SI,E) :- sendSuccessors@NI(NI,PI,E),
   successor@NI(NI,S,SI).
 */
void ruleS5(string name,
            Plumber::ConfigurationPtr conf,
            TablePtr successorTable,
            ElementSpecPtr pushSendSuccessorsIn,
            int pushSendSuccessorsInPort,
            ElementSpecPtr pullReturnSuccessorOut,
            int pullReturnSuccessorOutPort)
{
  // Join with successor
  ElementSpecPtr joinS =
    conf->addElement(ElementPtr(new MultLookup(string("SendSuccessorsInSuccessor:") + name,
                                                  successorTable,
                                                  1, // Match sendSuccessors.NI
                                                  1 // with successor.NI
                                                  )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));
  // Link it to the sendSuccessors coming in. Pushes match already
  conf->hookUp(pushSendSuccessorsIn, pushSendSuccessorsInPort, joinS, 0);
  conf->hookUp(joinS, 0, noNullS, 0);



  // Produce result
  // returnSuccessor(PI, S, SI, E) from
  // sendSuccessors(NI,PI,E), successsor(NI,S, SI)
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(string("Project:").append(name),
                                                    "\"returnSuccessor\" pop \
                                                     $0 2 field pop /* PI */\
                                                     $1 2 field pop /* S */\
                                                     $1 3 field pop /* SI */\
                                                     $0 3 field pop /* E */\
                                                     ")));
  conf->hookUp(noNullS, 0, projectS, 0);
  conf->hookUp(projectS, 0, pullReturnSuccessorOut, pullReturnSuccessorOutPort);
}


/** 
    rule S5a successor@NI(NI, S, SI) :- returnSuccessor@NI(NI,S,SI,E),
    stabilizeRecord@NI(NI, E).
*/
void ruleS5a(string name,
             Plumber::ConfigurationPtr conf,
             TablePtr stabilizeRecordTable,
             ElementSpecPtr pushReturnSuccessorIn,
             int pushReturnSuccessorInPort,
             ElementSpecPtr pullSuccessorOut,
             int pullSuccessorOutPort)
{
  // returnSuccessor with stabilizerecord table
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new UniqueLookup(string("returnSuccessorIntoStabilizeRecord:") + name,
                                                    stabilizeRecordTable,
                                                    1, // Match returnSuccessor.NI
                                                    1 // with stabilizeRecord.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));
  conf->hookUp(pushReturnSuccessorIn, pushReturnSuccessorInPort, join1S, 0);
  conf->hookUp(join1S, 0, noNullS, 0);

  // Select rS.E == stabilizeRecord.E
  // from
  // rS(NI, S, SI, E), stabilizeRecord(NI, E)
  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(string("select:") + name,
                                                    "$0 4 field /* rS.E */\
                                                     $1 2 field /* rS1.E sR.E */\
                                                     ==s not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     ")));
  conf->hookUp(noNullS, 0, selectS, 0);

  // Finally project onto the result
  // successor(NI, S, SI)
  // from
  // rS(NI, S, SI, E), stabilizeRecord(NI, E)
  ElementSpecPtr project3S =
    conf->addElement(ElementPtr(new PelTransform(string("project3:").append(name),
                                                    "\"successor\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field pop /* out S */\
                                                     $0 3 field pop /* out SI */\
                                                     ")));
  conf->hookUp(selectS, 0, project3S, 0);
  conf->hookUp(project3S, 0, pullSuccessorOut, pullSuccessorOutPort);
}


/** rule S6a notify@NI(NI) :- periodic@NI(TTL * 0.5), NI=ni. */
void ruleS6a(string name,
             Plumber::ConfigurationPtr conf,
             string localAddress,
             double fingerTTL,
             ElementSpecPtr pullNotifyOut,
             int pullNotifyOutPort)
{
  // My notify tuple
  TuplePtr notifyTuple = Tuple::mk();
  notifyTuple->append(Val_Str::mk("notify"));
  notifyTuple->append(Val_Str::mk(localAddress));
  notifyTuple->freeze();

  ElementSpecPtr sourceS =
    conf->addElement(ElementPtr(new TupleSource(string("NotifySource:") + name,
                                                   notifyTuple)));
  
  // The timed pusher
  ElementSpecPtr pushS =
    conf->addElement(ElementPtr(new TimedPullPush(string("NotifyPush:") + name,
                                                     fingerTTL * 0.5)));
  
  // And a slot from which to pull
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("NotifySlot:") + name)));

  // Link everything
  conf->hookUp(sourceS, 0, pushS, 0);
  conf->hookUp(pushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullNotifyOut, pullNotifyOutPort);
}


/** 
    rule S6 notifyPredecessor@SI(SI,N,NI) :- notify@NI(NI),
    node@NI(NI,N), successor@NI(NI,S,SI).
*/
void ruleS6(string name,
            Plumber::ConfigurationPtr conf,
            TablePtr nodeTable,
            TablePtr successorTable,
            ElementSpecPtr pushNotifyIn,
            int pushNotifyInPort,
            ElementSpecPtr pullNotifyPredecessorOut,
            int pullNotifyPredecessorOutPort)
{
  // Join notify with node
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new UniqueLookup(string("notifyIntoNode:") + name,
                                                    nodeTable,
                                                    1, // Match notify.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));
  conf->hookUp(pushNotifyIn, pushNotifyInPort, join1S, 0);
  conf->hookUp(join1S, 0, noNullS, 0);

  // Project res1(NI, N)
  // from
  // notify(NI), node(NI, N)>
  ElementSpecPtr project1S =
    conf->addElement(ElementPtr(new PelTransform(string("Project:").append(name),
                                                    "\"res1\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $1 2 field pop /* out node.N */\
                                                     ")));
  conf->hookUp(noNullS, 0, project1S, 0);

  
  // Now join res1 with successor
  ElementSpecPtr push1S =
    conf->addElement(ElementPtr(new TimedPullPush(string("Push1:") + name, 0)));
  ElementSpecPtr join2S =
    conf->addElement(ElementPtr(new MultLookup(string("res1IntoSuccessor:") + name,
                                                    successorTable,
                                                    1, // Match res1.NI
                                                    1 // with successor.NI
                                                    )));
  ElementSpecPtr noNull2S = conf->addElement(ElementPtr(new NoNullField(string("NoNull2:") + name, 1)));
  conf->hookUp(project1S, 0, push1S, 0);
  conf->hookUp(push1S, 0, join2S, 0);
  conf->hookUp(join2S, 0, noNull2S, 0);
  
  // Project to notifyPredecessor(SI, N, NI)
  // from
  // res1(NI, N), successor(NI, S, SI)
  ElementSpecPtr project2S =
    conf->addElement(ElementPtr(new PelTransform(string("Project2:").append(name),
                                                    "\"notifyPredecessor\" pop \
                                                     $1 3 field pop /* out SI */\
                                                     $0 2 field pop /* out N */\
                                                     $0 1 field pop /* out NI */\
                                                     ")));
  conf->hookUp(noNull2S, 0, project2S, 0);

  conf->hookUp(project2S, 0, pullNotifyPredecessorOut, pullNotifyPredecessorOutPort);
}


/** 
    rule S7 predecessor@NI(NI,P,PI) :- node@NI(NI,N),
    notifyPredecessor@NI(NI,P,PI), predecessor@NI(NI,P1,PI1), ((PI1 ==
    "") || (P in (P1, N))).
*/
void ruleS7(string name,
            Plumber::ConfigurationPtr conf,
            TablePtr nodeTable,
            TablePtr predecessorTable,
            ElementSpecPtr pushNotifyPredecessorIn,
            int pushNotifyPredecessorInPort,
            ElementSpecPtr pullPredecessorOut,
            int pullPredecessorOutPort)
{
  // Join notifyPredecessor with table
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new UniqueLookup(string("notifyPredecessorIntoNode:") + name,
                                                    nodeTable,
                                                    1, // Match notifyPredecessor.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(string("NoNull:") + name, 1)));
  conf->hookUp(pushNotifyPredecessorIn, pushNotifyPredecessorInPort, join1S, 0);
  conf->hookUp(join1S, 0, noNullS, 0);

  // Project res1(NI, P, PI, N)
  // from
  // notifyPredecessor(NI, P, PI), node(NI, N)>
  ElementSpecPtr project1S =
    conf->addElement(ElementPtr(new PelTransform(string("Project:").append(name),
                                                    "\"res1\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field pop /* out P */\
                                                     $0 3 field pop /* out PI */\
                                                     $1 2 field pop /* out node.N */\
                                                     ")));
  conf->hookUp(noNullS, 0, project1S, 0);

  
  // Finally join res1 with predecessorTable
  ElementSpecPtr push2S =
    conf->addElement(ElementPtr(new TimedPullPush(string("Push2:") + name, 0)));
  ElementSpecPtr join3S =
    conf->addElement(ElementPtr(new UniqueLookup(string("res1IntoPredecessor:") + name,
                                                    predecessorTable,
                                                    1, // Match res1.NI
                                                    1 // with predecessor.NI
                                                    )));
  ElementSpecPtr noNull3S = conf->addElement(ElementPtr(new NoNullField(string("NoNull3:") + name, 1)));
  conf->hookUp(project1S, 0, push2S, 0);
  conf->hookUp(push2S, 0, join3S, 0);
  conf->hookUp(join3S, 0, noNull3S, 0);
  

  // Select (PI1 == "" || (P in (P1, N))).
  // from
  // res1(NI, P, PI, N), predecessor(NI, P1, PI1)
  ElementSpecPtr select2S =
    conf->addElement(ElementPtr(new PelTransform(string("select2:") + name,
                                                    "$0 2 field /* P */\
                                                     $1 2 field /* P P1 */\
                                                     $0 4 field /* P P1 N */\
                                                     ()id /* (P in(P1,N)) */\
                                                     $1 3 field /* (P in(P1,N)) PI1 */\
                                                     \"-\" ==s or not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     ")));
  conf->hookUp(noNull3S, 0, select2S, 0);


  // Finally project onto the result
  // predecessor(NI, P, PI)
  // from
  // res1(NI, P, PI, N), predecessor(NI, S, SI)
  ElementSpecPtr project3S =
    conf->addElement(ElementPtr(new PelTransform(string("project3:").append(name),
                                                    "\"predecessor\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field pop /* out P */\
                                                     $0 3 field pop /* out PI */\
                                                     ")));
  conf->hookUp(select2S, 0, project3S, 0);
  conf->hookUp(project3S, 0, pullPredecessorOut, pullPredecessorOutPort);
}



/*
 * End of file 
 */
#endif /* __RING_H_ */
