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

#ifndef __CHORD_H__
#define __CHORD_H__


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


static const int SUCCESSORSIZE = 4;
static const double FINGERTTL = 10;
static const int SUCCEXPIRATION = 2;
static const int SUCCREFRESH = 1;
static const int FINGERSIZE = ID::WORDS * 32;
static const int QUEUE_LENGTH = 1000;

/**
 *rule L1 lookupResults@R(R,K,S,SI,E) :- node@NI(NI,N),
 *lookup@NI(NI,K,R,E), bestSuccessor@NI(NI,S,SI), K in (N,S].
 */
void ruleL1(str name,
            Router::ConfigurationRef conf,
            TableRef nodeTable,
            TableRef bestSuccessorTable,
            ElementSpecRef pushLookupIn,
            int pushLookupInPort,
            ElementSpecRef pullLookupResultsOut,
            int pullLookupResultsOutPort)
{
  // Join with node
  ElementSpecRef matchLookupIntoNodeS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("LookupInNode:") << name,
                                                    nodeTable,
                                                    1, // Match lookup.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));
  // Link it to the lookup coming in. Pushes match already
  conf->hookUp(pushLookupIn, pushLookupInPort, matchLookupIntoNodeS, 0);



  // Produce intermediate ephemeral result
  // res1(NI, K, R, E, N) from
  // <<lookup NI K R E><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("MakeRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 unbox \
                                                     drop /* No need for lookup literal */ \
                                                     pop pop pop pop /* all lookup fields */ \
                                                     $1 2 field pop /* node.N */"));
  conf->hookUp(matchLookupIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  



  // Join with best successor table
  ElementSpecRef matchRes1IntoBestSuccS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("LookupInBestSucc:") << name,
                                                    bestSuccessorTable,
                                                    1, // Match res1.NI
                                                    1 // with bestSuccessor.NI
                                                    ));
  ElementSpecRef noNull2S = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull2:") << name, 1));
  // Res1 must be pushed to second join
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name,
                                                     0));
  // Link the two joins together
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, matchRes1IntoBestSuccS, 0);



  // Select res1.K in (res1.N, bestSuccessor.S]
  // from <<res1 NI, K, R, E, N><bestSuccessor NI S SI>>
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("SelectContainment:").cat(name),
                                                    "$0 2 field /* res1.K */\
                                                     $0 5 field /* res1.K res1.N */\
                                                     $1 2 field /* res1.K res1.N bS.S */\
                                                     (]id not ifstop /* empty */\
                                                     $0 pop $1 pop /* pass through if not dropped */"));
  conf->hookUp(matchRes1IntoBestSuccS, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, selectS, 0);
  

  // Project to create lookupResults(R, K, S, SI, E)
  // from <<res1 NI, K, R, E, N><bestSuccessor NI S SI>>
  ElementSpecRef makeLookupResultS =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenLookupResult:").cat(name),
                                                    "\"lookupResults\" pop \
                                                     $0 3 field pop /* output R */ \
                                                     $0 2 field pop /* output K */ \
                                                     $1 unbox drop drop pop pop /* output S SI */ \
                                                     $0 4 field pop /* output E */"));
  conf->hookUp(selectS, 0, makeLookupResultS, 0);
  conf->hookUp(makeLookupResultS, 0, pullLookupResultsOut, pullLookupResultsOutPort);
}

/** L2: bestLookupDistance@NI(NI,K,R,E,min<D>) :- lookup@NI(NI,K,R,E),
    node@NI(NI, N), finger@NI(NI,I,B,BI,ET), B in (N,K), D=f_dist(B,K)-1
*/
void ruleL2(str name,
            Router::ConfigurationRef conf,
            TableRef nodeTable,
            TableRef fingerTable,
            ElementSpecRef pushLookupIn,
            int pushLookupInPort,
            ElementSpecRef pullLookupDistanceOut,
            int pullLookupDistanceOutPort)
{
  // Join with node
  ElementSpecRef matchLookupIntoNodeS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("LookupInNode:") << name,
                                                    nodeTable,
                                                    1, // Match lookup.NI
                                                    1 // with node.NI
                                                    ));
  // Link it to the lookup coming in. Pushes match already
  conf->hookUp(pushLookupIn, pushLookupInPort, matchLookupIntoNodeS, 0);
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  // Produce intermediate ephemeral result
  // res1(NI, K, R, E, N) from
  // <<lookup NI K R E><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 unbox \
                                                     drop /* No need for lookup literal */ \
                                                     pop pop pop pop /* all lookup fields */ \
                                                     $1 2 field pop /* node.N */"));
  conf->hookUp(matchLookupIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  // Run aggregation over finger table with input
  // res1(NI, K, R, E, N) from
  ElementSpecRef findMinInFingerS =
    conf->addElement(New refcounted< PelScan >(str("bestLookupDistance:") << name,
                                               fingerTable, 1,
                                               str("$1 /* res1.NI */ \
                                                    $2 /* NI res1.K */ \
                                                    $5 /* NI K res1.N */ \
                                                    0 /* NI K res1.N found? */\
                                                    1 ->u32 ->id 0 ->u32 ->id distance /* NI K N found? maxdist */"),
                                               str("4 peek /* NI */ \
                                                    $1 /* finger.NI */ \
                                                    ==s not /* res1.NI != finger.NI */ \
                                                    ifstop /* empty */ \
                                                    $3 /* B */ \
                                                    3 peek /* B N */ \
                                                    5 peek /* B N K */ \
                                                    ()id  /* (B in (N,K)) */ \
                                                    not ifstop /* empty */ \
                                                    swap drop 1 swap /* found is now true */\
                                                    $3 4 peek distance --id dup /* dist(B,K) twice */ \
                                                    2 peek /* newDist newDist oldDist */ \
                                                    <id /* newDist (newDist<oldMin) */ \
                                                    swap /* (old>new?) newDist */ \
                                                    2 peek ifelse /* ((old>new) ? new : old) */ \
                                                    swap /* swap newMin in state where oldMin was */ \
                                                    drop /* only state remains */"),
                                               str("swap not ifstop\
                                                    \"bestLookupDistance\" pop \
                                                    $1 pop /* output NI */\
                                                    $2 pop /* output K */\
                                                    $3 pop /* output R */\
                                                    $4 pop /* output E */\
                                                    pop /* output minDist */\
                                                    drop drop drop /* empty the stack */")));
  // Res1 must be pushed to second join
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name,
                                                     0));
  
  // Link the join to the agg
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);

  ElementSpecRef noNull2S = conf->addElement(New refcounted< NoNull >(strbuf("NoNull2:") << name));

  conf->hookUp(pushRes1S, 0, findMinInFingerS, 0);

  conf->hookUp(findMinInFingerS, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, pullLookupDistanceOut, pullLookupDistanceOutPort);
}







/** rule L3 lookup@BI(max<BI>,K,R,E) :-
    bestLookupDistance@NI(NI,K,R,E,D), finger@NI(NI,I,B,BI),
    D=f_dist(B,K), node@NI(NI, N), B in (N, K).*/
void ruleL3(str name,
            Router::ConfigurationRef conf,
            TableRef nodeTable,
            TableRef fingerTable,
            ElementSpecRef pushBLDIn,
            int pushBLDInPort,
            ElementSpecRef pullLookupOut,
            int pullLookupOutPort)
{
  // Join with node
  ElementSpecRef matchBLDIntoNodeS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("BLDInNode:") << name,
                                                    nodeTable,
                                                    1, // Match lookup.NI
                                                    1 // with node.NI
                                                    ));
  // Link it to the BLD coming in. Pushes match already
  conf->hookUp(pushBLDIn, pushBLDInPort, matchBLDIntoNodeS, 0);
  ElementSpecRef noNull2S = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull2:") << name, 1));



  // Produce intermediate ephemeral result
  // res1(NI, K, R, E, D, N) from
  // <<BLD NI K R E D><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 unbox \
                                                     drop /* No need for BLD literal */ \
                                                     pop pop pop pop pop /* all BLD fields */ \
                                                     $1 2 field pop /* node.N */"));
  conf->hookUp(matchBLDIntoNodeS, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, makeRes1S, 0);
  

  // Select out the min-BI for all the fingers that satisfy
  // (finger.B in (res1.N, res1.K)) and (res1.D == distance(finger.B,
  // res1.K)-1) and (res1.NI == finger.NI)
  // from <Res1 NI K R E D N> input joined with
  // <Finger NI I B BI>>
  ElementSpecRef aggregateS =
    conf->addElement(New refcounted< PelScan >(str("tieBreaker:") << name,
                                               fingerTable, 1,
                                               str("$1 /* res1.NI */ \
                                                    $5 /* NI res1.D */\
                                                    $2 /* NI D res1.K */ \
                                                    $6 /* NI D K res1.N */ \
                                                    0 /* NI D K N found?*/ \
                                                    \"\" /* NI D K N found? maxString */"),
                                               str("$3 /* finger.B */\
                                                    3 peek /* B res1.N */\
                                                    5 peek /* B N res1.K */\
                                                    ()id /* B in (N,K) */\
                                                    $3 /* B in (N,K) finger.B */\
                                                    5 peek /* B in (N,K) B res1.K */\
                                                    distance --id /* B in (N,K) dist(B,K) */\
                                                    6 peek /* B in (N,K) dist(B,K) res1.D */\
                                                    ==id /* B in (N,K) (dist==D) */\
                                                    and /* [B in (N,K) and (dist==D)] */\
                                                    6 peek /* [first and] res1.NI */\
                                                    $1 /* [first and] res1.NI finger.NI */\
                                                    ==s and /* select clause */\
                                                    not ifstop /* done with select */\
                                                    swap drop 1 swap /* replace found? with true in state */\
                                                    $4 dup /* finger.BI finger.BI */\
                                                    2 peek /* finger.BI finger.BI oldMax */\
                                                    >s /* finger.BI (finger.BI>oldMax?) */\
                                                    swap /* (BI>oldMax?) finger.BI */ \
                                                    2 peek ifelse /* ((new>old) ? new : old) */ \
                                                    swap /* swap newMax in state where oldMax was */ \
                                                    drop /* only state remains */"),
                                               str("swap not ifstop /* return nothing if none found */\
                                                    \"lookup\" pop\
                                                    pop /* output maxBI */\
                                                    $2 pop /* output K */\
                                                    $3 pop /* output R */\
                                                    $4 pop /* output E */\
                                                    drop drop drop drop /* empty the stack */")));
  
  // Res1 must be pushed to aggregate
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name,
                                                     0));

  ElementSpecRef noNullS =
    conf->addElement(New refcounted< NoNull >(strbuf("NoNull:") << name));

  // Link the join to the aggregate
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, aggregateS, 0);



  
  conf->hookUp(aggregateS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, pullLookupOut, pullLookupOutPort);
}




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


/** 
    rule SU3 finger@NI(NI,0,S,SI) :- bestSuccessor@NI(NI,S,SI).
 */
void ruleSU3(str name,
             Router::ConfigurationRef conf,
             ElementSpecRef pushBestSuccessorIn,
             int pushBestSuccessorInPort,
             ElementSpecRef pullFingerOut,
             int pullFingerOutPort)
{
  // Project onto finger(NI, 0, B, BI)
  // from
  // bestSuccessor(NI, S, SI)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"finger\" pop \
                                                     $1 pop /* out bS.NI */\
                                                     0 pop /* out 0 */\
                                                     $2 pop /* out bS.S */\
                                                     $3 pop /* out bS.SI */\
                                                     "));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:") << name));
  conf->hookUp(pushBestSuccessorIn, pushBestSuccessorInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullFingerOut, pullFingerOutPort);
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




/** rule F1 fixFinger@NI(ni) :- periodic@NI(finger.TTL*0.5). */
void ruleF1(str name,
            Router::ConfigurationRef conf,
            str localAddress,
            double fingerTTL,
            ElementSpecRef pullFixFingerOut,
            int pullFixFingerOutPort)
{
  // My fix finger tuple
  TupleRef fixFingerTuple = Tuple::mk();
  fixFingerTuple->append(Val_Str::mk("fixFinger"));
  fixFingerTuple->append(Val_Str::mk(localAddress));
  fixFingerTuple->freeze();

  ElementSpecRef sourceS =
    conf->addElement(New refcounted< TupleSource >(str("FFSource:") << name,
                                                   fixFingerTuple));
  
  // The timed pusher
  ElementSpecRef pushS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("FFPush:") << name,
                                                     fingerTTL * 0.5));

  // And a slot from which to pull
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("FFSlot:") << name));

  // Link everything
  conf->hookUp(sourceS, 0, pushS, 0);
  conf->hookUp(pushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullFixFingerOut, pullFixFingerOutPort);
}




/** rule F2 nextFingerFix@NI(ni, 0). */
void ruleF2(str name,
            Router::ConfigurationRef conf,
            str localAddress,
            ElementSpecRef pullNextFingerFixOut,
            int pullNextFingerFixOutPort)
{
  // My next finger fix tuple
  TupleRef nextFingerFixTuple = Tuple::mk();
  nextFingerFixTuple->append(Val_Str::mk("nextFingerFix"));
  nextFingerFixTuple->append(Val_Str::mk(localAddress));
  nextFingerFixTuple->append(Val_Int32::mk(0));
  nextFingerFixTuple->freeze();

  ElementSpecRef sourceS =
    conf->addElement(New refcounted< TupleSource >(str("NFFSource:") << name,
                                                   nextFingerFixTuple));
  
  // The once pusher
  ElementSpecRef onceS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("NFFPush:") << name,
                                                     0, // run immediately
                                                     1 // run once
                                                     ));

  // And a slot from which to pull
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("NFFSlot:") << name));

  // Link everything
  conf->hookUp(sourceS, 0, onceS, 0);
  conf->hookUp(onceS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullNextFingerFixOut, pullNextFingerFixOutPort);
}


/** rule F3 fingerLookup@NI(NI, E, I) :- fixFinger@NI(NI), E = random(),
    nextFingerFix@NI(NI, I).
*/
void ruleF3(str name,
            Router::ConfigurationRef conf,
            TableRef nextFingerFixTable,
            ElementSpecRef pushFixFingerIn,
            int pushFixFingerInPort,
            ElementSpecRef pullFingerLookupOut,
            int pullFingerLookupOutPort)
{
  // Join fixFinger with nextFingerFix
  ElementSpecRef join1S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("fixFingerIntoNextFingerFix:") << name,
                                                    nextFingerFixTable,
                                                    1, // Match fixFinger.NI
                                                    1 // with nextFingerFix.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  // Link it to the evictSuccessor coming in. Pushes match already
  conf->hookUp(pushFixFingerIn, pushFixFingerInPort, join1S, 0);

  // Produce finger lookup
  // fingerLookup(NI, E, I) from
  // <<fixFinger NI><nextFingerFix NI I>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenFingerLookup:").cat(name),
                                                    "\"fingerLookup\" pop \
                                                     $0 1 field pop /* output fixFinger.NI */ \
                                                     $0 1 field \":\" strcat rand strcat pop /* output NI|rand */ \
                                                     $1 2 field pop /* output nextFingerFix.I */"));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  conf->hookUp(makeRes1S, 0, pullFingerLookupOut, pullFingerLookupOutPort);
}


/** rule F4 lookup@NI(NI, K, NI, E) :- fingerLookup@NI(NI, E, I),
    node(NI, N), K = N + 1 << I.
*/
void ruleF4(str name,
            Router::ConfigurationRef conf,
            TableRef nodeTable,
            ElementSpecRef pushFingerLookupIn,
            int pushFingerLookupInPort,
            ElementSpecRef pullLookupOut,
            int pullLookupOutPort)
{
  // Join fingerLookup with node
  ElementSpecRef join1S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("fingerLookupIntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match fingerLookup.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  conf->hookUp(pushFingerLookupIn, pushFingerLookupInPort, join1S, 0);



  // Produce lookup
  // lookup(NI, K, NI, E) from
  // <<fingerLookup NI E I><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("lookup:").cat(name),
                                                    "\"lookup\" pop \
                                                     $0 1 field pop /* output fixFinger.NI */ \
                                                     $1 2 field /* node.N */\
                                                     1 ->u32 ->id /* N (1 as ID) */\
                                                     $0 3 field /* N 1 fingerLookup.I */\
                                                     <<id /* N 2^(I) */\
                                                     +id pop /* output (N+2^I) */\
                                                     $0 1 field pop /* output fixFinger.NI again */ \
                                                     $0 2 field pop /* output fingerLookup.E */"));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  conf->hookUp(makeRes1S, 0, pullLookupOut, pullLookupOutPort);
}



/** rule F5 eagerFinger@NI(NI, I, B, BI) :- fingerLookup@NI(NI, E,
    I), lookupResults@NI(NI, K, B, BI, E).
*/
void ruleF5(str name,
            Router::ConfigurationRef conf,
            TableRef fingerLookupTable,
            ElementSpecRef pushLookupResultsIn,
            int pushLookupResultsInPort,
            ElementSpecRef pullEagerFingerOut,
            int pullEagerFingerOutPort)
{
  // Join lookupResults with fingerLookup table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< MultLookup >(strbuf("lookupResultsIntoFingerLookup:") << name,
                                                  fingerLookupTable,
                                                  1, // Match lookupResults.NI
                                                  1 // with fingerLookup.NI
                                                  ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  conf->hookUp(pushLookupResultsIn, pushLookupResultsInPort, join1S, 0);

  // Select fingerLookup.E == lookupResults.E
  // <<lookupResults NI K B BI E><fingerLookup NI E I>>
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("select:") << name,
                                                    "$0 5 field /* lookupResults.E */\
                                                     $1 2 field /* lR.E fingerLookup.E */\
                                                     ==s not /* lR.E!=fL.E? */\
                                                     ifstop /* empty */\
                                                     $0 pop $1 pop /* pass through */\
                                                     "));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, selectS, 0);



  // Produce eagerFinger
  // eagerFinger(NI, I, B, BI) from
  // <<lookupResults NI K B BI E><fingerLookup NI E I>>
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"eagerFinger\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $1 3 field pop /* out I */\
                                                     $0 3 field pop /* out B */\
                                                     $0 4 field pop /* out BI */\
                                                     "));
  conf->hookUp(selectS, 0, projectS, 0);

  conf->hookUp(projectS, 0, pullEagerFingerOut, pullEagerFingerOutPort);
}


/** rule F6 finger@NI(NI, I, B, BI) :- eagerFinger@NI(NI, I, B, BI). */
void ruleF6(str name,
            Router::ConfigurationRef conf,
            ElementSpecRef pushEagerFingerIn,
            int pushEagerFingerInPort,
            ElementSpecRef pullFingerOut,
            int pullFingerOutPort)
{
  // Project onto finger(NI, I, B, BI)
  // from
  // eagerFinger(NI, I, B, BI)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"finger\" pop \
                                                     $1 pop /* out eF.NI */\
                                                     $2 pop /* out eF.I */\
                                                     $3 pop /* out eF.B */\
                                                     $4 pop /* out eF.BI */\
                                                     "));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:") << name));
  conf->hookUp(pushEagerFingerIn, pushEagerFingerInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullFingerOut, pullFingerOutPort);
}


/** rule F7 eagerFinger@NI(NI, I, B, BI) :- node@NI(NI, N),
    eagerFinger@NI(NI, I1, B, BI), I = I1 + 1, I > I1, K = N + 1 <<
    I, K in (N, B), BI!=NI.
*/
void ruleF7(str name,
            Router::ConfigurationRef conf,
            TableRef nodeTable,
            ElementSpecRef pushEagerFingerIn,
            int pushEagerFingerInPort,
            ElementSpecRef pullEagerFingerOut,
            int pullEagerFingerOutPort)
{
  // Join eagerFinger with node table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("eagerFingerIntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match eagerFinger.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  conf->hookUp(pushEagerFingerIn, pushEagerFingerInPort, join1S, 0);

  // Select I1+1>I1, N+(1<<I1+1) in (N,B), NI!=BI
  // from
  // eagerFinger(NI, I1, B, BI), node(NI, N)
  ElementSpecRef select1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("select1:") << name,
                                                    "$0 2 field dup 1 +i /* I1 (I1+1) */\
                                                     <i /* I1 < I1+1? */\
                                                     $0 1 field $0 4 field ==s not and /* I1 < I1+1? && BI!=NI */\
                                                     not ifstop /* (I1+1>?I1) */\
                                                     $0 pop $1 pop\
                                                     "));
  ElementSpecRef select2S =
    conf->addElement(New refcounted< PelTransform >(strbuf("select2:") << name,
                                                    "$1 2 field /* N */\
                                                     1 ->u32 ->id /* N 1 */\
                                                     $0 2 field 1 +i /* N 1 (I1+1) */\
                                                     <<id +id /* K=N+[1<<(I1+1)] */\
                                                     $1 2 field /* K N */\
                                                     $0 3 field /* K N B */\
                                                     ()id /* K in (N,B) */\
                                                     not ifstop /* select clause, empty stack */\
                                                     $0 pop $1 pop\
                                                     "));
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:") << name,
                                                    "$0 0 field pop /* out eagerFinger */\
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field 1 +i pop /* out I1+1 */\
                                                     $0 3 field pop /* out B */\
                                                     $0 4 field pop /* out BI */\
                                                     "));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, select1S, 0);
  conf->hookUp(select1S, 0, select2S, 0);
  conf->hookUp(select2S, 0, projectS, 0);
  conf->hookUp(projectS, 0, pullEagerFingerOut, pullEagerFingerOutPort);
}


/**
   rule F8 nextFingerFix@NI(NI, 0) :- eagerFinger@NI(NI, I, B, BI), ((I
   == finger.SIZE - 1) || (BI == NI)).
*/
void ruleF8(str name,
            Router::ConfigurationRef conf,
            int fingerSize,
            ElementSpecRef pushEagerFingerIn,
            int pushEagerFingerInPort,
            ElementSpecRef pullNextFingerFixOut,
            int pullNextFingerFixOutPort)
{
  // Select I==finger.SIZE-1 or BI==NI
  // from 
  // eagerFinger(NI, I, B, BI)
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("select:") << name,
                                                    strbuf("$2 /* I */ ")
                                                    << fingerSize <<
                                                    " 1 -i /* I fingerSize-1 */\
                                                     ==i /* I==?fingerSize-1 */\
                                                     $1 $4 /* I==?fingerSize-1 NI BI */\
                                                     ==s /* I==?fingerSize-1 NI==?BI */\
                                                     or /* I==?fingerSize-1 || NI==?BI */\
                                                     not ifstop /* empty */\
                                                     \"nextFingerFix\" pop /* out nextFingerFix */\
                                                     $1 pop /* out NI */\
                                                     0 pop /* out 0 */\
                                                     "));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:") << name));
  conf->hookUp(pushEagerFingerIn, pushEagerFingerInPort, selectS, 0);
  conf->hookUp(selectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullNextFingerFixOut, pullNextFingerFixOutPort);
}



/** rule F9 nextFingerFix@NI(NI, I) :- node@NI(NI, N),
    eagerFinger@NI(NI, I1, B, BI), I = I1 + 1, I > I1, K = N + 1 << I, K
    in (B, N), NI!=BI.
*/
void ruleF9(str name,
            Router::ConfigurationRef conf,
            TableRef nodeTable,
            ElementSpecRef pushEagerFingerIn,
            int pushEagerFingerInPort,
            ElementSpecRef pullNextFingerFixOut,
            int pullNextFingerFixOutPort)
{
  // Join eagerFinger with node table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("eagerFingerIntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match eagerFinger.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  conf->hookUp(pushEagerFingerIn, pushEagerFingerInPort, join1S, 0);

  // Select I1+1>I1
  // from
  // eagerFinger(NI, I1, B, BI), node(NI, N)
  ElementSpecRef select1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("select1:") << name,
                                                    "$0 2 field dup 1 +i /* I1 (I1+1) */\
                                                     <i /* I1 < I1+1? */\
                                                     not ifstop /* (I1+1>?I1) */\
                                                     $0 pop $1 pop\
                                                     "));
  ElementSpecRef select2S =
    conf->addElement(New refcounted< PelTransform >(strbuf("select2:") << name,
                                                    "$0 1 field /* NI */\
                                                     $0 4 field /* NI BI */\
                                                     ==s /* NI == BI */\
                                                     ifstop /* (NI!=BI) */\
                                                     $0 pop $1 pop\
                                                     "));
  ElementSpecRef select3S =
    conf->addElement(New refcounted< PelTransform >(strbuf("select2:") << name,
                                                    "$1 2 field /* N */\
                                                     1 ->u32 ->id /* N 1 */\
                                                     $0 2 field 1 +i /* N 1 (I1+1) */\
                                                     <<id +id /* K=N+[1<<(I1+1)] */\
                                                     $0 3 field /* K B */\
                                                     $1 2 field /* K B N */\
                                                     ()id /* K in (B,N) */\
                                                     not ifstop /* select clause, empty stack */\
                                                     $0 pop $1 pop\
                                                     "));
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:") << name,
                                                    "\"nextFingerFix\" pop /* out nextFingerFix */\
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field 1 +i pop /* out I1+1 */\
                                                     "));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, select1S, 0);
  conf->hookUp(select1S, 0, select2S, 0);
  conf->hookUp(select2S, 0, select3S, 0);
  conf->hookUp(select3S, 0, projectS, 0);
  conf->hookUp(projectS, 0, pullNextFingerFixOut, pullNextFingerFixOutPort);
}



/** rule J1 join@NI(NI,E) :- joinEvent@NI(NI), E=f_rand(). */
void ruleJ1(str name,
            Router::ConfigurationRef conf,
            ElementSpecRef pushJoinEventIn,
            int pushJoinEventInPort,
            ElementSpecRef pullJoinOut,
            int pullJoinOutPort)
{
  // Project onto join(NI, E)
  // from
  // joinEvent(NI)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"join\" pop \
                                                     $1 pop /* out jE.NI */\
                                                     $1 \":\" strcat rand strcat pop /* out random E */\
                                                     "));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:") << name));
  conf->hookUp(pushJoinEventIn, pushJoinEventInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullJoinOut, pullJoinOutPort);
}


/** rule J1a joinEvent@NI(ni) once. */
void ruleJ1a(str name,
             Router::ConfigurationRef conf,
             str localAddress,
             double delay,
             ElementSpecRef pullJoinEventOut,
             int pullJoinEventOutPort)
{
  // My next finger fix tuple
  TupleRef joinEventTuple = Tuple::mk();
  joinEventTuple->append(Val_Str::mk("joinEvent"));
  joinEventTuple->append(Val_Str::mk(localAddress));
  joinEventTuple->freeze();

  ElementSpecRef sourceS =
    conf->addElement(New refcounted< TupleSource >(str("JoinEventSource:") << name,
                                                   joinEventTuple));
  
  // The once pusher
  ElementSpecRef onceS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("JoinEventPush:") << name,
                                                     delay, // run then
                                                     4 // run once
                                                     ));

  // And a slot from which to pull
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("JoinEventSlot:") << name));

  // Link everything
  conf->hookUp(sourceS, 0, onceS, 0);
  conf->hookUp(onceS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullJoinEventOut, pullJoinEventOutPort);
}


/** rule J2 joinRecord@NI(NI,E) :- join@NI(NI,E).
 */
void ruleJ2(str name,
            Router::ConfigurationRef conf,
            ElementSpecRef pushJoinIn,
            int pushJoinInPort,
            ElementSpecRef pullJoinRecordOut,
            int pullJoinRecordOutPort)
{
  // Project onto joinRecord(NI, E)
  // from
  // join(NI, E)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"joinRecord\" pop \
                                                     $1 pop /* out j.NI */\
                                                     $2 pop /* out j.E */\
                                                     "));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:") << name));
  conf->hookUp(pushJoinIn, pushJoinInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullJoinRecordOut, pullJoinRecordOutPort);
}

/** rule J3 startJoin@LI(LI,N,NI,E) :- join@NI(NI,E), node@NI(NI,N),
    landmarkNode@NI(NI,LI), LI != "".
*/
void ruleJ3(str name,
            Router::ConfigurationRef conf,
            TableRef landmarkNodeTable,
            TableRef nodeTable,
            ElementSpecRef pushJoinIn,
            int pushJoinInPort,
            ElementSpecRef pullStartJoinOut,
            int pullStartJoinOutPort)
{
  // Join join with landmark table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("joinIntoLandmark:") << name,
                                                    landmarkNodeTable,
                                                    1, // Match join.NI
                                                    1 // with landmarkNode.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  conf->hookUp(pushJoinIn, pushJoinInPort, join1S, 0);

  // Produce res1(NI, E, LI)
  // <join(NI, E),landmarkNode(NI, LI)>
  ElementSpecRef project1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("projectRes1:").cat(name),
                                                    "\"res1\" pop \
                                                     $0 1 field pop /* out join.NI */\
                                                     $0 2 field pop /* out join.E */\
                                                     $1 2 field pop /* out lN.LI */\
                                                     "));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, project1S, 0);


  // Join res1 with node table
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name, 0));
  ElementSpecRef join2S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("res1IntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match res1.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNull2S = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull2:") << name, 1));

  conf->hookUp(project1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, join2S, 0);

  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("select:") << name,
                                                    "$0 3 field /* res1.LI */\
                                                     \"-\" ==s ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     "));


  // Produce startJoin(LI,N,NI,E)
  // <res1(NI, E, LI),node(NI, N)>
  ElementSpecRef project2S =
    conf->addElement(New refcounted< PelTransform >(strbuf("makeStartJoin:").cat(name),
                                                    "\"startJoin\" pop \
                                                     $0 3 field pop /* out res1.LI */\
                                                     $1 2 field pop /* out node.N */\
                                                     $0 1 field pop /* out res1.NI */\
                                                     $0 2 field pop /* out res1.E */\
                                                     "));
  conf->hookUp(join2S, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, selectS, 0);
  conf->hookUp(selectS, 0, project2S, 0);


  conf->hookUp(project2S, 0, pullStartJoinOut, pullStartJoinOutPort);
}


/** rule J4 lookup@LI(LI,N,NI,E) :- startJoin@LI(LI,N,NI,E).
 */
void ruleJ4(str name,
            Router::ConfigurationRef conf,
            ElementSpecRef pushStartJoinIn,
            int pushStartJoinInPort,
            ElementSpecRef pullLookupOut,
            int pullLookupOutPort)
{
  // Project onto lookup(NI, K, R, E)
  // from
  // startJoin(NI, J, JI, E)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"lookup\" pop \
                                                     $1 pop /* out sj.NI */\
                                                     $2 pop /* out sj.J */\
                                                     $3 pop /* out sj.JI */\
                                                     $4 pop /* out sj.E */\
                                                     "));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:") << name));
  conf->hookUp(pushStartJoinIn, pushStartJoinInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullLookupOut, pullLookupOutPort);
}



/** rule J5 successor@NI(NI,S,SI) :- joinRecord@NI(NI,E),
    lookupResults@NI(NI,K,S,SI,E).
*/
void ruleJ5(str name,
            Router::ConfigurationRef conf,
            TableRef joinRecordTable,
            ElementSpecRef pushLookupResultsIn,
            int pushLookupResultsInPort,
            ElementSpecRef pullSuccessorOut,
            int pullSuccessorOutPort)
{
  // Join lookupResults with joinRecord table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< MultLookup >(strbuf("lookupResultsIntoJoinRecord:") << name,
                                                  joinRecordTable,
                                                  1, // Match lookupResults.NI
                                                  1 // with joinRecord.NI
                                                  ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  conf->hookUp(pushLookupResultsIn, pushLookupResultsInPort, join1S, 0);

  // Select joinRecord.E == lookupResults.E
  // lookupResults(NI, K, S, SI, E),joinRecord(NI, E)
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("select:") << name,
                                                    "$0 5 field /* lookupResults.E */\
                                                     $1 2 field /* lR.E joinRecord.E */\
                                                     ==s not /* lR.E!=fL.E? */\
                                                     ifstop /* empty */\
                                                     $0 pop $1 pop /* pass through */\
                                                     "));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, selectS, 0);

  // Produce successor(NI, S, SI)
  // from
  // lookupResults(NI, K, S, SI, E),joinRecord(NI, E)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"successor\" pop \
                                                     $0 1 field pop /* out lR.NI */\
                                                     $0 3 field pop /* fL.S */\
                                                     $0 4 field pop /* fL.SI */\
                                                     "));
  conf->hookUp(selectS, 0, projectS, 0);
  conf->hookUp(projectS, 0, pullSuccessorOut, pullSuccessorOutPort);
}


/** rule J6 predecessor@NI(ni,null,""). */
void ruleJ6(str name,
            Router::ConfigurationRef conf,
            str localAddress,
            ElementSpecRef pullPredecessorOut,
            int pullPredecessorOutPort)
{
  // My predecessor tuple
  TupleRef predecessorTuple = Tuple::mk();
  predecessorTuple->append(Val_Str::mk("predecessor"));
  predecessorTuple->append(Val_Str::mk(localAddress));
  predecessorTuple->append(Val_ID::mk(ID::mk()));
  predecessorTuple->append(Val_Str::mk(str("-"))); // this is "null"
  predecessorTuple->freeze();

  ElementSpecRef sourceS =
    conf->addElement(New refcounted< TupleSource >(str("PredSource:") << name,
                                                   predecessorTuple));
  
  // The once pusher
  ElementSpecRef onceS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PredPush:") << name,
                                                     0, // run immediately
                                                     1 // run once
                                                     ));

  // And a slot from which to pull
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("PredSlot:") << name));

  // Link everything
  conf->hookUp(sourceS, 0, onceS, 0);
  conf->hookUp(onceS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullPredecessorOut, pullPredecessorOutPort);
}


/** rule J7 successor@NI(NI, NI, N) :- landmarkNode@NI(NI, LI),
    node@NI(NI, N), LI == "".
*/
void ruleJ7(str name,
            Router::ConfigurationRef conf,
            TableRef landmarkNodeTable,
            TableRef nodeTable,
            ElementSpecRef pushJoinIn,
            int pushJoinInPort,
            ElementSpecRef pullSuccessorOut,
            int pullSuccessorOutPort)
{
  // Join join with landmark table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("joinIntoLandmark:") << name,
                                                    landmarkNodeTable,
                                                    1, // Match join.NI
                                                    1 // with landmarkNode.NI
                                                    ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  conf->hookUp(pushJoinIn, pushJoinInPort, join1S, 0);

  // Produce res1(NI, E, LI)
  // <join(NI, E),landmarkNode(NI, LI)>
  ElementSpecRef project1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("projectRes1:").cat(name),
                                                    "\"res1\" pop \
                                                     $0 1 field pop /* out join.NI */\
                                                     $0 2 field pop /* out join.E */\
                                                     $1 2 field pop /* out lN.LI */\
                                                     "));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, project1S, 0);


  // Join res1 with node table
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name, 0));
  ElementSpecRef join2S =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("res1IntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match res1.NI
                                                    1 // with node.NI
                                                    ));
  ElementSpecRef noNull2S = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull2:") << name, 1));

  conf->hookUp(project1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, join2S, 0);

  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("select:") << name,
                                                    "$0 3 field /* res1.LI */\
                                                     \"-\" ==s not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     "));


  // Produce successor(NI, N, NI)
  // <res1(NI, E, LI),node(NI, N)>
  ElementSpecRef project2S =
    conf->addElement(New refcounted< PelTransform >(strbuf("makeSuccessor:").cat(name),
                                                    "\"successor\" pop \
                                                     $0 1 field pop /* out res1.NI */\
                                                     $1 2 field pop /* out node.N */\
                                                     $0 1 field pop /* out res1.NI */\
                                                     "));
  conf->hookUp(join2S, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, selectS, 0);
  conf->hookUp(selectS, 0, project2S, 0);


  conf->hookUp(project2S, 0, pullSuccessorOut, pullSuccessorOutPort);
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




void
connectRules(str name,
             str localAddress,
             Router::ConfigurationRef conf,
             TableRef bestSuccessorTable,
             TableRef fingerLookupTable,
             TableRef fingerTable,
             TableRef joinRecordTable,
             TableRef landmarkNodeTable,
             TableRef nextFingerFixTable,
             TableRef nodeTable,
             TableRef predecessorTable,
             TableRef stabilizeRecordTable,
             TableRef successorTable,
             Table::MultAggregate successorCountAggregate,
             ElementSpecRef pushTupleIn,
             int pushTupleInPort,
             ElementSpecRef pullTupleOut,
             int pullTupleOutPort,
             double delay = 0)
{
  // My wraparound mux.  On input 0 comes the outside world. On input 1
  // come tuples that have left locally destined for local rules
  ElementSpecRef wrapAroundMux = conf->addElement(New refcounted< Mux >(strbuf("WA:") <<(name), 2));
  ElementSpecRef incomingP = conf->addElement(New refcounted< PrintTime >(strbuf("In:") << name));
  ElementSpecRef incomingQueue = conf->addElement(New refcounted< Queue >(strbuf("incomingQueue")<< name, 1000));
  ElementSpecRef incomingPusher = conf->addElement(New refcounted< TimedPullPush >(strbuf("incomingPusher:") << name, 0));
  conf->hookUp(pushTupleIn, pushTupleInPort, incomingP, 0);
  conf->hookUp(incomingP, 0, incomingQueue, 0);
  conf->hookUp(incomingQueue, 0, incomingPusher, 0);
  conf->hookUp(incomingPusher, 0, wrapAroundMux, 0);


  // The demux element for tuples
  ref< vec< ValueRef > > demuxKeys = New refcounted< vec< ValueRef > >;
  demuxKeys->push_back(New refcounted< Val_Str >(str("successor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("lookup")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("bestLookupDistance")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("bestSuccessorDist")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("maxSuccessorDist")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("evictSuccessor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("successorCount")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("node")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("finger")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("predecessor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("bestSuccessor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("nextFingerFix")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("fingerLookup")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("stabilizeRecord")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("fixFinger")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("lookupResults")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("join")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("joinEvent")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("joinRecord")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("landmarkNode")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("startJoin")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("stabilize")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("stabilizeEvent")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("stabilizeRequest")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("sendPredecessor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("notify")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("notifyPredecessor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("eagerFinger")));
  ElementSpecRef demuxS = conf->addElement(New refcounted< Demux >("demux", demuxKeys));
  conf->hookUp(wrapAroundMux, 0, demuxS, 0);

  int nextDemuxOutput = 0;
  // Create the duplicator for each tuple name.  Store the tuple first
  // for materialized tuples
  ElementSpecRef dupSuccessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("successor") << "Dup:" << name, 1));
  ElementSpecRef insertSuccessor = conf->addElement(New refcounted< Insert >(strbuf("successor") << "Insert:" << name, successorTable));
  conf->hookUp(demuxS, nextDemuxOutput++, insertSuccessor, 0);
  conf->hookUp(insertSuccessor, 0, dupSuccessor, 0);

  ElementSpecRef dupLookup = conf->addElement(New refcounted< DuplicateConservative >(strbuf("lookup") << "Dup:" << name, 2));
  ElementSpecRef qLookup = conf->addElement(New refcounted< Queue >("lookupQueue", QUEUE_LENGTH));
  ElementSpecRef tPPLookup = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qLookup, 0);
  conf->hookUp(qLookup, 0, tPPLookup, 0);
  conf->hookUp(tPPLookup, 0, dupLookup, 0);

  ElementSpecRef dupBestLookupDistance = conf->addElement(New refcounted< DuplicateConservative >(strbuf("bestLookupDistance") << "Dup:" << name, 1));
  ElementSpecRef qBestLookupDistance = conf->addElement(New refcounted< Queue >("BestLookupDistance", QUEUE_LENGTH));
  ElementSpecRef tPPBestLookupDistance = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qBestLookupDistance, 0);
  conf->hookUp(qBestLookupDistance, 0, tPPBestLookupDistance, 0);
  conf->hookUp(tPPBestLookupDistance, 0, dupBestLookupDistance, 0);

  ElementSpecRef dupBestSuccessorDistance = conf->addElement(New refcounted< DuplicateConservative >(strbuf("bestSuccessorDist") << "Dup:" << name, 1));
  ElementSpecRef qBestSuccessorDistance = conf->addElement(New refcounted< Queue >("BestSuccessorDistanceQueue", QUEUE_LENGTH));
  ElementSpecRef tPPBestSuccessorDistance = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qBestSuccessorDistance, 0);
  conf->hookUp(qBestSuccessorDistance, 0, tPPBestSuccessorDistance, 0);
  conf->hookUp(tPPBestSuccessorDistance, 0, dupBestSuccessorDistance, 0);

  ElementSpecRef dupMaxSuccessorDist = conf->addElement(New refcounted< DuplicateConservative >(strbuf("maxSuccessorDist") << "Dup:" << name, 1));
  ElementSpecRef qMaxSuccessorDist = conf->addElement(New refcounted< Queue >("maxSuccessorDistQueue", QUEUE_LENGTH));
  ElementSpecRef tPPMaxSuccessorDist = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qMaxSuccessorDist, 0);
  conf->hookUp(qMaxSuccessorDist, 0, tPPMaxSuccessorDist, 0);
  conf->hookUp(tPPMaxSuccessorDist, 0, dupMaxSuccessorDist, 0);

  ElementSpecRef dupEvictSuccessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("evictSuccessor") << "Dup:" << name, 1));
  ElementSpecRef qEvictSuccessor = conf->addElement(New refcounted< Queue >("evictSuccessorQueue", QUEUE_LENGTH));
  ElementSpecRef tPPEvictSuccessor = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qEvictSuccessor, 0);
  conf->hookUp(qEvictSuccessor, 0, tPPEvictSuccessor, 0);
  conf->hookUp(tPPEvictSuccessor, 0, dupEvictSuccessor, 0);

  ElementSpecRef dupSuccessorCount = conf->addElement(New refcounted< DuplicateConservative >(strbuf("successorCount") << "Dup:" << name, 1));
  ElementSpecRef qSuccessorCount = conf->addElement(New refcounted< Queue >("successorCountQueue", QUEUE_LENGTH));
  ElementSpecRef tPPSuccessorCount = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qSuccessorCount, 0);
  conf->hookUp(qSuccessorCount, 0, tPPSuccessorCount, 0);
  conf->hookUp(tPPSuccessorCount, 0, dupSuccessorCount, 0);

  ElementSpecRef insertNode = conf->addElement(New refcounted< Insert >(strbuf("node") << "Insert:" << name, nodeTable));
  ElementSpecRef discardNode = conf->addElement(New refcounted< Discard >(strbuf("node") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertNode, 0);
  conf->hookUp(insertNode, 0, discardNode, 0);

  ElementSpecRef insertFinger = conf->addElement(New refcounted< Insert >(strbuf("finger") << "Insert:" << name, fingerTable));
  ElementSpecRef discardFinger = conf->addElement(New refcounted< Discard >(strbuf("finger") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertFinger, 0);
  conf->hookUp(insertFinger, 0, discardFinger, 0);
 
  ElementSpecRef insertPredecessor = conf->addElement(New refcounted< Insert >(strbuf("predecessor") << "Insert:" << name, predecessorTable));
  ElementSpecRef discardPredecessor = conf->addElement(New refcounted< Discard >(strbuf("predecessor") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertPredecessor, 0);
  conf->hookUp(insertPredecessor, 0, discardPredecessor, 0);

  ElementSpecRef insertBestSuccessor = conf->addElement(New refcounted< Insert >(strbuf("bestSuccessor") << "Insert:" << name, bestSuccessorTable));
  ElementSpecRef dupBestSuccessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("bestSuccessor") << "Dup:" << name, 1));
  ElementSpecRef qBestSuccessor = conf->addElement(New refcounted< Queue >("bestSuccessorQueue", QUEUE_LENGTH));
  ElementSpecRef tPPBestSuccessor = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, insertBestSuccessor, 0);
  conf->hookUp(insertBestSuccessor, 0, qBestSuccessor, 0);
  conf->hookUp(qBestSuccessor, 0, tPPBestSuccessor, 0);
  conf->hookUp(tPPBestSuccessor, 0, dupBestSuccessor, 0);

  ElementSpecRef insertNextFingerFix = conf->addElement(New refcounted< Insert >(strbuf("nextFingerFix") << "Insert:" << name, nextFingerFixTable));
  ElementSpecRef discardNextFingerFix = conf->addElement(New refcounted< Discard >(strbuf("nextFingerFix") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertNextFingerFix, 0);
  conf->hookUp(insertNextFingerFix, 0, discardNextFingerFix, 0);

  ElementSpecRef insertFingerLookup = conf->addElement(New refcounted< Insert >(strbuf("fingerLookup") << "Insert:" << name, fingerLookupTable));
  ElementSpecRef dupFingerLookup = conf->addElement(New refcounted< DuplicateConservative >(strbuf("fingerLookup") << "Dup:" << name, 1));
  ElementSpecRef qFingerLookup = conf->addElement(New refcounted< Queue >("fingerLookupQueue", QUEUE_LENGTH));
  ElementSpecRef tPPFingerLookup = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, insertFingerLookup, 0);
  conf->hookUp(insertFingerLookup, 0, qFingerLookup, 0);
  conf->hookUp(qFingerLookup, 0, tPPFingerLookup, 0);
  conf->hookUp(tPPFingerLookup, 0, dupFingerLookup, 0);

  ElementSpecRef insertStabilizeRecord = conf->addElement(New refcounted< Insert >(strbuf("stabilizeRecord") << "Insert:" << name, stabilizeRecordTable));
  ElementSpecRef discardStabilizeRecord = conf->addElement(New refcounted< Discard >(strbuf("stabilizeRecord") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertStabilizeRecord, 0);
  conf->hookUp(insertStabilizeRecord, 0, discardStabilizeRecord, 0);

  ElementSpecRef dupFixFinger = conf->addElement(New refcounted< DuplicateConservative >(strbuf("fixFinger") << "Dup:" << name, 1));
  ElementSpecRef qFixFinger = conf->addElement(New refcounted< Queue >("fixFingerQueue", QUEUE_LENGTH));
  ElementSpecRef tPPFixFinger = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qFixFinger, 0);
  conf->hookUp(qFixFinger, 0, tPPFixFinger, 0);
  conf->hookUp(tPPFixFinger, 0, dupFixFinger, 0);

  ElementSpecRef dupLookupResults = conf->addElement(New refcounted< DuplicateConservative >(strbuf("lookupResults") << "Dup:" << name, 2));
  ElementSpecRef qLookupResults = conf->addElement(New refcounted< Queue >("LookupResultsQueue", QUEUE_LENGTH));
  ElementSpecRef tPPLookupResults = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qLookupResults, 0);
  conf->hookUp(qLookupResults, 0, tPPLookupResults, 0);
  conf->hookUp(tPPLookupResults, 0, dupLookupResults, 0);

  ElementSpecRef dupJoin = conf->addElement(New refcounted< DuplicateConservative >(strbuf("join") << "Dup:" << name, 3));
  ElementSpecRef qJoin = conf->addElement(New refcounted< Queue >("JoinQueue", QUEUE_LENGTH));
  ElementSpecRef tPPJoin = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qJoin, 0);
  conf->hookUp(qJoin, 0, tPPJoin, 0);
  conf->hookUp(tPPJoin, 0, dupJoin, 0);

  ElementSpecRef dupJoinEvent = conf->addElement(New refcounted< DuplicateConservative >(strbuf("joinEvent") << "Dup:" << name, 1));
  ElementSpecRef qJoinEvent = conf->addElement(New refcounted< Queue >("joinEventQueue", QUEUE_LENGTH));
  ElementSpecRef tPPJoinEvent = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qJoinEvent, 0);
  conf->hookUp(qJoinEvent, 0, tPPJoinEvent, 0);
  conf->hookUp(tPPJoinEvent, 0, dupJoinEvent, 0);

  ElementSpecRef insertJoinRecord = conf->addElement(New refcounted< Insert >(strbuf("joinRecord") << "Insert:" << name, joinRecordTable));
  ElementSpecRef discardJoinRecord = conf->addElement(New refcounted< Discard >(strbuf("joinRecord") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertJoinRecord, 0);
  conf->hookUp(insertJoinRecord, 0, discardJoinRecord, 0);

  ElementSpecRef insertLandmarkNode = conf->addElement(New refcounted< Insert >(strbuf("landmarkNode") << "Insert:" << name, landmarkNodeTable));
  ElementSpecRef discardLandmarkNode = conf->addElement(New refcounted< Discard >(strbuf("landmarkNode") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertLandmarkNode, 0);
  conf->hookUp(insertLandmarkNode, 0, discardLandmarkNode, 0);

  ElementSpecRef dupStartJoin = conf->addElement(New refcounted< DuplicateConservative >(strbuf("startJoin") << "Dup:" << name, 1));
  ElementSpecRef qStartJoin = conf->addElement(New refcounted< Queue >("startJoinQueue", QUEUE_LENGTH));
  ElementSpecRef tPPStartJoin = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qStartJoin, 0);
  conf->hookUp(qStartJoin, 0, tPPStartJoin, 0);
  conf->hookUp(tPPStartJoin, 0, dupStartJoin, 0);

  ElementSpecRef dupStabilize = conf->addElement(New refcounted< DuplicateConservative >(strbuf("stabilize") << "Dup:" << name, 2));
  ElementSpecRef qStabilize = conf->addElement(New refcounted< Queue >("StabilizeQueue", QUEUE_LENGTH));
  ElementSpecRef tPPStabilize = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qStabilize, 0);
  conf->hookUp(qStabilize, 0, tPPStabilize, 0);
  conf->hookUp(tPPStabilize, 0, dupStabilize, 0);

  ElementSpecRef dupStabilizeEvent = conf->addElement(New refcounted< DuplicateConservative >(strbuf("stabilizeEvent") << "Dup:" << name, 1));
  ElementSpecRef qStabilizeEvent = conf->addElement(New refcounted< Queue >("stabilizeEventQueue", QUEUE_LENGTH));
  ElementSpecRef tPPStabilizeEvent = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qStabilizeEvent, 0);
  conf->hookUp(qStabilizeEvent, 0, tPPStabilizeEvent, 0);
  conf->hookUp(tPPStabilizeEvent, 0, dupStabilizeEvent, 0);

  ElementSpecRef dupStabilizeRequest = conf->addElement(New refcounted< DuplicateConservative >(strbuf("stabilizeRequest") << "Dup:" << name, 1));
  ElementSpecRef qStabilizeRequest = conf->addElement(New refcounted< Queue >("stabilizeRequestQueue", QUEUE_LENGTH));
  ElementSpecRef tPPStabilizeRequest = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qStabilizeRequest, 0);
  conf->hookUp(qStabilizeRequest, 0, tPPStabilizeRequest, 0);
  conf->hookUp(tPPStabilizeRequest, 0, dupStabilizeRequest, 0);

  ElementSpecRef dupSendPredecessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("sendPredecessor") << "Dup:" << name, 1));
  ElementSpecRef qSendPredecessor = conf->addElement(New refcounted< Queue >("sendPredecessorQueue", QUEUE_LENGTH));
  ElementSpecRef tPPSendPredecessor = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qSendPredecessor, 0);
  conf->hookUp(qSendPredecessor, 0, tPPSendPredecessor, 0);
  conf->hookUp(tPPSendPredecessor, 0, dupSendPredecessor, 0);

  ElementSpecRef dupNotify = conf->addElement(New refcounted< DuplicateConservative >(strbuf("notify") << "Dup:" << name, 1));
  ElementSpecRef qNotify = conf->addElement(New refcounted< Queue >("notifyQueue", QUEUE_LENGTH));
  ElementSpecRef tPPNotify = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qNotify, 0);
  conf->hookUp(qNotify, 0, tPPNotify, 0);
  conf->hookUp(tPPNotify, 0, dupNotify, 0);

  ElementSpecRef dupNotifyPredecessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("notifyPredecessor") << "Dup:" << name, 1));
  ElementSpecRef qNotifyPredecessor = conf->addElement(New refcounted< Queue >("notifyPredecessorQueue", QUEUE_LENGTH));
  ElementSpecRef tPPNotifyPredecessor = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qNotifyPredecessor, 0);
  conf->hookUp(qNotifyPredecessor, 0, tPPNotifyPredecessor, 0);
  conf->hookUp(tPPNotifyPredecessor, 0, dupNotifyPredecessor, 0);

  ElementSpecRef dupEagerFinger = conf->addElement(New refcounted< DuplicateConservative >(strbuf("eagerFinger") << "Dup:" << name, 4));
  ElementSpecRef qEagerFinger = conf->addElement(New refcounted< Queue >("EagerFingerQueue", QUEUE_LENGTH));
  ElementSpecRef tPPEagerFinger = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qEagerFinger, 0);
  conf->hookUp(qEagerFinger, 0, tPPEagerFinger, 0);
  conf->hookUp(tPPEagerFinger, 0, dupEagerFinger, 0);








  // Tuples that match nothing
  ElementSpecRef discardDefault = conf->addElement(New refcounted< Discard >(strbuf("DEFAULT") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, discardDefault, 0);



  int roundRobinPortCounter = 0;
  ElementSpecRef roundRobin = conf->addElement(New refcounted< RoundRobin >(strbuf("RoundRobin:") << name, 35));
  ElementSpecRef wrapAroundPush = conf->addElement(New refcounted< TimedPullPush >(strbuf("WrapAroundPush") << name, 0));

  // The wrap around for locally bound tuples
  ref< vec< ValueRef > > wrapAroundDemuxKeys = New refcounted< vec< ValueRef > >;
  wrapAroundDemuxKeys->push_back(New refcounted< Val_Str >(localAddress));
  ElementSpecRef wrapAroundDemux = conf->addElement(New refcounted< Demux >("wrapAround", wrapAroundDemuxKeys, 1));
  ElementSpecRef outgoingQueue = conf->addElement(New refcounted< Queue >("outgoingQueue", 1000));
  ElementSpecRef wrapAroundPrint = conf->addElement(New refcounted< PrintTime >(strbuf("Wrap:") << name));
  ElementSpecRef outgoingP = conf->addElement(New refcounted< PrintTime >(strbuf("Out:") << name));
  conf->hookUp(roundRobin, 0, wrapAroundPush, 0);
  conf->hookUp(wrapAroundPush, 0, wrapAroundDemux, 0);
  conf->hookUp(wrapAroundDemux, 0, wrapAroundPrint, 0);
  conf->hookUp(wrapAroundPrint, 0, wrapAroundMux, 1);
  conf->hookUp(wrapAroundDemux, 1, outgoingQueue, 0);
  conf->hookUp(outgoingQueue, 0, outgoingP, 0);
  conf->hookUp(outgoingP, 0, pullTupleOut, pullTupleOutPort);




  ruleL1(strbuf(name) << ":L1",
         conf,
         nodeTable,
         bestSuccessorTable,
         dupLookup, 0,
         roundRobin, roundRobinPortCounter++);
  ruleL2(strbuf(name) << ":L2",
         conf,
         nodeTable,
         fingerTable,
         dupLookup, 1,
         roundRobin, roundRobinPortCounter++);
  ruleL3(strbuf(name) << ":L3",
         conf,
         nodeTable,
         fingerTable,
         dupBestLookupDistance, 0,
         roundRobin, roundRobinPortCounter++);
  ruleSU1(strbuf(name) << ":SU1",
          conf,
          nodeTable,
          successorTable,
          dupSuccessor, 0,
          roundRobin, roundRobinPortCounter++);
  ruleSU2(strbuf(name) << ":SU2",
          conf,
          nodeTable,
          successorTable,
          bestSuccessorTable,
          dupBestSuccessorDistance, 0,
          roundRobin, roundRobinPortCounter++);
  ruleSU3(strbuf(name) << ":SU3",
          conf,
          dupBestSuccessor, 0,
          roundRobin, roundRobinPortCounter++);
  ruleSR1(strbuf(name) << ":SR1",
          conf,
          successorCountAggregate,
          roundRobin, roundRobinPortCounter++);
  ruleSR2(strbuf(name) << ":SR2",
          conf,
          SUCCESSORSIZE,
          dupSuccessorCount, 0,
          roundRobin, roundRobinPortCounter++);
  ruleSR3(strbuf(name) << ":SR3",
          conf,
          nodeTable,
          successorTable,
          dupEvictSuccessor, 0,
          roundRobin, roundRobinPortCounter++);
  ruleSR4(strbuf(name) << ":SR4",
          conf,
          nodeTable,
          successorTable,
          dupMaxSuccessorDist, 0);
  ruleF1(strbuf(name) << ":F1",
         conf,
         localAddress,
         FINGERTTL,
         roundRobin, roundRobinPortCounter++);
  ruleF2(strbuf(name) << ":F2",
         conf,
         localAddress,
         roundRobin, roundRobinPortCounter++);
  ruleF3(strbuf(name) << ":F3",
         conf,
         nextFingerFixTable,
         dupFixFinger, 0,
         roundRobin, roundRobinPortCounter++);
  ruleF4(strbuf(name) << ":F4",
         conf,
         nodeTable,
         dupFingerLookup, 0,
         roundRobin, roundRobinPortCounter++);
  ruleF5(strbuf(name) << ":F5",
         conf,
         fingerLookupTable,
         dupLookupResults, 0,
         roundRobin, roundRobinPortCounter++);
  ruleF6(strbuf(name) << ":F6",
         conf,
         dupEagerFinger, 0,
         roundRobin, roundRobinPortCounter++);
  ruleF7(strbuf(name) << ":F7",
         conf,
         nodeTable,
         dupEagerFinger, 1,
         roundRobin, roundRobinPortCounter++);
  ruleF8(strbuf(name) << ":F8",
         conf,
         FINGERSIZE,
         dupEagerFinger, 2,
         roundRobin, roundRobinPortCounter++);
  ruleF9(strbuf(name) << ":F9",
         conf,
         nodeTable,
         dupEagerFinger, 3,
         roundRobin, roundRobinPortCounter++);
  ruleJ1(strbuf(name) << ":J1",
         conf,
         dupJoinEvent, 0,
         roundRobin, roundRobinPortCounter++);
  ruleJ1a(strbuf(name) << ":J1a",
          conf,
          localAddress,
          delay,
          roundRobin, roundRobinPortCounter++);
  ruleJ2(strbuf(name) << ":J2",
         conf,
         dupJoin, 0,
         roundRobin, roundRobinPortCounter++);
  ruleJ3(strbuf(name) << ":J3",
         conf,
         landmarkNodeTable,
         nodeTable,
         dupJoin, 1,
         roundRobin, roundRobinPortCounter++);
  ruleJ4(strbuf(name) << ":J4",
         conf,
         dupStartJoin, 0,
         roundRobin, roundRobinPortCounter++);
  ruleJ5(strbuf(name) << ":J5",
         conf,
         joinRecordTable,
         dupLookupResults, 1,
         roundRobin, roundRobinPortCounter++);
  ruleJ6(strbuf(name) << ":J6",
         conf,
         localAddress,
         roundRobin, roundRobinPortCounter++);
  ruleJ7(strbuf(name) << ":J7",
         conf,
         landmarkNodeTable,
         nodeTable,
         dupJoin, 2,
         roundRobin, roundRobinPortCounter++);
  ruleS0(strbuf(name) << ":S0",
         conf,
         localAddress,
         FINGERTTL,
         roundRobin, roundRobinPortCounter++);
  ruleS0a(strbuf(name) << ":S0a",
          conf,
          dupStabilizeEvent, 0,
          roundRobin, roundRobinPortCounter++);
  ruleS0b(strbuf(name) << ":S0b",
          conf,
          dupStabilize, 0,
          roundRobin, roundRobinPortCounter++);
  ruleS1(strbuf(name) << ":S1",
         conf,
         bestSuccessorTable,
         dupStabilize, 1,
         roundRobin, roundRobinPortCounter++);
  ruleS2(strbuf(name) << ":S2",
         conf,
         predecessorTable,
         dupStabilizeRequest, 0,
         roundRobin, roundRobinPortCounter++);
  ruleS3(strbuf(name) << ":S3",
         conf,
         stabilizeRecordTable,
         nodeTable,
         bestSuccessorTable,
         dupSendPredecessor, 0,
         roundRobin, roundRobinPortCounter++);
  ruleS6a(strbuf(name) << ":S6a",
          conf,
          localAddress,
          FINGERTTL,
          roundRobin, roundRobinPortCounter++);
  ruleS6(strbuf(name) << ":S6",
         conf,
         nodeTable,
         successorTable,
         dupNotify, 0,
         roundRobin, roundRobinPortCounter++);
  ruleS7(strbuf(name) << ":S7",
         conf,
         nodeTable,
         predecessorTable,
         dupNotifyPredecessor, 0,
         roundRobin, roundRobinPortCounter++);
}

  


void createNode(str myAddress,
                str landmarkAddress,
                Router::ConfigurationRef conf,
                Udp* udp,
                double delay = 0)
{
  TableRef fingerTable =
    New refcounted< Table >(strbuf("fingerTable"), FINGERSIZE);
  fingerTable->add_unique_index(2);
  fingerTable->add_multiple_index(1);
  
  uint32_t r[ID::WORDS];
  for (uint32_t i = 0;
       i < ID::WORDS;
       i++) {
    r[i] = random();
  }
  IDRef myKey = ID::mk(r);

  TableRef nodeTable =
    New refcounted< Table >(strbuf("nodeTable"), 1);
  nodeTable->add_unique_index(1);
  

  {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("node"));
    
    tuple->append(Val_Str::mk(myAddress));
    tuple->append(Val_ID::mk(myKey));
    tuple->freeze();
    nodeTable->insert(tuple);
  }
    

  TableRef bestSuccessorTable =
    New refcounted< Table >(strbuf("bestSuccessorTable"), 1);
  bestSuccessorTable->add_unique_index(1);
  
  timespec* successorExpiration = New timespec;
  successorExpiration->tv_sec = SUCCEXPIRATION;
  successorExpiration->tv_nsec = 0;
  TableRef successorTable =
    New refcounted< Table >(strbuf("successorTable"), 100,
                            successorExpiration); // let the replacement
                                                  // policy deal with
                                                  // evictions
  successorTable->add_multiple_index(1);
  successorTable->add_unique_index(2);

  // Create the count aggregate on the unique index
  std::vector< unsigned > groupBy;
  groupBy.push_back(1);
  Table::MultAggregate successorCountAggregate =
    successorTable->add_mult_groupBy_agg(1, groupBy, 1, &Table::AGG_COUNT);

  // The next finger fix table starts out with a single tuple that never
  // expires and is only replaced
  TableRef nextFingerFixTable = New refcounted< Table >(strbuf("nextFingerFix"), 1);
  nextFingerFixTable->add_unique_index(1);

  /** The finger lookup table.  It is indexed uniquely by its event ID */
  TableRef fingerLookupTable = New refcounted< Table >(strbuf("fingerLookup"), 100);
  fingerLookupTable->add_unique_index(2);
  fingerLookupTable->add_multiple_index(1);

  // The predecessor table, indexed by its first field
  TableRef predecessorTable = New refcounted< Table >(strbuf("predecessor"), 3);
  predecessorTable->add_unique_index(1);
  

  // The joinRecord table. Singleton
  TableRef joinRecordTable = New refcounted< Table >(strbuf("joinRecord"), 100);
  joinRecordTable->add_unique_index(2);
  joinRecordTable->add_multiple_index(1);

  // The stabilizeRecord table. Singleton.
  TableRef stabilizeRecordTable = New refcounted< Table >(strbuf("stabilizeRecord"), 2);
  stabilizeRecordTable->add_unique_index(1);

  // The landmarkNode table. Singleton
  TableRef landmarkNodeTable = New refcounted< Table >(strbuf("landmarkNode"), 2);
  landmarkNodeTable->add_unique_index(1);
  


  

  

  ////////////////////////////////////////////////////////
  // The front end, UDP to rules engine
  ElementSpecRef udpRxS = conf->addElement(udp->get_rx());
  ElementSpecRef unmarshalS = conf->addElement(New refcounted< UnmarshalField >(strbuf("Unmarshal:").cat(myAddress), 1));


  // Drop the source address and decapsulate
  ElementSpecRef unBoxS = conf->addElement(New refcounted< PelTransform >(strbuf("UnBox:").cat(myAddress),
                                                                          "$1 unboxPop"));
  conf->hookUp(udpRxS, 0, unmarshalS, 0);
  conf->hookUp(unmarshalS, 0, unBoxS, 0);




  ///////////////////////////////////////////////////////
  // The back end, rules engine to UDP

  // Create the encapsulated version of this tuple, holding the
  // destination address and, encapsulated, the payload containing the
  // reach tuple.
  ElementSpecRef encapS = conf->addElement(New refcounted< PelTransform >(strbuf("encap:").cat(myAddress),
                                                                          "$1 pop /* The From address */\
                                                                           swallow pop /* The entire input tuple encapsulated */"));

  // Now marshall the payload (second field)
  ElementSpecRef marshalS = conf->addElement(New refcounted< MarshalField >(strbuf("Marshal:").cat(myAddress), 1));

  // And translate the address into a sockaddr.  This is a hack until
  // we have a consistent view of the UDP interface.
  ElementSpecRef routeS = conf->addElement(New refcounted< StrToSockaddr >(strbuf("Router:").cat(myAddress), 0));
  ElementSpecRef udpTxS = conf->addElement(udp->get_tx());
  
  
  conf->hookUp(encapS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, routeS, 0);
  conf->hookUp(routeS, 0, udpTxS, 0);



  TupleRef landmark = Tuple::mk();
  landmark->append(Val_Str::mk("landmarkNode"));
  landmark->append(Val_Str::mk(myAddress));
  landmark->append(Val_Str::mk(landmarkAddress));
  landmark->freeze();
  landmarkNodeTable->insert(landmark);
  
  connectRules(strbuf("[") << myAddress << str("]"),
               myAddress,
               conf,
               bestSuccessorTable,
               fingerLookupTable,
               fingerTable,
               joinRecordTable,
               landmarkNodeTable,
               nextFingerFixTable,
               nodeTable,
               predecessorTable,
               stabilizeRecordTable,
               successorTable,
               successorCountAggregate,
               unBoxS, 0,
               encapS, 0,
               delay);         // not alone

}



/*
 * End of file 
 */
#endif /* __CHORD_H_ */
