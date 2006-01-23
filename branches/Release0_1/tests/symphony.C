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
 * DESCRIPTION: A symphony dataflow.
 *
 */

#ifndef __SYMPHONY_H__
#define __SYMPHONY_H__


#ifdef FOOBAR  // FIX SHARED POINTERS AND STRINGS

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <stdlib.h>
#include <stdio.h> 

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


//static const int SYMFINGERSIZE = ID::WORDS * 32; // first finger is best successor, last finger is predecessor
static const int SYMFINGERSIZE = 5; // first finger is best successor, last finger is predecessor
static const string SYMFINGERSIZESTR("5"); // first finger is best successor, last finger is predecessor
static const int SYMQUEUE_LENGTH = 1000;
static const int SYMFINGERTTL = 30;

/**
 *rule L1 symLookupResults@R(R,K,S,SI,E) :- node@NI(NI,N),
 *symLookup@NI(NI,K,R,E), bestSuccessor@NI(NI,S,SI), K in (N,S].
 */
void ruleSymphonyL1(string name,
                    Plumber::ConfigurationRef conf,
                    TableRef nodeTable,
                    TableRef bestSuccessorTable,
                    ElementSpecRef pushSymLookupIn,
                    int pushSymLookupInPort,
                    ElementSpecRef pullSymLookupResultsOut,
                    int pullSymLookupResultsOutPort)
{
  // Join with node
  ElementSpecRef matchSymLookupIntoNodeS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("SymLookupInNode:") << name,
                                                    nodeTable,
                                                    1, // Match symLookup.NI
                                                    1, // with node.NI
                                                    cbv_null ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));
  // Link it to the symLookup coming in. Pushes match already
  conf->hookUp(pushSymLookupIn, pushSymLookupInPort, matchSymLookupIntoNodeS, 0);



  // Produce intermediate ephemeral result
  // res1(NI, K, R, E, N) from
  // <<symLookup NI K R E><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("MakeRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 unbox \
                                                     drop /* No need for symLookup literal */ \
                                                     pop pop pop pop /* all symLookup fields */ \
                                                     $1 2 field pop /* node.N */"));
  conf->hookUp(matchSymLookupIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  



  // Join with best successor table
  ElementSpecRef matchRes1IntoBestSuccS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("SymLookupInBestSucc:") << name,
                                                    bestSuccessorTable,
                                                    1, // Match res1.NI
                                                    1, // with bestSuccessor.NI
                                                    cbv_null ));
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
  

  // Project to create symLookupResults(R, K, S, SI, E)
  // from <<res1 NI, K, R, E, N><bestSuccessor NI S SI>>
  ElementSpecRef makeSymLookupResultS =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenSymLookupResult:").cat(name),
                                                    "\"symLookupResults\" pop \
                                                     $0 3 field pop /* output R */ \
                                                     $0 2 field pop /* output K */ \
                                                     $1 unbox drop drop pop pop /* output S SI */ \
                                                     $0 4 field pop /* output E */"));
  conf->hookUp(selectS, 0, makeSymLookupResultS, 0);
  conf->hookUp(makeSymLookupResultS, 0, pullSymLookupResultsOut, pullSymLookupResultsOutPort);
}




/** L2: bestSymLookupDistance@NI(NI,K,R,E,min<D>) :- symLookup@NI(NI,K,R,E),
    node@NI(NI, N), symFinger@NI(NI,I,B,BI,ET), B in (N,K), D=f_dist(B,K)-1
*/
void ruleSymphonyL2(str name,
            Plumber::ConfigurationRef conf,
            TableRef nodeTable,
            TableRef symFingerTable,
            ElementSpecRef pushSymLookupIn,
            int pushSymLookupInPort,
            ElementSpecRef pullBestSymLookupDistanceOut,
            int pullBestSymLookupDistanceOutPort)
{
  // Join with node
  ElementSpecRef matchSymLookupIntoNodeS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("SymLookupInNode:") << name,
                                                    nodeTable,
                                                    1, // Match symLookup.NI
                                                    1, // with node.NI
                                                    cbv_null ));
  // Link it to the symLookup coming in. Pushes match already
  conf->hookUp(pushSymLookupIn, pushSymLookupInPort, matchSymLookupIntoNodeS, 0);
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  // Produce intermediate ephemeral result
  // res1(NI, K, R, E, N) from
  // <<symLookup NI K R E><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 unbox \
                                                     drop /* No need for symLookup literal */ \
                                                     pop pop pop pop /* all symLookup fields */ \
                                                     $1 2 field pop /* node.N */"));
  conf->hookUp(matchSymLookupIntoNodeS, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  // Run aggregation over symFinger table with input
  // res1(NI, K, R, E, N) from
  ElementSpecRef findMinInSymFingerS =
    conf->addElement(New refcounted< PelScan >(str("bestSymLookupDistance:") << name,
                                               symFingerTable, 1,
                                               str("$1 /* res1.NI */ \
                                                    $2 /* NI res1.K */ \
                                                    $5 /* NI K res1.N */ \
                                                    0 /* NI K res1.N found? */\
                                                    1 ->u32 ->id 0 ->u32 ->id distance /* NI K N found? maxdist */"),
                                               str("4 peek /* NI */ \
                                                    $1 /* symFinger.NI */ \
                                                    ==s not /* res1.NI != symFinger.NI */ \
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
                                                    \"bestSymLookupDistance\" pop \
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

  conf->hookUp(pushRes1S, 0, findMinInSymFingerS, 0);

  conf->hookUp(findMinInSymFingerS, 0, noNull2S, 0);
  conf->hookUp(noNull2S, 0, pullBestSymLookupDistanceOut, pullBestSymLookupDistanceOutPort);
}







/** rule L3 symLookup@BI(max<BI>,K,R,E) :-
    bestSymLookupDistance@NI(NI,K,R,E,D), symFinger@NI(NI,I,B,BI),
    D=f_dist(B,K), node@NI(NI, N), B in (N, K).*/
void ruleSymphonyL3(str name,
            Plumber::ConfigurationRef conf,
            TableRef nodeTable,
            TableRef symFingerTable,
            ElementSpecRef pushBLDIn,
            int pushBLDInPort,
            ElementSpecRef pullSymLookupOut,
            int pullSymLookupOutPort)
{
  // Join with node
  ElementSpecRef matchBLDIntoNodeS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("BLDInNode:") << name,
                                                    nodeTable,
                                                    1, // Match symLookup.NI
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
  

  // Select out the min-BI for all the symFingers that satisfy
  // (symFinger.B in (res1.N, res1.K)) and (res1.D == distance(symFinger.B,
  // res1.K)-1) and (res1.NI == symFinger.NI)
  // from <Res1 NI K R E D N> input joined with
  // <SymFinger NI I B BI>>
  ElementSpecRef aggregateS =
    conf->addElement(New refcounted< PelScan >(str("tieBreaker:") << name,
                                               symFingerTable, 1,
                                               str("$1 /* res1.NI */ \
                                                    $5 /* NI res1.D */\
                                                    $2 /* NI D res1.K */ \
                                                    $6 /* NI D K res1.N */ \
                                                    0 /* NI D K N found?*/ \
                                                    \"\" /* NI D K N found? maxString */"),
                                               str("$3 /* symFinger.B */\
                                                    3 peek /* B res1.N */\
                                                    5 peek /* B N res1.K */\
                                                    ()id /* B in (N,K) */\
                                                    $3 /* B in (N,K) symFinger.B */\
                                                    5 peek /* B in (N,K) B res1.K */\
                                                    distance --id /* B in (N,K) dist(B,K) */\
                                                    6 peek /* B in (N,K) dist(B,K) res1.D */\
                                                    ==id /* B in (N,K) (dist==D) */\
                                                    and /* [B in (N,K) and (dist==D)] */\
                                                    6 peek /* [first and] res1.NI */\
                                                    $1 /* [first and] res1.NI symFinger.NI */\
                                                    ==s and /* select clause */\
                                                    not ifstop /* done with select */\
                                                    swap drop 1 swap /* replace found? with true in state */\
                                                    $4 dup /* symFinger.BI symFinger.BI */\
                                                    2 peek /* symFinger.BI symFinger.BI oldMax */\
                                                    >s /* symFinger.BI (symFinger.BI>oldMax?) */\
                                                    swap /* (BI>oldMax?) symFinger.BI */ \
                                                    2 peek ifelse /* ((new>old) ? new : old) */ \
                                                    swap /* swap newMax in state where oldMax was */ \
                                                    drop /* only state remains */"),
                                               str("swap not ifstop /* return nothing if none found */\
                                                    \"symLookup\" pop\
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
  conf->hookUp(noNullS, 0, pullSymLookupOut, pullSymLookupOutPort);
}




/** 
    rule SU3 symFinger@NI(NI,0,S,SI) :- bestSuccessor@NI(NI,S,SI).
*/
void ruleSymphonySU3(str name,
                     Plumber::ConfigurationRef conf,
                     ElementSpecRef pushBestSuccessorIn,
                     int pushBestSuccessorInPort,
                     ElementSpecRef pullSymFingerOut,
                     int pullSymFingerOutPort)
{
  // Project onto symFinger(NI, 0, B, BI)
  // from
  // bestSuccessor(NI, S, SI)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"symFinger\" pop \
                                                     $1 pop /* out bS.NI */\
                                                     0 pop /* out 0 */\
                                                     $2 pop /* out bS.S */\
                                                     $3 pop /* out bS.SI */\
                                                     "));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:") << name));
  conf->hookUp(pushBestSuccessorIn, pushBestSuccessorInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullSymFingerOut, pullSymFingerOutPort);
}



/** 
    rule SU3a symFinger@NI(NI,SYMFINGERSIZE,S,SI) :- predecessor@NI(NI,S,SI).
*/
void ruleSymphonySU3a(str name,
		      Plumber::ConfigurationRef conf,
		      ElementSpecRef pushPredecessorIn,
		      int pushBestSuccessorInPort,
		      ElementSpecRef pullSymFingerOut,
		      int pullSymFingerOutPort)
{
  // Project onto symFinger(NI, 0, B, BI)
  // from
  // bestSuccessor(NI, S, SI)
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"symFinger\" pop \
                                                     $1 pop /* out bS.NI */" 
						    << SYMFINGERSIZESTR <<
                                                     " pop /* out 0 */\
                                                     $2 pop /* out bS.S */\
                                                     $3 pop /* out bS.SI */\
                                                     "));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:") << name));
  conf->hookUp(pushPredecessorIn, pushBestSuccessorInPort, projectS, 0);
  conf->hookUp(projectS, 0, slotS, 0);
  conf->hookUp(slotS, 0, pullSymFingerOut, pullSymFingerOutPort);
}




/** rule F1 symFixFinger@NI(ni) :- periodic@NI(finger.TTL*0.5). */
void ruleSymphonyF1(str name,
		    Plumber::ConfigurationRef conf,
		    str localAddress,
		    double fingerTTL,
		    ElementSpecRef pullFixFingerOut,
		    int pullFixFingerOutPort)
{
  // My fix finger tuple
  TupleRef fixFingerTuple = Tuple::mk();
  fixFingerTuple->append(Val_Str::mk("symFixFinger"));
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



/** rule F2 nextSymFingerFix@NI(ni, 1). */
void ruleSymphonyF2(str name,
		    Plumber::ConfigurationRef conf,
		    str localAddress,
		    ElementSpecRef pullNextFingerFixOut,
		    int pullNextFingerFixOutPort)
{
  // My next finger fix tuple
  TupleRef nextFingerFixTuple = Tuple::mk();
  nextFingerFixTuple->append(Val_Str::mk("nextSymFingerFix"));
  nextFingerFixTuple->append(Val_Str::mk(localAddress));
  nextFingerFixTuple->append(Val_Int32::mk(1)); // start fixing from finger 1
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




/** rule F3 symFingerLookup@NI(NI, E, 1) :- fixFinger@NI(NI), E = random(),
    nextSymFingerFix@NI(NI, 1).
*/
void ruleSymphonyF3(str name,
            Plumber::ConfigurationRef conf,
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
                                                    "\"symFingerLookup\" pop \
                                                     $0 1 field pop /* output fixFinger.NI */ \
                                                     $0 1 field \":\" strcat rand strcat pop /* output NI|rand */ \
                                                     $1 2 field pop /* output nextFingerFix.I */"));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  conf->hookUp(makeRes1S, 0, pullFingerLookupOut, pullFingerLookupOutPort);
}


/** rule F4 symLookup@NI(NI, K, NI, E) :- symFingerLookup@NI(NI, E, I),
    node(NI, N), K = N + 1 << I.
*/
void ruleSymphonyF4(str name,
            Plumber::ConfigurationRef conf,
            TableRef nodeTable,
            ElementSpecRef pushFingerLookupIn,
            int pushFingerLookupInPort,
            ElementSpecRef pullLookupOut,
            int pullLookupOutPort, int networkSize)
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
  // <<symFingerLookup NI E I><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("lookup:").cat(name),
						    "\"symLookup\" pop \
                                                    $0 1 field pop /* output fixFinger.NI */\
                                                    $1 2 field /* node.N */\
						    rand ->u32 ->id +id pop \
						    $0 1 field pop /* output fixFinger.NI again */\
						    $0 2 field pop /* output fingerLookup.E */"));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  conf->hookUp(makeRes1S, 0, pullLookupOut, pullLookupOutPort);
}




/** 
    rule F5 symFingers@NI(NI, I, B, BI) :- symFingerLookup@NI(NI, E, I), 
    symLookupResults@NI(NI, K, B, BI, E).
*/
void ruleSymphonyF5(str name,
		    Plumber::ConfigurationRef conf,
		    TableRef symFingerLookupTable,
		    ElementSpecRef pushLookupResultsIn,
		    int pushLookupResultsInPort,
		    ElementSpecRef pullEagerFingerOut,
		    int pullEagerFingerOutPort)
{
  // Join lookupResults with fingerLookup table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< MultLookup >(strbuf("symLookupResultsIntoSymFingerLookup:") << name,
                                                  symFingerLookupTable,
                                                  1, // Match lookupResults.NI
                                                  1 // with fingerLookup.NI
                                                  ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  conf->hookUp(pushLookupResultsIn, pushLookupResultsInPort, join1S, 0);

  // Select fingerLookup.E == lookupResults.E
  // <<lookupResults NI K B BI E><fingerLookup NI E I>>
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("select:") << name,
                                                    "$0 5 field /* symLookupResults.E */\
                                                     $1 2 field /* lR.E symFingerLookup.E */\
                                                     ==s not /* lR.E!=fL.E? */\
                                                     ifstop /* empty */\
                                                     $0 pop $1 pop /* pass through */\
                                                     "));

  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, selectS, 0);



  // Produce symFingers
  // symFingers(NI, I, B, BI) from
  // <<lookupResults NI K B BI E><fingerLookup NI E I>>
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"symFingers\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $1 3 field pop /* out I */\
                                                     $0 3 field pop /* out B */\
                                                     $0 4 field pop /* out BI */\
                                                     "));
  conf->hookUp(selectS, 0, projectS, 0);

  conf->hookUp(projectS, 0, pullEagerFingerOut, pullEagerFingerOutPort);
}




/** 
    rule F6 symFingerLookup@NI(NI, E, I+1) :- symFingerLookup@NI(NI, E, I), 
    symLookupResults@NI(NI, K, B, BI, E), I < symFingersize.SIZE-1
*/

void ruleSymphonyF6(str name,
		    Plumber::ConfigurationRef conf,
		    TableRef symFingerLookupTable,
		    ElementSpecRef pushLookupResultsIn,
		    int pushLookupResultsInPort,
		    ElementSpecRef pullSymLookupOut,
		    int pullSymLookupOutPort)
{
  // Join lookupResults with fingerLookup table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< MultLookup >(strbuf("symLookupResultsIntoSymFingerLookup:") << name,
                                                  symFingerLookupTable,
                                                  1, // Match lookupResults.NI
                                                  1 // with fingerLookup.NI
                                                  ));
  ElementSpecRef noNullS = conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << name, 1));

  conf->hookUp(pushLookupResultsIn, pushLookupResultsInPort, join1S, 0);

  // Select fingerLookup.E == lookupResults.E
  // <<lookupResults NI K B BI E><fingerLookup NI E I>>
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("select:") << name,
                                                    strbuf("$0 5 field /* symLookupResults.E */\
                                                     $1 2 field /* lR.E symFingerLookup.E */\
                                                     ==s not /* lR.E!=fL.E? */\
                                                     ifstop /* empty */\
                                                     $1 3 field ") 
						    << SYMFINGERSIZESTR << strbuf(" 1 -i >=i ifstop \
						    $0 pop $1 pop /* pass through */\
                                                     ")));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, selectS, 0);



  // Produce eagerFinger
  // symLookup(NI, I+1, B, BI) from
  // <<lookupResults NI K B BI E><fingerLookup NI E I>>
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"symFingerLookup\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $0 1 field \":\" strcat rand strcat pop /* output NI|rand */ \
                                                     $1 3 field 1 +i pop"));      

  conf->hookUp(selectS, 0, projectS, 0);

  conf->hookUp(projectS, 0, pullSymLookupOut, pullSymLookupOutPort);
}











/** rule J1 join@NI(NI,E) :- joinEvent@NI(NI), E=f_rand(). */
void ruleSymphonyJ1(str name,
            Plumber::ConfigurationRef conf,
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
void ruleSymphonyJ1a(str name,
             Plumber::ConfigurationRef conf,
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
                                                     10 // run once
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
void ruleSymphonyJ2(str name,
            Plumber::ConfigurationRef conf,
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
void ruleSymphonyJ3(str name,
            Plumber::ConfigurationRef conf,
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


/** rule J4 symLookup@LI(LI,N,NI,E) :- startJoin@LI(LI,N,NI,E).
 */
void ruleSymphonyJ4(str name,
            Plumber::ConfigurationRef conf,
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
                                                    "\"symLookup\" pop \
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
void ruleSymphonyJ5(str name,
            Plumber::ConfigurationRef conf,
            TableRef joinRecordTable,
            ElementSpecRef pushLookupResultsIn,
            int pushLookupResultsInPort,
            ElementSpecRef pullSuccessorOut,
            int pullSuccessorOutPort)
{
  // Join lookupResults with joinRecord table
  ElementSpecRef join1S =
    conf->addElement(New refcounted< MultLookup >(strbuf("symLookupResultsIntoJoinRecord:") << name,
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
                                                    "$0 5 field /* symLookupResults.E */\
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
void ruleSymphonyJ6(str name,
            Plumber::ConfigurationRef conf,
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
void ruleSymphonyJ7(str name,
            Plumber::ConfigurationRef conf,
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




void
connectRulesSymphony(str name,
                     str localAddress,
                     Plumber::ConfigurationRef conf,
                     TableRef bestSuccessorTable,
                     TableRef symFingerLookupTable,
                     TableRef symFingerTable,
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
		     int networkSize,
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
  demuxKeys->push_back(New refcounted< Val_Str >(str("symLookup")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("bestSymLookupDistance")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("bestSuccessorDist")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("maxSuccessorDist")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("evictSuccessor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("successorCount")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("node")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("symFinger")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("predecessor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("bestSuccessor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("nextSymFingerFix")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("symFingerLookup")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("stabilizeRecord")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("symFixFinger"))); // same as Chord
  demuxKeys->push_back(New refcounted< Val_Str >(str("symLookupResults")));
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
  //demuxKeys->push_back(New refcounted< Val_Str >(str("eagerFinger")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("sendSuccessors")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("returnSuccessor")));
  ElementSpecRef demuxS = conf->addElement(New refcounted< Demux >("demux", demuxKeys));
  conf->hookUp(wrapAroundMux, 0, demuxS, 0);

  int nextDemuxOutput = 0;
  // Create the duplicator for each tuple name.  Store the tuple first
  // for materialized tuples
  ElementSpecRef dupSuccessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("successor") << "Dup:" << name, 1));
  ElementSpecRef insertSuccessor = conf->addElement(New refcounted< Insert >(strbuf("successor") << "Insert:" << name, successorTable));
  conf->hookUp(demuxS, nextDemuxOutput++, insertSuccessor, 0);
  conf->hookUp(insertSuccessor, 0, dupSuccessor, 0);

  ElementSpecRef dupSymLookup = conf->addElement(New refcounted< DuplicateConservative >(strbuf("symLookup") << "Dup:" << name, 2));
  ElementSpecRef qSymLookup = conf->addElement(New refcounted< Queue >("symLookupQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPSymLookup = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qSymLookup, 0);
  conf->hookUp(qSymLookup, 0, tPPSymLookup, 0);
  conf->hookUp(tPPSymLookup, 0, dupSymLookup, 0);

  ElementSpecRef dupBestSymLookupDistance = conf->addElement(New refcounted< DuplicateConservative >(strbuf("bestSymLookupDistance") << "Dup:" << name, 1));
  ElementSpecRef qBestSymLookupDistance = conf->addElement(New refcounted< Queue >("BestSymLookupDistance", SYMQUEUE_LENGTH));
  ElementSpecRef tPPBestSymLookupDistance = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qBestSymLookupDistance, 0);
  conf->hookUp(qBestSymLookupDistance, 0, tPPBestSymLookupDistance, 0);
  conf->hookUp(tPPBestSymLookupDistance, 0, dupBestSymLookupDistance, 0);

  ElementSpecRef dupBestSuccessorDistance = conf->addElement(New refcounted< DuplicateConservative >(strbuf("bestSuccessorDist") << "Dup:" << name, 1));
  ElementSpecRef qBestSuccessorDistance = conf->addElement(New refcounted< Queue >("BestSuccessorDistanceQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPBestSuccessorDistance = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qBestSuccessorDistance, 0);
  conf->hookUp(qBestSuccessorDistance, 0, tPPBestSuccessorDistance, 0);
  conf->hookUp(tPPBestSuccessorDistance, 0, dupBestSuccessorDistance, 0);

  ElementSpecRef dupMaxSuccessorDist = conf->addElement(New refcounted< DuplicateConservative >(strbuf("maxSuccessorDist") << "Dup:" << name, 1));
  ElementSpecRef qMaxSuccessorDist = conf->addElement(New refcounted< Queue >("maxSuccessorDistQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPMaxSuccessorDist = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qMaxSuccessorDist, 0);
  conf->hookUp(qMaxSuccessorDist, 0, tPPMaxSuccessorDist, 0);
  conf->hookUp(tPPMaxSuccessorDist, 0, dupMaxSuccessorDist, 0);

  ElementSpecRef dupEvictSuccessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("evictSuccessor") << "Dup:" << name, 1));
  ElementSpecRef qEvictSuccessor = conf->addElement(New refcounted< Queue >("evictSuccessorQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPEvictSuccessor = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qEvictSuccessor, 0);
  conf->hookUp(qEvictSuccessor, 0, tPPEvictSuccessor, 0);
  conf->hookUp(tPPEvictSuccessor, 0, dupEvictSuccessor, 0);

  ElementSpecRef dupSuccessorCount = conf->addElement(New refcounted< DuplicateConservative >(strbuf("successorCount") << "Dup:" << name, 1));
  ElementSpecRef qSuccessorCount = conf->addElement(New refcounted< Queue >("successorCountQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPSuccessorCount = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qSuccessorCount, 0);
  conf->hookUp(qSuccessorCount, 0, tPPSuccessorCount, 0);
  conf->hookUp(tPPSuccessorCount, 0, dupSuccessorCount, 0);

  ElementSpecRef insertNode = conf->addElement(New refcounted< Insert >(strbuf("node") << "Insert:" << name, nodeTable));
  ElementSpecRef discardNode = conf->addElement(New refcounted< Discard >(strbuf("node") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertNode, 0);
  conf->hookUp(insertNode, 0, discardNode, 0);

  ElementSpecRef insertFinger = conf->addElement(New refcounted< Insert >(strbuf("finger") << "Insert:" << name, symFingerTable));
  ElementSpecRef discardFinger = conf->addElement(New refcounted< Discard >(strbuf("finger") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertFinger, 0);
  conf->hookUp(insertFinger, 0, discardFinger, 0);
 
  ElementSpecRef insertPredecessor = conf->addElement(New refcounted< Insert >(strbuf("predecessor") << "Insert:" << name, predecessorTable));
  //ElementSpecRef discardPredecessor = conf->addElement(New refcounted< Discard >(strbuf("predecessor") << "Discard:" << name));
  ElementSpecRef dupPredecessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("predecessor") << "Dup:" << name, 1));
  ElementSpecRef qPredecessor = conf->addElement(New refcounted< Queue >("predecessor", SYMQUEUE_LENGTH));
  ElementSpecRef tPPPredecessor = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, insertPredecessor, 0);
  conf->hookUp(insertPredecessor, 0, qPredecessor, 0);
  conf->hookUp(qPredecessor, 0,tPPPredecessor, 0);
  conf->hookUp(tPPPredecessor, 0,dupPredecessor, 0);

  ElementSpecRef insertBestSuccessor = conf->addElement(New refcounted< Insert >(strbuf("bestSuccessor") << "Insert:" << name, bestSuccessorTable));
  ElementSpecRef dupBestSuccessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("bestSuccessor") << "Dup:" << name, 1));
  ElementSpecRef qBestSuccessor = conf->addElement(New refcounted< Queue >("bestSuccessorQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPBestSuccessor = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, insertBestSuccessor, 0);
  conf->hookUp(insertBestSuccessor, 0, qBestSuccessor, 0);
  conf->hookUp(qBestSuccessor, 0, tPPBestSuccessor, 0);
  conf->hookUp(tPPBestSuccessor, 0, dupBestSuccessor, 0);

  ElementSpecRef insertNextFingerFix = conf->addElement(New refcounted< Insert >(strbuf("nextFingerFix") << "Insert:" << name, nextFingerFixTable));
  ElementSpecRef discardNextFingerFix = conf->addElement(New refcounted< Discard >(strbuf("nextFingerFix") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertNextFingerFix, 0);
  conf->hookUp(insertNextFingerFix, 0, discardNextFingerFix, 0);

  ElementSpecRef insertFingerLookup = conf->addElement(New refcounted< Insert >(strbuf("fingerLookup") << "Insert:" << name, symFingerLookupTable));
  ElementSpecRef dupFingerLookup = conf->addElement(New refcounted< DuplicateConservative >(strbuf("fingerLookup") << "Dup:" << name, 1));
  ElementSpecRef qFingerLookup = conf->addElement(New refcounted< Queue >("fingerLookupQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPFingerLookup = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, insertFingerLookup, 0);
  conf->hookUp(insertFingerLookup, 0, qFingerLookup, 0);
  conf->hookUp(qFingerLookup, 0, tPPFingerLookup, 0);
  conf->hookUp(tPPFingerLookup, 0, dupFingerLookup, 0);

  ElementSpecRef insertStabilizeRecord = conf->addElement(New refcounted< Insert >(strbuf("stabilizeRecord") << "Insert:" << name, stabilizeRecordTable));
  ElementSpecRef discardStabilizeRecord = conf->addElement(New refcounted< Discard >(strbuf("stabilizeRecord") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertStabilizeRecord, 0);
  conf->hookUp(insertStabilizeRecord, 0, discardStabilizeRecord, 0);

  ElementSpecRef dupFixFinger = conf->addElement(New refcounted< DuplicateConservative >(strbuf("symFixFinger") << "Dup:" << name, 1));
  ElementSpecRef qFixFinger = conf->addElement(New refcounted< Queue >("symFixFingerQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPFixFinger = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qFixFinger, 0);
  conf->hookUp(qFixFinger, 0, tPPFixFinger, 0);
  conf->hookUp(tPPFixFinger, 0, dupFixFinger, 0);

  ElementSpecRef dupSymLookupResults = conf->addElement(New refcounted< DuplicateConservative >(strbuf("symLookupResults") << "Dup:" << name, 3));
  ElementSpecRef qSymLookupResults = conf->addElement(New refcounted< Queue >("SymLookupResultsQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPSymLookupResults = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qSymLookupResults, 0);
  conf->hookUp(qSymLookupResults, 0, tPPSymLookupResults, 0);
  conf->hookUp(tPPSymLookupResults, 0, dupSymLookupResults, 0);

  ElementSpecRef dupJoin = conf->addElement(New refcounted< DuplicateConservative >(strbuf("join") << "Dup:" << name, 3));
  ElementSpecRef qJoin = conf->addElement(New refcounted< Queue >("JoinQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPJoin = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qJoin, 0);
  conf->hookUp(qJoin, 0, tPPJoin, 0);
  conf->hookUp(tPPJoin, 0, dupJoin, 0);

  ElementSpecRef dupJoinEvent = conf->addElement(New refcounted< DuplicateConservative >(strbuf("joinEvent") << "Dup:" << name, 1));
  ElementSpecRef qJoinEvent = conf->addElement(New refcounted< Queue >("joinEventQueue", SYMQUEUE_LENGTH));
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
  ElementSpecRef qStartJoin = conf->addElement(New refcounted< Queue >("startJoinQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPStartJoin = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qStartJoin, 0);
  conf->hookUp(qStartJoin, 0, tPPStartJoin, 0);
  conf->hookUp(tPPStartJoin, 0, dupStartJoin, 0);

  ElementSpecRef dupStabilize = conf->addElement(New refcounted< DuplicateConservative >(strbuf("stabilize") << "Dup:" << name, 3));
  ElementSpecRef qStabilize = conf->addElement(New refcounted< Queue >("StabilizeQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPStabilize = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qStabilize, 0);
  conf->hookUp(qStabilize, 0, tPPStabilize, 0);
  conf->hookUp(tPPStabilize, 0, dupStabilize, 0);

  ElementSpecRef dupStabilizeEvent = conf->addElement(New refcounted< DuplicateConservative >(strbuf("stabilizeEvent") << "Dup:" << name, 1));
  ElementSpecRef qStabilizeEvent = conf->addElement(New refcounted< Queue >("stabilizeEventQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPStabilizeEvent = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qStabilizeEvent, 0);
  conf->hookUp(qStabilizeEvent, 0, tPPStabilizeEvent, 0);
  conf->hookUp(tPPStabilizeEvent, 0, dupStabilizeEvent, 0);

  ElementSpecRef dupStabilizeRequest = conf->addElement(New refcounted< DuplicateConservative >(strbuf("stabilizeRequest") << "Dup:" << name, 1));
  ElementSpecRef qStabilizeRequest = conf->addElement(New refcounted< Queue >("stabilizeRequestQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPStabilizeRequest = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qStabilizeRequest, 0);
  conf->hookUp(qStabilizeRequest, 0, tPPStabilizeRequest, 0);
  conf->hookUp(tPPStabilizeRequest, 0, dupStabilizeRequest, 0);

  ElementSpecRef dupSendPredecessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("sendPredecessor") << "Dup:" << name, 1));
  ElementSpecRef qSendPredecessor = conf->addElement(New refcounted< Queue >("sendPredecessorQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPSendPredecessor = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qSendPredecessor, 0);
  conf->hookUp(qSendPredecessor, 0, tPPSendPredecessor, 0);
  conf->hookUp(tPPSendPredecessor, 0, dupSendPredecessor, 0);

  ElementSpecRef dupNotify = conf->addElement(New refcounted< DuplicateConservative >(strbuf("notify") << "Dup:" << name, 1));
  ElementSpecRef qNotify = conf->addElement(New refcounted< Queue >("notifyQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPNotify = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qNotify, 0);
  conf->hookUp(qNotify, 0, tPPNotify, 0);
  conf->hookUp(tPPNotify, 0, dupNotify, 0);

  ElementSpecRef dupNotifyPredecessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("notifyPredecessor") << "Dup:" << name, 1));
  ElementSpecRef qNotifyPredecessor = conf->addElement(New refcounted< Queue >("notifyPredecessorQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPNotifyPredecessor = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qNotifyPredecessor, 0);
  conf->hookUp(qNotifyPredecessor, 0, tPPNotifyPredecessor, 0);
  conf->hookUp(tPPNotifyPredecessor, 0, dupNotifyPredecessor, 0);

  /*ElementSpecRef dupEagerFinger = conf->addElement(New refcounted< DuplicateConservative >(strbuf("eagerFinger") << "Dup:" << name, 4));
  ElementSpecRef qEagerFinger = conf->addElement(New refcounted< Queue >("EagerFingerQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPEagerFinger = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qEagerFinger, 0);
  conf->hookUp(qEagerFinger, 0, tPPEagerFinger, 0);
  conf->hookUp(tPPEagerFinger, 0, dupEagerFinger, 0);*/

  ElementSpecRef dupSendSuccessors = conf->addElement(New refcounted< DuplicateConservative >(strbuf("sendSuccessors") << "Dup:" << name, 1));
  ElementSpecRef qSendSuccessors = conf->addElement(New refcounted< Queue >("sendSuccessorsQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPSendSuccessors = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qSendSuccessors, 0);
  conf->hookUp(qSendSuccessors, 0, tPPSendSuccessors, 0);
  conf->hookUp(tPPSendSuccessors, 0, dupSendSuccessors, 0);

  ElementSpecRef dupReturnSuccessor = conf->addElement(New refcounted< DuplicateConservative >(strbuf("returnSuccessor") << "Dup:" << name, 1));
  ElementSpecRef qReturnSuccessor = conf->addElement(New refcounted< Queue >("returnSuccessorQueue", SYMQUEUE_LENGTH));
  ElementSpecRef tPPReturnSuccessor = conf->addElement(New refcounted< TimedPullPush >(strbuf("TPP") << name, 0));
  conf->hookUp(demuxS, nextDemuxOutput++, qReturnSuccessor, 0);
  conf->hookUp(qReturnSuccessor, 0, tPPReturnSuccessor, 0);
  conf->hookUp(tPPReturnSuccessor, 0, dupReturnSuccessor, 0);








  // Tuples that match nothing
  ElementSpecRef discardDefault = conf->addElement(New refcounted< Discard >(strbuf("DEFAULT") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, discardDefault, 0);



  int roundRobinPortCounter = 0;
  ElementSpecRef roundRobin = conf->addElement(New refcounted< RoundRobin >(strbuf("RoundRobin:") << name, 36));
  // we reduce because no connect fix fingers
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




  /** Keep the lookup rules */
  ruleSymphonyL1(strbuf(name) << ":SymL1",
         conf,
         nodeTable,
         bestSuccessorTable,
         dupSymLookup, 0,
         roundRobin, roundRobinPortCounter++);
  ruleSymphonyL2(strbuf(name) << ":SymL2",
         conf,
         nodeTable,
         symFingerTable,
         dupSymLookup, 1,
         roundRobin, roundRobinPortCounter++);
  ruleSymphonyL3(strbuf(name) << ":SymL3",
         conf,
         nodeTable,
         symFingerTable,
         dupBestSymLookupDistance, 0,
         roundRobin, roundRobinPortCounter++);


  /* Successor rules eventually have to be factored away and connected by ring.C */
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
  ruleSymphonySU3(strbuf(name) << ":SymSU3",
                  conf,
                  dupBestSuccessor, 0,
                  roundRobin, roundRobinPortCounter++);

  ruleSymphonySU3a(strbuf(name) << ":SymSU3",
		   conf,
		   dupPredecessor, 0,
		   roundRobin, 
		   roundRobinPortCounter++);

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

  /* Finger rules for now */  
  ruleSymphonyF1(strbuf(name) << ":F1",
         conf,
         localAddress,
         SYMFINGERTTL,
         roundRobin, roundRobinPortCounter++);
  ruleSymphonyF2(strbuf(name) << ":F2",
         conf,
         localAddress,
         roundRobin, roundRobinPortCounter++);
  ruleSymphonyF3(strbuf(name) << ":F3",
         conf,
         nextFingerFixTable,
         dupFixFinger, 0,
         roundRobin, roundRobinPortCounter++);
  ruleSymphonyF4(strbuf(name) << ":F4",
		 conf,
		 nodeTable,
		 dupFingerLookup, 0,
		 roundRobin, roundRobinPortCounter++, 
		 networkSize);
  ruleSymphonyF5(strbuf(name) << ":F5",
         conf,
         symFingerLookupTable,
         dupSymLookupResults, 0,
         roundRobin, roundRobinPortCounter++);
  ruleSymphonyF6(strbuf(name) << ":F6",
		 conf,
		 symFingerLookupTable,
		 dupSymLookupResults, 1,
		 roundRobin, roundRobinPortCounter++);

  ruleSymphonyJ1(strbuf(name) << ":J1",
                 conf,
                 dupJoinEvent, 0,
                 roundRobin, roundRobinPortCounter++);
  ruleSymphonyJ1a(strbuf(name) << ":J1a",
                  conf,
                  localAddress,
                  delay,
                  roundRobin, roundRobinPortCounter++);
  ruleSymphonyJ2(strbuf(name) << ":J2",
                 conf,
                 dupJoin, 0,
                 roundRobin, roundRobinPortCounter++);
  ruleSymphonyJ3(strbuf(name) << ":J3",
                 conf,
                 landmarkNodeTable,
                 nodeTable,
                 dupJoin, 1,
                 roundRobin, roundRobinPortCounter++);
  ruleSymphonyJ4(strbuf(name) << ":J4",
                 conf,
                 dupStartJoin, 0,
                 roundRobin, roundRobinPortCounter++);
  ruleSymphonyJ5(strbuf(name) << ":J5",
                 conf,
                 joinRecordTable,
                 dupSymLookupResults, 2, 
                 roundRobin, roundRobinPortCounter++);
  ruleSymphonyJ6(strbuf(name) << ":J6",
                 conf,
                 localAddress,
                 roundRobin, roundRobinPortCounter++);
  ruleSymphonyJ7(strbuf(name) << ":J7",
                 conf,
                 landmarkNodeTable,
                 nodeTable,
                 dupJoin, 2,
                 roundRobin, roundRobinPortCounter++);


  /** Eventually, we will put the hooking up part inside ring.C */
  ruleS0(strbuf(name) << ":S0",
         conf,
         localAddress,
         SYMFINGERTTL,
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
          SYMFINGERTTL,
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

  


void createSymNode(str myAddress,
		   str landmarkAddress,
		   Plumber::ConfigurationRef conf,
		   Udp* udp,
		   int networkSize,
		   double delay = 0)
{
  TableRef symFingerTable =
      New refcounted< Table >(strbuf("symFingerTable"), SYMFINGERSIZE + 1); // add one for the predecessor
  symFingerTable->add_unique_index(2);
  symFingerTable->add_multiple_index(1);
  
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
  
  TableRef successorTable =
    New refcounted< Table >(strbuf("successorTable"), 100);

  successorTable->add_multiple_index(1);
  successorTable->add_unique_index(2);

  // Create the count aggregate on the unique index
  std::vector< unsigned > groupBy;
  groupBy.push_back(1);
  Table::MultAggregate successorCountAggregate =
    successorTable->add_mult_groupBy_agg(1, groupBy, 1, &Table::AGG_COUNT);

  // The next finger fix table starts out with a single tuple that never
  // expires and is only replaced
  TableRef nextFingerFixTable = New refcounted< Table >(strbuf("nextSymFingerFix"), 1);
  nextFingerFixTable->add_unique_index(1);

  /** The finger lookup table.  It is indexed uniquely by its event ID */
  TableRef symFingerLookupTable = New refcounted< Table >(strbuf("symFingerLookup"), 100);
  symFingerLookupTable->add_unique_index(2);
  symFingerLookupTable->add_multiple_index(1);

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

  // The network size table. Singleton. 
  TableRef networkSizeTable = New refcounted< Table >(strbuf("networkSize"), 2);
  networkSizeTable->add_unique_index(1);
  


  

  

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


  TupleRef networkSizeTuple = Tuple::mk();
  networkSizeTuple->append(Val_Str::mk("networkSize"));
  networkSizeTuple->append(Val_Str::mk(myAddress));
  networkSizeTuple->append(Val_Int32::mk(networkSize));
  networkSizeTuple->freeze();
  networkSizeTable->insert(networkSizeTuple);

  
  connectRulesSymphony(strbuf("[") << myAddress << str("]"),
		       myAddress,
		       conf,
		       bestSuccessorTable,
		       symFingerLookupTable,
		       symFingerTable,
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
		       networkSize,
		       delay);         // not alone

}

#endif /*FIX SHARED POINTERS*/

/*
 * End of file 
 */
#endif /* __SYMPHONY_H_ */
