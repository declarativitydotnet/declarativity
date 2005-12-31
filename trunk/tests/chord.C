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

#include "loop.h"

static const int FINGERSIZE = ID::WORDS * 32;
static const int QUEUE_LENGTH = 1000;
#define __P2__WITH_CHURN__
#ifdef __P2__WITH_CHURN__
//#warning WITH CHURN
static const int FINGERTTL = 10;
static const int SUCCEXPIRATION = 15;
static const int FINGEREXPIRATION = 15;
#else
//#warning WITHOUT CHURN
static const int FINGERTTL = 10;
#endif

/**
 *rule L1 lookupResults@R(R,K,S,SI,E) :- node@NI(NI,N),
 *lookup@NI(NI,K,R,E), bestSuccessor@NI(NI,S,SI), K in (N,S].
 */
void ruleL1(str name,
            Router::ConfigurationPtr conf,
            TablePtr nodeTable,
            TablePtr bestSuccessorTable,
            ElementSpecPtr pushLookupIn,
            int pushLookupInPort,
            ElementSpecPtr pullLookupResultsOut,
            int pullLookupResultsOutPort)
{
  // Join with node
  ElementSpecPtr matchLookupIntoNodeS =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("LookupInNode:") << name,
                                                    nodeTable,
                                                    1, // Match lookup.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull:") << name, 1)));
  // Link it to the lookup coming in. Pushes match already
  conf->hookUp(pushLookupIn, pushLookupInPort, matchLookupIntoNodeS, 0);



  // Produce intermediate ephemeral result
  // res1(NI, K, R, E, N) from
  // <<lookup NI K R E><node NI N>>
  ElementSpecPtr makeRes1S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("MakeRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 unbox \
                                                     drop /* No need for lookup literal */ \
                                                     pop pop pop pop /* all lookup fields */ \
                                                     $1 2 field pop /* node.N */")));
  conf->hookUp(matchLookupIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  



  // Join with best successor table
  ElementSpecPtr matchRes1IntoBestSuccS =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("LookupInBestSucc:") << name,
                                                    bestSuccessorTable,
                                                    1, // Match res1.NI
                                                    1 // with bestSuccessor.NI
                                                    )));
  ElementSpecPtr noNull2S = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull2:") << name, 1)));
  // Res1 must be pushed to second join
  ElementSpecPtr pushRes1S =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("PushRes1:") << name, 0)));
  // Link the two joins together
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, matchRes1IntoBestSuccS, 0);



  // Select res1.K in (res1.N, bestSuccessor.S]
  // from <<res1 NI, K, R, E, N><bestSuccessor NI S SI>>
  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("SelectContainment:").cat(name),
                                                    "$0 2 field /* res1.K */\
                                                     $0 5 field /* res1.K res1.N */\
                                                     $1 2 field /* res1.K res1.N bS.S */\
                                                     (]id not ifstop /* empty */\
                                                     $0 pop $1 pop /* pass through if not dropped */")));
  conf->hookUp(matchRes1IntoBestSuccS, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, selectS, 0);
  

  // Project to create lookupResults(R, K, S, SI, E)
  // from <<res1 NI, K, R, E, N><bestSuccessor NI S SI>>
  ElementSpecPtr makeLookupResultS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("FlattenLookupResult:").cat(name),
                                                    "\"lookupResults\" pop \
                                                     $0 3 field pop /* output R */ \
                                                     $0 2 field pop /* output K */ \
                                                     $1 unbox drop drop pop pop /* output S SI */ \
                                                     $0 4 field pop /* output E */")));
  conf->hookUp(selectS, 0, makeLookupResultS, 0);
  conf->hookUp(makeLookupResultS, 0, pullLookupResultsOut, pullLookupResultsOutPort);
}

/** L2: bestLookupDistance@NI(NI,K,R,E,min<D>) :- lookup@NI(NI,K,R,E),
    node@NI(NI, N), finger@NI(NI,I,B,BI,ET), B in (N,K), D=f_dist(B,K)-1
*/
void ruleL2(str name,
            Router::ConfigurationPtr conf,
            TablePtr nodeTable,
            TablePtr fingerTable,
            ElementSpecPtr pushLookupIn,
            int pushLookupInPort,
            ElementSpecPtr pullLookupDistanceOut,
            int pullLookupDistanceOutPort)
{
  // Join with node
  ElementSpecPtr matchLookupIntoNodeS =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("LookupInNode:") << name,
                                                    nodeTable,
                                                    1, // Match lookup.NI
                                                    1 // with node.NI
                                                    )));
  // Link it to the lookup coming in. Pushes match already
  conf->hookUp(pushLookupIn, pushLookupInPort, matchLookupIntoNodeS, 0);
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull:") << name, 1)));

  // Produce intermediate ephemeral result
  // res1(NI, K, R, E, N) from
  // <<lookup NI K R E><node NI N>>
  ElementSpecPtr makeRes1S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("FlattenRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 unbox \
                                                     drop /* No need for lookup literal */ \
                                                     pop pop pop pop /* all lookup fields */ \
                                                     $1 2 field pop /* node.N */")));
  conf->hookUp(matchLookupIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  // Run aggregation over finger table with input
  // res1(NI, K, R, E, N) from
  ElementSpecPtr findMinInFingerS =
    conf->addElement(ElementPtr(new PelScan(str("bestLookupDistance:") << name,
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
                                                    drop drop drop /* empty the stack */"))));
  // Res1 must be pushed to second join
  ElementSpecPtr pushRes1S =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("PushRes1:") << name, 0)));
  
  // Link the join to the agg
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);

  ElementSpecPtr noNull2S = conf->addElement(ElementPtr(new NoNull(strbuf("NoNull2:") << name)));

  conf->hookUp(pushRes1S, 0, findMinInFingerS, 0);

  conf->hookUp(findMinInFingerS, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, pullLookupDistanceOut, pullLookupDistanceOutPort);
}







/** rule L3 lookup@BI(max<BI>,K,R,E) :-
    bestLookupDistance@NI(NI,K,R,E,D), finger@NI(NI,I,B,BI),
    D=f_dist(B,K), node@NI(NI, N), B in (N, K).*/
void ruleL3(str name,
            Router::ConfigurationPtr conf,
            TablePtr nodeTable,
            TablePtr fingerTable,
            ElementSpecPtr pushBLDIn,
            int pushBLDInPort,
            ElementSpecPtr pullLookupOut,
            int pullLookupOutPort)
{
  // Join with node
  ElementSpecPtr matchBLDIntoNodeS =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("BLDInNode:") << name,
                                                    nodeTable,
                                                    1, // Match lookup.NI
                                                    1 // with node.NI
                                                    )));
  // Link it to the BLD coming in. Pushes match already
  conf->hookUp(pushBLDIn, pushBLDInPort, matchBLDIntoNodeS, 0);
  ElementSpecPtr noNull2S = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull2:") << name, 1)));



  // Produce intermediate ephemeral result
  // res1(NI, K, R, E, D, N) from
  // <<BLD NI K R E D><node NI N>>
  ElementSpecPtr makeRes1S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("FlattenRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 unbox \
                                                     drop /* No need for BLD literal */ \
                                                     pop pop pop pop pop /* all BLD fields */ \
                                                     $1 2 field pop /* node.N */")));
  conf->hookUp(matchBLDIntoNodeS, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, makeRes1S, 0);
  

  // Select out the min-BI for all the fingers that satisfy
  // (finger.B in (res1.N, res1.K)) and (res1.D == distance(finger.B,
  // res1.K)-1) and (res1.NI == finger.NI)
  // from <Res1 NI K R E D N> input joined with
  // <Finger NI I B BI>>
  ElementSpecPtr aggregateS =
    conf->addElement(ElementPtr(new PelScan(str("tieBreaker:") << name,
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
                                                    drop drop drop drop /* empty the stack */"))));
  
  // Res1 must be pushed to aggregate
  ElementSpecPtr pushRes1S =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("PushRes1:") << name, 0)));

  ElementSpecPtr noNullS =
    conf->addElement(ElementPtr(new NoNull(strbuf("NoNull:") << name)));

  // Link the join to the aggregate
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, aggregateS, 0);



  
  conf->hookUp(aggregateS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, pullLookupOut, pullLookupOutPort);
}




/** 
    rule SU3 finger@NI(NI,0,S,SI) :- bestSuccessor@NI(NI,S,SI).
 */
void ruleSU3(str name,
             Router::ConfigurationPtr conf,
             ElementSpecPtr pushBestSuccessorIn,
             int pushBestSuccessorInPort,
             ElementSpecPtr pullFingerOut,
             int pullFingerOutPort)
{
  // Project onto finger(NI, 0, B, BI)
  // from
  // bestSuccessor(NI, S, SI)
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("project:").cat(name),
                                                    "\"finger\" pop \
                                                     $1 pop /* out bS.NI */\
                                                     0 pop /* out 0 */\
                                                     $2 pop /* out bS.S */\
                                                     $3 pop /* out bS.SI */\
                                                     ")));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("Slot:") << name)));
  conf->hookUp(pushBestSuccessorIn, pushBestSuccessorInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullFingerOut, pullFingerOutPort);
}



/** rule F1 fixFinger@NI(ni) :- periodic@NI(finger.TTL*0.5). */
void ruleF1(str name,
            Router::ConfigurationPtr conf,
            str localAddress,
            double fingerTTL,
            ElementSpecPtr pullFixFingerOut,
            int pullFixFingerOutPort)
{
  // My fix finger tuple
  TuplePtr fixFingerTuple = Tuple::mk();
  fixFingerTuple->append(Val_Str::mk("fixFinger"));
  fixFingerTuple->append(Val_Str::mk(localAddress));
  fixFingerTuple->freeze();

  ElementSpecPtr sourceS =
    conf->addElement(ElementPtr(new TupleSource(str("FFSource:") << name, fixFingerTuple)));
  
  // The timed pusher
  ElementSpecPtr pushS =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("FFPush:") << name, fingerTTL * 0.5)));

  // And a slot from which to pull
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("FFSlot:") << name)));

  // Link everything
  conf->hookUp(sourceS, 0, pushS, 0);
  conf->hookUp(pushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullFixFingerOut, pullFixFingerOutPort);
}




/** rule F2 nextFingerFix@NI(ni, 0). */
void ruleF2(str name,
            Router::ConfigurationPtr conf,
            str localAddress,
            ElementSpecPtr pullNextFingerFixOut,
            int pullNextFingerFixOutPort)
{
  // My next finger fix tuple
  TuplePtr nextFingerFixTuple = Tuple::mk();
  nextFingerFixTuple->append(Val_Str::mk("nextFingerFix"));
  nextFingerFixTuple->append(Val_Str::mk(localAddress));
  nextFingerFixTuple->append(Val_Int32::mk(0));
  nextFingerFixTuple->freeze();

  ElementSpecPtr sourceS =
    conf->addElement(ElementPtr(new TupleSource(str("NFFSource:") << name,
                                                   nextFingerFixTuple)));
  
  // The once pusher
  ElementSpecPtr onceS =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("NFFPush:") << name,
                                                     0, // run immediately
                                                     1 // run once
                                                     )));

  // And a slot from which to pull
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("NFFSlot:") << name)));

  // Link everything
  conf->hookUp(sourceS, 0, onceS, 0);
  conf->hookUp(onceS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullNextFingerFixOut, pullNextFingerFixOutPort);
}


/** rule F3 fingerLookup@NI(NI, E, I) :- fixFinger@NI(NI), E = random(),
    nextFingerFix@NI(NI, I).
*/
void ruleF3(str name,
            Router::ConfigurationPtr conf,
            TablePtr nextFingerFixTable,
            ElementSpecPtr pushFixFingerIn,
            int pushFixFingerInPort,
            ElementSpecPtr pullFingerLookupOut,
            int pullFingerLookupOutPort)
{
  // Join fixFinger with nextFingerFix
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("fixFingerIntoNextFingerFix:") << name,
                                                    nextFingerFixTable,
                                                    1, // Match fixFinger.NI
                                                    1 // with nextFingerFix.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull:") << name, 1)));

  // Link it to the evictSuccessor coming in. Pushes match already
  conf->hookUp(pushFixFingerIn, pushFixFingerInPort, join1S, 0);

  // Produce finger lookup
  // fingerLookup(NI, E, I) from
  // <<fixFinger NI><nextFingerFix NI I>>
  ElementSpecPtr makeRes1S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("FlattenFingerLookup:").cat(name),
                                                    "\"fingerLookup\" pop \
                                                     $0 1 field pop /* output fixFinger.NI */ \
                                                     $0 1 field \":\" strcat rand strcat pop /* output NI|rand */ \
                                                     $1 2 field pop /* output nextFingerFix.I */")));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  conf->hookUp(makeRes1S, 0, pullFingerLookupOut, pullFingerLookupOutPort);
}


/** rule F4 lookup@NI(NI, K, NI, E) :- fingerLookup@NI(NI, E, I),
    node(NI, N), K = N + 1 << I.
*/
void ruleF4(str name,
            Router::ConfigurationPtr conf,
            TablePtr nodeTable,
            ElementSpecPtr pushFingerLookupIn,
            int pushFingerLookupInPort,
            ElementSpecPtr pullLookupOut,
            int pullLookupOutPort)
{
  // Join fingerLookup with node
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("fingerLookupIntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match fingerLookup.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull:") << name, 1)));

  conf->hookUp(pushFingerLookupIn, pushFingerLookupInPort, join1S, 0);



  // Produce lookup
  // lookup(NI, K, NI, E) from
  // <<fingerLookup NI E I><node NI N>>
  ElementSpecPtr makeRes1S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("lookup:").cat(name),
                                                    "\"lookup\" pop \
                                                     $0 1 field pop /* output fixFinger.NI */ \
                                                     $1 2 field /* node.N */\
                                                     1 ->u32 ->id /* N (1 as ID) */\
                                                     $0 3 field /* N 1 fingerLookup.I */\
                                                     <<id /* N 2^(I) */\
                                                     +id pop /* output (N+2^I) */\
                                                     $0 1 field pop /* output fixFinger.NI again */ \
                                                     $0 2 field pop /* output fingerLookup.E */")));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  conf->hookUp(makeRes1S, 0, pullLookupOut, pullLookupOutPort);
}



/** rule F5 eagerFinger@NI(NI, I, B, BI) :- fingerLookup@NI(NI, E,
    I), lookupResults@NI(NI, K, B, BI, E).
*/
void ruleF5(str name,
            Router::ConfigurationPtr conf,
            TablePtr fingerLookupTable,
            ElementSpecPtr pushLookupResultsIn,
            int pushLookupResultsInPort,
            ElementSpecPtr pullEagerFingerOut,
            int pullEagerFingerOutPort)
{
  // Join lookupResults with fingerLookup table
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new MultLookup(strbuf("lookupResultsIntoFingerLookup:") << name,
                                                  fingerLookupTable,
                                                  1, // Match lookupResults.NI
                                                  1 // with fingerLookup.NI
                                                  )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull:") << name, 1)));

  conf->hookUp(pushLookupResultsIn, pushLookupResultsInPort, join1S, 0);

  // Select fingerLookup.E == lookupResults.E
  // <<lookupResults NI K B BI E><fingerLookup NI E I>>
  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("select:") << name,
                                                    "$0 5 field /* lookupResults.E */\
                                                     $1 2 field /* lR.E fingerLookup.E */\
                                                     ==s not /* lR.E!=fL.E? */\
                                                     ifstop /* empty */\
                                                     $0 pop $1 pop /* pass through */\
                                                     ")));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, selectS, 0);



  // Produce eagerFinger
  // eagerFinger(NI, I, B, BI) from
  // <<lookupResults NI K B BI E><fingerLookup NI E I>>
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("project:").cat(name),
                                                    "\"eagerFinger\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $1 3 field pop /* out I */\
                                                     $0 3 field pop /* out B */\
                                                     $0 4 field pop /* out BI */\
                                                     ")));
  conf->hookUp(selectS, 0, projectS, 0);

  conf->hookUp(projectS, 0, pullEagerFingerOut, pullEagerFingerOutPort);
}


/** rule F6 finger@NI(NI, I, B, BI) :- eagerFinger@NI(NI, I, B, BI). */
void ruleF6(str name,
            Router::ConfigurationPtr conf,
            ElementSpecPtr pushEagerFingerIn,
            int pushEagerFingerInPort,
            ElementSpecPtr pullFingerOut,
            int pullFingerOutPort)
{
  // Project onto finger(NI, I, B, BI)
  // from
  // eagerFinger(NI, I, B, BI)
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("project:").cat(name),
                                                    "\"finger\" pop \
                                                     $1 pop /* out eF.NI */\
                                                     $2 pop /* out eF.I */\
                                                     $3 pop /* out eF.B */\
                                                     $4 pop /* out eF.BI */\
                                                     ")));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("Slot:") << name)));
  conf->hookUp(pushEagerFingerIn, pushEagerFingerInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullFingerOut, pullFingerOutPort);
}


/** rule F6a delete<fingerLookup@NI(NI, E, I1)> :- eagerFinger@NI(NI, I,
    B, BI), I > 0, I1 = I - 1, fingerLookup@NI(NI, E, I1).
*/
void ruleF6a(str name,
             Router::ConfigurationPtr conf,
             TablePtr fingerLookupTable,
             ElementSpecPtr pushEagerFingerIn,
             int pushEagerFingerInPort)
{
  // Join eagerFinger with fingerLookup
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new MultLookup(strbuf("eagerFingerIntoFingerLookup:") << name,
                                                  fingerLookupTable,
                                                  1, // Match eagerFinger.NI
                                                  1 // with fingerLookup.NI
                                                  )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull:") << name, 1)));

  conf->hookUp(pushEagerFingerIn, pushEagerFingerInPort, join1S, 0);
  conf->hookUp(join1S, 0, noNullS, 0);


  // Select I > 0, I1 = I - 1, fingerLookup.I == I1
  // from
  // eagerFinger(NI, I, B, BI), fingerLookup(NI, E, I1)
  ElementSpecPtr select1S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("select1:") << name,
                                                    "$0 2 field /* eF.I */\
                                                     0 >i /* I > 0? */\
                                                     $0 2 field /* (I>0) I  */\
                                                     1 -i /* (I>0) (I-1) */\
                                                     $1 3 field /* (I>0) (I-1) fL.I */\
                                                     ==i /* (I>0) (I-1==fL.I) */\
                                                     and not ifstop /* selection criterion */\
                                                     $0 pop $1 pop\
                                                     ")));

  // Produce tuple with only E
  // from 
  // eagerFinger(NI, I, B, BI), fingerLookup(NI, E, I1)
  ElementSpecPtr makeRes1S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("makeDeleteTuple:").cat(name),
                                                    "\"deleteTuple\" pop \
                                                     $1 2 field pop /* output fingerLookup.E */ \
                                                     ")));


  ElementSpecPtr pushRes1S =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("PushRes1:") << name,
                                                     0)));
  // Send it for deletion
  ElementSpecPtr deleteFingerLookup =
    conf->addElement(ElementPtr(new Delete(strbuf("DeleteFingerLookup:") << name,
                                              fingerLookupTable,
                                              2,
                                              1)));


  conf->hookUp(noNullS, 0, select1S, 0);
  conf->hookUp(select1S, 0, makeRes1S, 0);
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, deleteFingerLookup, 0);
}



/** rule F7 eagerFinger@NI(NI, I, B, BI) :- node@NI(NI, N),
    eagerFinger@NI(NI, I1, B, BI), I = I1 + 1, I > I1, K = N + 1 <<
    I, K in (N, B), BI!=NI.
*/
void ruleF7(str name,
            Router::ConfigurationPtr conf,
            TablePtr nodeTable,
            ElementSpecPtr pushEagerFingerIn,
            int pushEagerFingerInPort,
            ElementSpecPtr pullEagerFingerOut,
            int pullEagerFingerOutPort)
{
  // Join eagerFinger with node table
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("eagerFingerIntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match eagerFinger.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull:") << name, 1)));

  conf->hookUp(pushEagerFingerIn, pushEagerFingerInPort, join1S, 0);

  // Select I1+1>I1, N+(1<<I1+1) in (N,B), NI!=BI
  // from
  // eagerFinger(NI, I1, B, BI), node(NI, N)
  ElementSpecPtr select1S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("select1:") << name,
                                                    "$0 2 field dup 1 +i /* I1 (I1+1) */\
                                                     <i /* I1 < I1+1? */\
                                                     $0 1 field $0 4 field ==s not and /* I1 < I1+1? && BI!=NI */\
                                                     not ifstop /* (I1+1>?I1) */\
                                                     $0 pop $1 pop\
                                                     ")));
  ElementSpecPtr select2S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("select2:") << name,
                                                    "$1 2 field /* N */\
                                                     1 ->u32 ->id /* N 1 */\
                                                     $0 2 field 1 +i /* N 1 (I1+1) */\
                                                     <<id +id /* K=N+[1<<(I1+1)] */\
                                                     $1 2 field /* K N */\
                                                     $0 3 field /* K N B */\
                                                     ()id /* K in (N,B) */\
                                                     not ifstop /* select clause, empty stack */\
                                                     $0 pop $1 pop\
                                                     ")));
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("project:") << name,
                                                    "$0 0 field pop /* out eagerFinger */\
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field 1 +i pop /* out I1+1 */\
                                                     $0 3 field pop /* out B */\
                                                     $0 4 field pop /* out BI */\
                                                     ")));
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
            Router::ConfigurationPtr conf,
            int fingerSize,
            ElementSpecPtr pushEagerFingerIn,
            int pushEagerFingerInPort,
            ElementSpecPtr pullNextFingerFixOut,
            int pullNextFingerFixOutPort)
{
  // Select I==finger.SIZE-1 or BI==NI
  // from 
  // eagerFinger(NI, I, B, BI)
  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("select:") << name,
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
                                                     ")));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("Slot:") << name)));
  conf->hookUp(pushEagerFingerIn, pushEagerFingerInPort, selectS, 0);
  conf->hookUp(selectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullNextFingerFixOut, pullNextFingerFixOutPort);
}



/** rule F9 nextFingerFix@NI(NI, I) :- node@NI(NI, N),
    eagerFinger@NI(NI, I1, B, BI), I = I1 + 1, I > I1, K = N + 1 << I, K
    in (B, N), NI!=BI.
*/
void ruleF9(str name,
            Router::ConfigurationPtr conf,
            TablePtr nodeTable,
            ElementSpecPtr pushEagerFingerIn,
            int pushEagerFingerInPort,
            ElementSpecPtr pullNextFingerFixOut,
            int pullNextFingerFixOutPort)
{
  // Join eagerFinger with node table
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("eagerFingerIntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match eagerFinger.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull:") << name, 1)));

  conf->hookUp(pushEagerFingerIn, pushEagerFingerInPort, join1S, 0);

  // Select I1+1>I1
  // from
  // eagerFinger(NI, I1, B, BI), node(NI, N)
  ElementSpecPtr select1S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("select1:") << name,
                                                    "$0 2 field dup 1 +i /* I1 (I1+1) */\
                                                     <i /* I1 < I1+1? */\
                                                     not ifstop /* (I1+1>?I1) */\
                                                     $0 pop $1 pop\
                                                     ")));
  ElementSpecPtr select2S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("select2:") << name,
                                                    "$0 1 field /* NI */\
                                                     $0 4 field /* NI BI */\
                                                     ==s /* NI == BI */\
                                                     ifstop /* (NI!=BI) */\
                                                     $0 pop $1 pop\
                                                     ")));
  ElementSpecPtr select3S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("select2:") << name,
                                                    "$1 2 field /* N */\
                                                     1 ->u32 ->id /* N 1 */\
                                                     $0 2 field 1 +i /* N 1 (I1+1) */\
                                                     <<id +id /* K=N+[1<<(I1+1)] */\
                                                     $0 3 field /* K B */\
                                                     $1 2 field /* K B N */\
                                                     ()id /* K in (B,N) */\
                                                     not ifstop /* select clause, empty stack */\
                                                     $0 pop $1 pop\
                                                     ")));
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("project:") << name,
                                                    "\"nextFingerFix\" pop /* out nextFingerFix */\
                                                     $0 1 field pop /* out NI */\
                                                     $0 2 field 1 +i pop /* out I1+1 */\
                                                     ")));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, select1S, 0);
  conf->hookUp(select1S, 0, select2S, 0);
  conf->hookUp(select2S, 0, select3S, 0);
  conf->hookUp(select3S, 0, projectS, 0);
  conf->hookUp(projectS, 0, pullNextFingerFixOut, pullNextFingerFixOutPort);
}



/** rule J1 join@NI(NI,E) :- joinEvent@NI(NI), E=f_rand(). */
void ruleJ1(str name,
            Router::ConfigurationPtr conf,
            ElementSpecPtr pushJoinEventIn,
            int pushJoinEventInPort,
            ElementSpecPtr pullJoinOut,
            int pullJoinOutPort)
{
  // Project onto join(NI, E)
  // from
  // joinEvent(NI)
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("project:").cat(name),
                                                    "\"join\" pop \
                                                     $1 pop /* out jE.NI */\
                                                     $1 \":\" strcat rand strcat pop /* out random E */\
                                                     ")));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("Slot:") << name)));
  conf->hookUp(pushJoinEventIn, pushJoinEventInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullJoinOut, pullJoinOutPort);
}


/** rule J1a joinEvent@NI(ni) once. */
void ruleJ1a(str name,
             Router::ConfigurationPtr conf,
             str localAddress,
             double delay,
             ElementSpecPtr pullJoinEventOut,
             int pullJoinEventOutPort)
{
  // My next finger fix tuple
  TuplePtr joinEventTuple = Tuple::mk();
  joinEventTuple->append(Val_Str::mk("joinEvent"));
  joinEventTuple->append(Val_Str::mk(localAddress));
  joinEventTuple->freeze();

  ElementSpecPtr sourceS =
    conf->addElement(ElementPtr(new TupleSource(str("JoinEventSource:") << name,
                                                   joinEventTuple)));
  
  // The once pusher
  ElementSpecPtr onceS =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("JoinEventPush:") << name,
                                                     delay, // run then
                                                     4 // run once
                                                     )));

  // And a slot from which to pull
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("JoinEventSlot:") << name)));

  // Link everything
  conf->hookUp(sourceS, 0, onceS, 0);
  conf->hookUp(onceS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullJoinEventOut, pullJoinEventOutPort);
}


/** rule J2 joinRecord@NI(NI,E) :- join@NI(NI,E).
 */
void ruleJ2(str name,
            Router::ConfigurationPtr conf,
            ElementSpecPtr pushJoinIn,
            int pushJoinInPort,
            ElementSpecPtr pullJoinRecordOut,
            int pullJoinRecordOutPort)
{
  // Project onto joinRecord(NI, E)
  // from
  // join(NI, E)
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("project:").cat(name),
                                                    "\"joinRecord\" pop \
                                                     $1 pop /* out j.NI */\
                                                     $2 pop /* out j.E */\
                                                     ")));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("Slot:") << name)));
  conf->hookUp(pushJoinIn, pushJoinInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullJoinRecordOut, pullJoinRecordOutPort);
}

/** rule J3 startJoin@LI(LI,N,NI,E) :- join@NI(NI,E), node@NI(NI,N),
    landmarkNode@NI(NI,LI), LI != "".
*/
void ruleJ3(str name,
            Router::ConfigurationPtr conf,
            TablePtr landmarkNodeTable,
            TablePtr nodeTable,
            ElementSpecPtr pushJoinIn,
            int pushJoinInPort,
            ElementSpecPtr pullStartJoinOut,
            int pullStartJoinOutPort)
{
  // Join join with landmark table
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("joinIntoLandmark:") << name,
                                                    landmarkNodeTable,
                                                    1, // Match join.NI
                                                    1 // with landmarkNode.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull:") << name, 1)));

  conf->hookUp(pushJoinIn, pushJoinInPort, join1S, 0);

  // Produce res1(NI, E, LI)
  // <join(NI, E),landmarkNode(NI, LI)>
  ElementSpecPtr project1S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("projectRes1:").cat(name),
                                                    "\"res1\" pop \
                                                     $0 1 field pop /* out join.NI */\
                                                     $0 2 field pop /* out join.E */\
                                                     $1 2 field pop /* out lN.LI */\
                                                     ")));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, project1S, 0);


  // Join res1 with node table
  ElementSpecPtr pushRes1S =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("PushRes1:") << name, 0)));
  ElementSpecPtr join2S =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("res1IntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match res1.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNull2S = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull2:") << name, 1)));

  conf->hookUp(project1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, join2S, 0);

  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("select:") << name,
                                                    "$0 3 field /* res1.LI */\
                                                     \"-\" ==s ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     ")));


  // Produce startJoin(LI,N,NI,E)
  // <res1(NI, E, LI),node(NI, N)>
  ElementSpecPtr project2S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("makeStartJoin:").cat(name),
                                                    "\"startJoin\" pop \
                                                     $0 3 field pop /* out res1.LI */\
                                                     $1 2 field pop /* out node.N */\
                                                     $0 1 field pop /* out res1.NI */\
                                                     $0 2 field pop /* out res1.E */\
                                                     ")));
  conf->hookUp(join2S, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, selectS, 0);
  conf->hookUp(selectS, 0, project2S, 0);


  conf->hookUp(project2S, 0, pullStartJoinOut, pullStartJoinOutPort);
}


/** rule J4 lookup@LI(LI,N,NI,E) :- startJoin@LI(LI,N,NI,E).
 */
void ruleJ4(str name,
            Router::ConfigurationPtr conf,
            ElementSpecPtr pushStartJoinIn,
            int pushStartJoinInPort,
            ElementSpecPtr pullLookupOut,
            int pullLookupOutPort)
{
  // Project onto lookup(NI, K, R, E)
  // from
  // startJoin(NI, J, JI, E)
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("project:").cat(name),
                                                    "\"lookup\" pop \
                                                     $1 pop /* out sj.NI */\
                                                     $2 pop /* out sj.J */\
                                                     $3 pop /* out sj.JI */\
                                                     $4 pop /* out sj.E */\
                                                     ")));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("Slot:") << name)));
  conf->hookUp(pushStartJoinIn, pushStartJoinInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullLookupOut, pullLookupOutPort);
}



/** rule J5 successor@NI(NI,S,SI) :- joinRecord@NI(NI,E),
    lookupResults@NI(NI,K,S,SI,E).
*/
void ruleJ5(str name,
            Router::ConfigurationPtr conf,
            TablePtr joinRecordTable,
            ElementSpecPtr pushLookupResultsIn,
            int pushLookupResultsInPort,
            ElementSpecPtr pullSuccessorOut,
            int pullSuccessorOutPort)
{
  // Join lookupResults with joinRecord table
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new MultLookup(strbuf("lookupResultsIntoJoinRecord:") << name,
                                                  joinRecordTable,
                                                  1, // Match lookupResults.NI
                                                  1 // with joinRecord.NI
                                                  )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull:") << name, 1)));

  conf->hookUp(pushLookupResultsIn, pushLookupResultsInPort, join1S, 0);

  // Select joinRecord.E == lookupResults.E
  // lookupResults(NI, K, S, SI, E),joinRecord(NI, E)
  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("select:") << name,
                                                    "$0 5 field /* lookupResults.E */\
                                                     $1 2 field /* lR.E joinRecord.E */\
                                                     ==s not /* lR.E!=fL.E? */\
                                                     ifstop /* empty */\
                                                     $0 pop $1 pop /* pass through */\
                                                     ")));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, selectS, 0);

  // Produce successor(NI, S, SI)
  // from
  // lookupResults(NI, K, S, SI, E),joinRecord(NI, E)
  ElementSpecPtr projectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("project:").cat(name),
                                                    "\"successor\" pop \
                                                     $0 1 field pop /* out lR.NI */\
                                                     $0 3 field pop /* fL.S */\
                                                     $0 4 field pop /* fL.SI */\
                                                     ")));
  conf->hookUp(selectS, 0, projectS, 0);
  conf->hookUp(projectS, 0, pullSuccessorOut, pullSuccessorOutPort);
}


/** rule J6 predecessor@NI(ni,null,""). */
void ruleJ6(str name,
            Router::ConfigurationPtr conf,
            str localAddress,
            ElementSpecPtr pullPredecessorOut,
            int pullPredecessorOutPort)
{
  // My predecessor tuple
  TuplePtr predecessorTuple = Tuple::mk();
  predecessorTuple->append(Val_Str::mk("predecessor"));
  predecessorTuple->append(Val_Str::mk(localAddress));
  predecessorTuple->append(Val_ID::mk(ID::mk()));
  predecessorTuple->append(Val_Str::mk(str("-"))); // this is "null"
  predecessorTuple->freeze();

  ElementSpecPtr sourceS =
    conf->addElement(ElementPtr(new TupleSource(str("PredSource:") << name,
                                                   predecessorTuple)));
  
  // The once pusher
  ElementSpecPtr onceS =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("PredPush:") << name,
                                                     0, // run immediately
                                                     1 // run once
                                                     )));

  // And a slot from which to pull
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("PredSlot:") << name)));

  // Link everything
  conf->hookUp(sourceS, 0, onceS, 0);
  conf->hookUp(onceS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullPredecessorOut, pullPredecessorOutPort);
}


/** rule J7 successor@NI(NI, NI, N) :- landmarkNode@NI(NI, LI),
    node@NI(NI, N), LI == "".
*/
void ruleJ7(str name,
            Router::ConfigurationPtr conf,
            TablePtr landmarkNodeTable,
            TablePtr nodeTable,
            ElementSpecPtr pushJoinIn,
            int pushJoinInPort,
            ElementSpecPtr pullSuccessorOut,
            int pullSuccessorOutPort)
{
  // Join join with landmark table
  ElementSpecPtr join1S =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("joinIntoLandmark:") << name,
                                                    landmarkNodeTable,
                                                    1, // Match join.NI
                                                    1 // with landmarkNode.NI
                                                    )));
  ElementSpecPtr noNullS = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull:") << name, 1)));

  conf->hookUp(pushJoinIn, pushJoinInPort, join1S, 0);

  // Produce res1(NI, E, LI)
  // <join(NI, E),landmarkNode(NI, LI)>
  ElementSpecPtr project1S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("projectRes1:").cat(name),
                                                    "\"res1\" pop \
                                                     $0 1 field pop /* out join.NI */\
                                                     $0 2 field pop /* out join.E */\
                                                     $1 2 field pop /* out lN.LI */\
                                                     ")));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, project1S, 0);


  // Join res1 with node table
  ElementSpecPtr pushRes1S =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("PushRes1:") << name, 0)));
  ElementSpecPtr join2S =
    conf->addElement(ElementPtr(new UniqueLookup(strbuf("res1IntoNode:") << name,
                                                    nodeTable,
                                                    1, // Match res1.NI
                                                    1 // with node.NI
                                                    )));
  ElementSpecPtr noNull2S = conf->addElement(ElementPtr(new NoNullField(strbuf("NoNull2:") << name, 1)));

  conf->hookUp(project1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, join2S, 0);

  ElementSpecPtr selectS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("select:") << name,
                                                    "$0 3 field /* res1.LI */\
                                                     \"-\" ==s not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through otherwise */\
                                                     ")));


  // Produce successor(NI, N, NI)
  // <res1(NI, E, LI),node(NI, N)>
  ElementSpecPtr project2S =
    conf->addElement(ElementPtr(new PelTransform(strbuf("makeSuccessor:").cat(name),
                                                    "\"successor\" pop \
                                                     $0 1 field pop /* out res1.NI */\
                                                     $1 2 field pop /* out node.N */\
                                                     $0 1 field pop /* out res1.NI */\
                                                     ")));
  conf->hookUp(join2S, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, selectS, 0);
  conf->hookUp(selectS, 0, project2S, 0);


  conf->hookUp(project2S, 0, pullSuccessorOut, pullSuccessorOutPort);
}






void
connectRules(str name,
             str localAddress,
             Router::ConfigurationPtr conf,
             TablePtr bestSuccessorTable,
             TablePtr fingerLookupTable,
             TablePtr fingerTable,
             TablePtr joinRecordTable,
             TablePtr landmarkNodeTable,
             TablePtr nextFingerFixTable,
             TablePtr nodeTable,
             TablePtr predecessorTable,
             TablePtr stabilizeRecordTable,
             TablePtr successorTable,
             Table::MultAggregate successorCountAggregate,
             ElementSpecPtr pushTupleIn,
             int pushTupleInPort,
             ElementSpecPtr pullTupleOut,
             int pullTupleOutPort,
             double delay = 0)
{
  // My wraparound mux.  On input 0 comes the outside world. On input 1
  // come tuples that have left locally destined for local rules
  ElementSpecPtr wrapAroundMux = conf->addElement(ElementPtr(new Mux(strbuf("WA:") <<(name), 2)));
  ElementSpecPtr incomingP = conf->addElement(ElementPtr(new PrintTime(strbuf("In:") << name)));
  ElementSpecPtr incomingQueue = conf->addElement(ElementPtr(new Queue(strbuf("incomingQueue")<< name, 1000)));
  ElementSpecPtr incomingPusher = conf->addElement(ElementPtr(new TimedPullPush(strbuf("incomingPusher:") << name, 0)));
  conf->hookUp(pushTupleIn, pushTupleInPort, incomingP, 0);
  conf->hookUp(incomingP, 0, incomingQueue, 0);
  conf->hookUp(incomingQueue, 0, incomingPusher, 0);
  conf->hookUp(incomingPusher, 0, wrapAroundMux, 0);


  // The demux element for tuples
  boost::shared_ptr< std::vector< ValuePtr > > demuxKeys(new std::vector< ValuePtr >);
  demuxKeys->push_back(ValuePtr(new Val_Str(str("successor"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("lookup"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("bestLookupDistance"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("bestSuccessorDist"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("maxSuccessorDist"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("evictSuccessor"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("successorCount"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("node"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("finger"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("predecessor"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("bestSuccessor"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("nextFingerFix"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("fingerLookup"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("stabilizeRecord"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("fixFinger"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("lookupResults"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("join"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("joinEvent"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("joinRecord"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("landmarkNode"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("startJoin"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("stabilize"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("stabilizeEvent"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("stabilizeRequest"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("sendPredecessor"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("notify"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("notifyPredecessor"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("eagerFinger"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("sendSuccessors"))));
  demuxKeys->push_back(ValuePtr(new Val_Str(str("returnSuccessor"))));
  ElementSpecPtr demuxS = conf->addElement(ElementPtr(new Demux("demux", demuxKeys)));
  conf->hookUp(wrapAroundMux, 0, demuxS, 0);

  int nextDemuxOutput = 0;
  // Create the duplicator for each tuple name.  Store the tuple first
  // for materialized tuples
  ElementSpecPtr dupSuccessor = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("successor") << "Dup:" << name, 1)));
  ElementSpecPtr insertSuccessor = conf->addElement(ElementPtr(new Insert(strbuf("successor") << "Insert:" << name, successorTable)));
  conf->hookUp(demuxS, nextDemuxOutput++, insertSuccessor, 0);
  conf->hookUp(insertSuccessor, 0, dupSuccessor, 0);

  ElementSpecPtr dupLookup = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("lookup") << "Dup:" << name, 2)));
  ElementSpecPtr qLookup = conf->addElement(ElementPtr(new Queue("lookupQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPLookup = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qLookup, 0);
  conf->hookUp(qLookup, 0, tPPLookup, 0);
  conf->hookUp(tPPLookup, 0, dupLookup, 0);

  ElementSpecPtr dupBestLookupDistance = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("bestLookupDistance") << "Dup:" << name, 1)));
  ElementSpecPtr qBestLookupDistance = conf->addElement(ElementPtr(new Queue("BestLookupDistance", QUEUE_LENGTH)));
  ElementSpecPtr tPPBestLookupDistance = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qBestLookupDistance, 0);
  conf->hookUp(qBestLookupDistance, 0, tPPBestLookupDistance, 0);
  conf->hookUp(tPPBestLookupDistance, 0, dupBestLookupDistance, 0);

  ElementSpecPtr dupBestSuccessorDistance = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("bestSuccessorDist") << "Dup:" << name, 1)));
  ElementSpecPtr qBestSuccessorDistance = conf->addElement(ElementPtr(new Queue("BestSuccessorDistanceQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPBestSuccessorDistance = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qBestSuccessorDistance, 0);
  conf->hookUp(qBestSuccessorDistance, 0, tPPBestSuccessorDistance, 0);
  conf->hookUp(tPPBestSuccessorDistance, 0, dupBestSuccessorDistance, 0);

  ElementSpecPtr dupMaxSuccessorDist = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("maxSuccessorDist") << "Dup:" << name, 1)));
  ElementSpecPtr qMaxSuccessorDist = conf->addElement(ElementPtr(new Queue("maxSuccessorDistQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPMaxSuccessorDist = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qMaxSuccessorDist, 0);
  conf->hookUp(qMaxSuccessorDist, 0, tPPMaxSuccessorDist, 0);
  conf->hookUp(tPPMaxSuccessorDist, 0, dupMaxSuccessorDist, 0);

  ElementSpecPtr dupEvictSuccessor = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("evictSuccessor") << "Dup:" << name, 1)));
  ElementSpecPtr qEvictSuccessor = conf->addElement(ElementPtr(new Queue("evictSuccessorQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPEvictSuccessor = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qEvictSuccessor, 0);
  conf->hookUp(qEvictSuccessor, 0, tPPEvictSuccessor, 0);
  conf->hookUp(tPPEvictSuccessor, 0, dupEvictSuccessor, 0);

  ElementSpecPtr dupSuccessorCount = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("successorCount") << "Dup:" << name, 1)));
  ElementSpecPtr qSuccessorCount = conf->addElement(ElementPtr(new Queue("successorCountQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPSuccessorCount = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qSuccessorCount, 0);
  conf->hookUp(qSuccessorCount, 0, tPPSuccessorCount, 0);
  conf->hookUp(tPPSuccessorCount, 0, dupSuccessorCount, 0);

  ElementSpecPtr insertNode = conf->addElement(ElementPtr(new Insert(strbuf("node") << "Insert:" << name, nodeTable)));
  ElementSpecPtr discardNode = conf->addElement(ElementPtr(new Discard(strbuf("node") << "Discard:" << name)));
  conf->hookUp(demuxS, nextDemuxOutput++, insertNode, 0);
  conf->hookUp(insertNode, 0, discardNode, 0);

  ElementSpecPtr insertFinger = conf->addElement(ElementPtr(new Insert(strbuf("finger") << "Insert:" << name, fingerTable)));
  ElementSpecPtr discardFinger = conf->addElement(ElementPtr(new Discard(strbuf("finger") << "Discard:" << name)));
  conf->hookUp(demuxS, nextDemuxOutput++, insertFinger, 0);
  conf->hookUp(insertFinger, 0, discardFinger, 0);
 
  ElementSpecPtr insertPredecessor = conf->addElement(ElementPtr(new Insert(strbuf("predecessor") << "Insert:" << name, predecessorTable)));
  ElementSpecPtr discardPredecessor = conf->addElement(ElementPtr(new Discard(strbuf("predecessor") << "Discard:" << name)));
  conf->hookUp(demuxS, nextDemuxOutput++, insertPredecessor, 0);
  conf->hookUp(insertPredecessor, 0, discardPredecessor, 0);

  ElementSpecPtr insertBestSuccessor = conf->addElement(ElementPtr(new Insert(strbuf("bestSuccessor") << "Insert:" << name, bestSuccessorTable)));
  ElementSpecPtr dupBestSuccessor = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("bestSuccessor") << "Dup:" << name, 1)));
  ElementSpecPtr qBestSuccessor = conf->addElement(ElementPtr(new Queue("bestSuccessorQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPBestSuccessor = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, insertBestSuccessor, 0);
  conf->hookUp(insertBestSuccessor, 0, qBestSuccessor, 0);
  conf->hookUp(qBestSuccessor, 0, tPPBestSuccessor, 0);
  conf->hookUp(tPPBestSuccessor, 0, dupBestSuccessor, 0);

  ElementSpecPtr insertNextFingerFix = conf->addElement(ElementPtr(new Insert(strbuf("nextFingerFix") << "Insert:" << name, nextFingerFixTable)));
  ElementSpecPtr discardNextFingerFix = conf->addElement(ElementPtr(new Discard(strbuf("nextFingerFix") << "Discard:" << name)));
  conf->hookUp(demuxS, nextDemuxOutput++, insertNextFingerFix, 0);
  conf->hookUp(insertNextFingerFix, 0, discardNextFingerFix, 0);

  ElementSpecPtr insertFingerLookup = conf->addElement(ElementPtr(new Insert(strbuf("fingerLookup") << "Insert:" << name, fingerLookupTable)));
  ElementSpecPtr dupFingerLookup = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("fingerLookup") << "Dup:" << name, 1)));
  ElementSpecPtr qFingerLookup = conf->addElement(ElementPtr(new Queue("fingerLookupQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPFingerLookup = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, insertFingerLookup, 0);
  conf->hookUp(insertFingerLookup, 0, qFingerLookup, 0);
  conf->hookUp(qFingerLookup, 0, tPPFingerLookup, 0);
  conf->hookUp(tPPFingerLookup, 0, dupFingerLookup, 0);

  ElementSpecPtr insertStabilizeRecord = conf->addElement(ElementPtr(new Insert(strbuf("stabilizeRecord") << "Insert:" << name, stabilizeRecordTable)));
  ElementSpecPtr discardStabilizeRecord = conf->addElement(ElementPtr(new Discard(strbuf("stabilizeRecord") << "Discard:" << name)));
  conf->hookUp(demuxS, nextDemuxOutput++, insertStabilizeRecord, 0);
  conf->hookUp(insertStabilizeRecord, 0, discardStabilizeRecord, 0);

  ElementSpecPtr dupFixFinger = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("fixFinger") << "Dup:" << name, 1)));
  ElementSpecPtr qFixFinger = conf->addElement(ElementPtr(new Queue("fixFingerQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPFixFinger = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qFixFinger, 0);
  conf->hookUp(qFixFinger, 0, tPPFixFinger, 0);
  conf->hookUp(tPPFixFinger, 0, dupFixFinger, 0);

  ElementSpecPtr dupLookupResults = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("lookupResults") << "Dup:" << name, 2)));
  ElementSpecPtr qLookupResults = conf->addElement(ElementPtr(new Queue("LookupResultsQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPLookupResults = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qLookupResults, 0);
  conf->hookUp(qLookupResults, 0, tPPLookupResults, 0);
  conf->hookUp(tPPLookupResults, 0, dupLookupResults, 0);

  ElementSpecPtr dupJoin = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("join") << "Dup:" << name, 3)));
  ElementSpecPtr qJoin = conf->addElement(ElementPtr(new Queue("JoinQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPJoin = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qJoin, 0);
  conf->hookUp(qJoin, 0, tPPJoin, 0);
  conf->hookUp(tPPJoin, 0, dupJoin, 0);

  ElementSpecPtr dupJoinEvent = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("joinEvent") << "Dup:" << name, 1)));
  ElementSpecPtr qJoinEvent = conf->addElement(ElementPtr(new Queue("joinEventQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPJoinEvent = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qJoinEvent, 0);
  conf->hookUp(qJoinEvent, 0, tPPJoinEvent, 0);
  conf->hookUp(tPPJoinEvent, 0, dupJoinEvent, 0);

  ElementSpecPtr insertJoinRecord = conf->addElement(ElementPtr(new Insert(strbuf("joinRecord") << "Insert:" << name, joinRecordTable)));
  ElementSpecPtr discardJoinRecord = conf->addElement(ElementPtr(new Discard(strbuf("joinRecord") << "Discard:" << name)));
  conf->hookUp(demuxS, nextDemuxOutput++, insertJoinRecord, 0);
  conf->hookUp(insertJoinRecord, 0, discardJoinRecord, 0);

  ElementSpecPtr insertLandmarkNode = conf->addElement(ElementPtr(new Insert(strbuf("landmarkNode") << "Insert:" << name, landmarkNodeTable)));
  ElementSpecPtr discardLandmarkNode = conf->addElement(ElementPtr(new Discard(strbuf("landmarkNode") << "Discard:" << name)));
  conf->hookUp(demuxS, nextDemuxOutput++, insertLandmarkNode, 0);
  conf->hookUp(insertLandmarkNode, 0, discardLandmarkNode, 0);

  ElementSpecPtr dupStartJoin = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("startJoin") << "Dup:" << name, 1)));
  ElementSpecPtr qStartJoin = conf->addElement(ElementPtr(new Queue("startJoinQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPStartJoin = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qStartJoin, 0);
  conf->hookUp(qStartJoin, 0, tPPStartJoin, 0);
  conf->hookUp(tPPStartJoin, 0, dupStartJoin, 0);

  ElementSpecPtr dupStabilize = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("stabilize") << "Dup:" << name, 3)));
  ElementSpecPtr qStabilize = conf->addElement(ElementPtr(new Queue("StabilizeQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPStabilize = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qStabilize, 0);
  conf->hookUp(qStabilize, 0, tPPStabilize, 0);
  conf->hookUp(tPPStabilize, 0, dupStabilize, 0);

  ElementSpecPtr dupStabilizeEvent = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("stabilizeEvent") << "Dup:" << name, 1)));
  ElementSpecPtr qStabilizeEvent = conf->addElement(ElementPtr(new Queue("stabilizeEventQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPStabilizeEvent = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qStabilizeEvent, 0);
  conf->hookUp(qStabilizeEvent, 0, tPPStabilizeEvent, 0);
  conf->hookUp(tPPStabilizeEvent, 0, dupStabilizeEvent, 0);

  ElementSpecPtr dupStabilizeRequest = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("stabilizeRequest") << "Dup:" << name, 1)));
  ElementSpecPtr qStabilizeRequest = conf->addElement(ElementPtr(new Queue("stabilizeRequestQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPStabilizeRequest = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qStabilizeRequest, 0);
  conf->hookUp(qStabilizeRequest, 0, tPPStabilizeRequest, 0);
  conf->hookUp(tPPStabilizeRequest, 0, dupStabilizeRequest, 0);

  ElementSpecPtr dupSendPredecessor = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("sendPredecessor") << "Dup:" << name, 1)));
  ElementSpecPtr qSendPredecessor = conf->addElement(ElementPtr(new Queue("sendPredecessorQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPSendPredecessor = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qSendPredecessor, 0);
  conf->hookUp(qSendPredecessor, 0, tPPSendPredecessor, 0);
  conf->hookUp(tPPSendPredecessor, 0, dupSendPredecessor, 0);

  ElementSpecPtr dupNotify = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("notify") << "Dup:" << name, 1)));
  ElementSpecPtr qNotify = conf->addElement(ElementPtr(new Queue("notifyQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPNotify = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qNotify, 0);
  conf->hookUp(qNotify, 0, tPPNotify, 0);
  conf->hookUp(tPPNotify, 0, dupNotify, 0);

  ElementSpecPtr dupNotifyPredecessor = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("notifyPredecessor") << "Dup:" << name, 1)));
  ElementSpecPtr qNotifyPredecessor = conf->addElement(ElementPtr(new Queue("notifyPredecessorQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPNotifyPredecessor = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qNotifyPredecessor, 0);
  conf->hookUp(qNotifyPredecessor, 0, tPPNotifyPredecessor, 0);
  conf->hookUp(tPPNotifyPredecessor, 0, dupNotifyPredecessor, 0);

  ElementSpecPtr dupEagerFinger = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("eagerFinger") << "Dup:" << name, 5)));
  ElementSpecPtr qEagerFinger = conf->addElement(ElementPtr(new Queue("EagerFingerQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPEagerFinger = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qEagerFinger, 0);
  conf->hookUp(qEagerFinger, 0, tPPEagerFinger, 0);
  conf->hookUp(tPPEagerFinger, 0, dupEagerFinger, 0);

  ElementSpecPtr dupSendSuccessors = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("sendSuccessors") << "Dup:" << name, 1)));
  ElementSpecPtr qSendSuccessors = conf->addElement(ElementPtr(new Queue("sendSuccessorsQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPSendSuccessors = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qSendSuccessors, 0);
  conf->hookUp(qSendSuccessors, 0, tPPSendSuccessors, 0);
  conf->hookUp(tPPSendSuccessors, 0, dupSendSuccessors, 0);

  ElementSpecPtr dupReturnSuccessor = conf->addElement(ElementPtr(new DuplicateConservative(strbuf("returnSuccessor") << "Dup:" << name, 1)));
  ElementSpecPtr qReturnSuccessor = conf->addElement(ElementPtr(new Queue("returnSuccessorQueue", QUEUE_LENGTH)));
  ElementSpecPtr tPPReturnSuccessor = conf->addElement(ElementPtr(new TimedPullPush(strbuf("TPP") << name, 0)));
  conf->hookUp(demuxS, nextDemuxOutput++, qReturnSuccessor, 0);
  conf->hookUp(qReturnSuccessor, 0, tPPReturnSuccessor, 0);
  conf->hookUp(tPPReturnSuccessor, 0, dupReturnSuccessor, 0);








  // Tuples that match nothing
  ElementSpecPtr discardDefault = conf->addElement(ElementPtr(new Discard(strbuf("DEFAULT") << "Discard:" << name)));
  conf->hookUp(demuxS, nextDemuxOutput++, discardDefault, 0);



  int roundRobinPortCounter = 0;
  ElementSpecPtr roundRobin = conf->addElement(ElementPtr(new RoundRobin(strbuf("RoundRobin:") << name, 38)));
  ElementSpecPtr wrapAroundPush = conf->addElement(ElementPtr(new  TimedPullPush(strbuf("WrapAroundPush") << name, 0)));

  // The wrap around for locally bound tuples
  boost::shared_ptr< std::vector< ValuePtr > > wrapAroundDemuxKeys(new std::vector< ValuePtr >);
  wrapAroundDemuxKeys->push_back(ValuePtr(new Val_Str(localAddress)));
  ElementSpecPtr wrapAroundDemux = conf->addElement(ElementPtr(new  Demux("wrapAround", wrapAroundDemuxKeys, 1)));
  ElementSpecPtr outgoingQueue = conf->addElement(ElementPtr(new  Queue("outgoingQueue", 1000)));
  ElementSpecPtr wrapAroundPrint = conf->addElement(ElementPtr(new  PrintTime(strbuf("Wrap:") << name)));
  ElementSpecPtr outgoingP = conf->addElement(ElementPtr(new  PrintTime(strbuf("Out:") << name)));
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
  ruleS4(strbuf(name) << ":S4",
         conf,
         successorTable,
         dupStabilize, 2,
         roundRobin, roundRobinPortCounter++);
  ruleS5(strbuf(name) << ":S5",
         conf,
         successorTable,
         dupSendSuccessors, 0,
         roundRobin, roundRobinPortCounter++);
  ruleS5a(strbuf(name) << ":S5a",
          conf,
          stabilizeRecordTable,
          dupReturnSuccessor, 0,
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
  ruleF6a(strbuf(name) << ":F6a",
          conf,
          fingerLookupTable,
          dupEagerFinger, 4);
}

  


void createNode(str myAddress,
                str landmarkAddress,
                Router::ConfigurationPtr conf,
                Udp* udp,
                double delay = 0)
{
#ifdef __P2__WITH_CHURN__
  timespec fingerExpiration;
  fingerExpiration.tv_sec = FINGEREXPIRATION;
  fingerExpiration.tv_nsec = 0;
  TablePtr fingerTable(new Table(strbuf("fingerTable"), FINGERSIZE, &fingerExpiration));
#else
  TablePtr fingerTable(new Table(strbuf("fingerTable"), FINGERSIZE));
#endif
  fingerTable->add_unique_index(2);
  fingerTable->add_multiple_index(1);
  
  uint32_t r[ID::WORDS];
  for (uint32_t i = 0;
       i < ID::WORDS;
       i++) {
    r[i] = random();
  }
  IDPtr myKey = ID::mk(r);

  TablePtr nodeTable(new Table(strbuf("nodeTable"), 1));
  nodeTable->add_unique_index(1);
  

  {
    TuplePtr tuple = Tuple::mk();
    tuple->append(Val_Str::mk("node"));
    
    tuple->append(Val_Str::mk(myAddress));
    tuple->append(Val_ID::mk(myKey));
    tuple->freeze();
    nodeTable->insert(tuple);
  }
    

  TablePtr bestSuccessorTable(new Table(strbuf("bestSuccessorTable"), 1));
  bestSuccessorTable->add_unique_index(1);
  
#ifdef __P2__WITH_CHURN__
  timespec successorExpiration;
  successorExpiration.tv_sec = SUCCEXPIRATION;
  successorExpiration.tv_nsec = 0;
  TablePtr successorTable(new Table(strbuf("successorTable"), 100,
                            &successorExpiration)); // let the
                                                   // replacement policy
                                                   // deal with
                                                   // evictions
#else
  TablePtr successorTable(new Table(strbuf("successorTable"), 100));
#endif

  successorTable->add_multiple_index(1);
  successorTable->add_unique_index(2);

  // Create the count aggregate on the unique index
  std::vector< unsigned > groupBy;
  groupBy.push_back(1);
  Table::MultAggregate successorCountAggregate =
    successorTable->add_mult_groupBy_agg(1, groupBy, 1, &Table::AGG_COUNT);

  // The next finger fix table starts out with a single tuple that never
  // expires and is only replaced
  TablePtr nextFingerFixTable(new Table(strbuf("nextFingerFix"), 1));
  nextFingerFixTable->add_unique_index(1);

  /** The finger lookup table.  It is indexed uniquely by its event ID */
  TablePtr fingerLookupTable(new Table(strbuf("fingerLookup"), 100));
  fingerLookupTable->add_unique_index(2);
  fingerLookupTable->add_multiple_index(1);

  // The predecessor table, indexed by its first field
  TablePtr predecessorTable(new Table(strbuf("predecessor"), 3));
  predecessorTable->add_unique_index(1);
  

  // The joinRecord table. Singleton
  TablePtr joinRecordTable(new Table(strbuf("joinRecord"), 100));
  joinRecordTable->add_unique_index(2);
  joinRecordTable->add_multiple_index(1);

  // The stabilizeRecord table. Singleton.
  TablePtr stabilizeRecordTable(new Table(strbuf("stabilizeRecord"), 2));
  stabilizeRecordTable->add_unique_index(1);

  // The landmarkNode table. Singleton
  TablePtr landmarkNodeTable(new Table(strbuf("landmarkNode"), 2));
  landmarkNodeTable->add_unique_index(1);
  


  

  

  ////////////////////////////////////////////////////////
  // The front end, UDP to rules engine
  ElementSpecPtr udpRxS = conf->addElement(udp->get_rx());
  ElementSpecPtr unmarshalS = conf->addElement(ElementPtr(new UnmarshalField(strbuf("Unmarshal:").cat(myAddress), 1)));


  // Drop the source address and decapsulate
  ElementSpecPtr unBoxS = conf->addElement(ElementPtr(new PelTransform(strbuf("UnBox:").cat(myAddress),
                                                                          "$1 unboxPop")));
  conf->hookUp(udpRxS, 0, unmarshalS, 0);
  conf->hookUp(unmarshalS, 0, unBoxS, 0);




  ///////////////////////////////////////////////////////
  // The back end, rules engine to UDP

  // Create the encapsulated version of this tuple, holding the
  // destination address and, encapsulated, the payload containing the
  // reach tuple.
  ElementSpecPtr encapS = conf->addElement(ElementPtr(new PelTransform(strbuf("encap:").cat(myAddress),
                                                                          "$1 pop /* The From address */\
                                                                           swallow pop /* The entire input tuple encapsulated */")));

  // Now marshall the payload (second field)
  ElementSpecPtr marshalS = conf->addElement(ElementPtr(new MarshalField(strbuf("Marshal:").cat(myAddress), 1)));

  // And translate the address into a sockaddr.  This is a hack until
  // we have a consistent view of the UDP interface.
  ElementSpecPtr routeS = conf->addElement(ElementPtr(new StrToSockaddr(strbuf("Router:").cat(myAddress), 0)));
  ElementSpecPtr udpTxS = conf->addElement(udp->get_tx());
  
  
  conf->hookUp(encapS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, routeS, 0);
  conf->hookUp(routeS, 0, udpTxS, 0);



  TuplePtr landmark = Tuple::mk();
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
