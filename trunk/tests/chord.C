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
#include "dupElim.h"
#include "filter.h"
#include "timedPullPush.h"
#include "udp.h"
#include "marshalField.h"
#include "unmarshalField.h"
#include "mux.h"
#include "strToSockaddr.h"
#include "slot.h"
#include "timedPullSink.h"
#include "timestampSource.h"
#include "hexdump.h"
#include "table.h"
#include "lookup.h"
#include "insert.h"
#include "scan.h"
#include "pelScan.h"
#include "functorSource.h"

void killJoin()
{
  exit(0);
}


static const uint FINGERS = 16;
str LOCAL("Local.com");
str REMOTE("Remote.com");
str FINGERIP("Finger.com");

struct LocalLookupGenerator : public FunctorSource::Generator
{
  virtual ~LocalLookupGenerator() {};
  TupleRef operator()() const {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("Lookup"));
    str address = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(address));

    uint zerowords = (uint) ((((double) (ID::WORDS + 1)) * rand()) / RAND_MAX);
    uint32_t words[ID::WORDS];
    for (uint i = 0;
         i < zerowords;
         i++) {
      words[i] = 0;
    }
    for (uint i = zerowords;
         i < ID::WORDS;
         i++) {
      words[i] = rand();
    }
    IDRef key = ID::mk(words);
    tuple->append(Val_ID::mk(key));

    str req = str(strbuf() << REMOTE);
    tuple->append(Val_Str::mk(req));

    tuple->append(Val_UInt32::mk(rand())); // the event ID

    IDRef me = ID::mk((uint32_t) 1);
    tuple->append(Val_ID::mk(me));

    tuple->freeze();

    return tuple;
  }
};

struct LocalLookupGenerator localLookupGenerator;


struct LookupGenerator : public FunctorSource::Generator
{
  virtual ~LookupGenerator() {};
  TupleRef operator()() const {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("Lookup"));
    str address = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(address));

    uint zerowords = (uint) ((((double) (ID::WORDS + 1)) * rand()) / RAND_MAX);
    uint32_t words[ID::WORDS];
    for (uint i = 0;
         i < zerowords;
         i++) {
      words[i] = 0;
    }
    for (uint i = zerowords;
         i < ID::WORDS;
         i++) {
      words[i] = rand();
    }
    IDRef key = ID::mk(words)->add(ID::mk((uint) 10));  // Adding 10 to
                                                        // have some
                                                        // lookups locally
                                                        // satisfiable
    tuple->append(Val_ID::mk(key));

    str req = str(strbuf() << REMOTE);
    tuple->append(Val_Str::mk(req));

    tuple->append(Val_UInt32::mk(rand())); // the event ID

    tuple->freeze();

    return tuple;
  }
};

struct LookupGenerator lookupGenerator;


/** Test best lookup distance. */
void testBestLookupDistance(LoggerI::Level level)
{
  TableRef fingers =
    New refcounted< Table >(strbuf("Fingers"), 10000);
  fingers->add_multiple_index(1);
  
  IDRef me = ID::mk((uint32_t) 1);

  // Fill up the table with fingers
  for (uint i = 0;
       i < FINGERS;
       i++) {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("Finger"));

    str myAddress = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(myAddress));

    IDRef target = ID::mk((uint32_t) 0X200)->shift(10 * i)->add(me);
    tuple->append(Val_ID::mk(target));

    IDRef best = ID::mk()->add(target)->add(ID::mk((uint32_t) i*10));
    tuple->append(Val_ID::mk(best));
  
    str address = str(strbuf() << FINGERIP << ":" << i);
    tuple->append(Val_Str::mk(address));
    tuple->freeze();
    fingers->insert(tuple);
    warn << tuple->toString() << "\n";
  }
  
  // Create the data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  
  ElementSpecRef funkyS =
    conf->addElement(New refcounted< FunctorSource >(str("Source"),
                                                     &localLookupGenerator));
  ElementSpecRef lookupPS =
    conf->addElement(New refcounted< Print >(strbuf("Lookup")));
  ElementSpecRef timedPullPushS =
    conf->addElement(New refcounted< TimedPullPush >("PullPush", 1));
  ElementSpecRef bestDistanceScannerS =
    conf->addElement(New refcounted< PelScan >(str("BestDistance"), fingers, 2,
                                               str("$1 /* NI */\
                                                    $2 /* NI K */\
                                                    $5 /* NI K N */\
                                                    1 ->u32 ->id 0 ->u32 ->id distance /* NI K N minDist=MaxDist */"),
                                               str("3 peek /* NI */ \
                                                    $1 /* finger.NI */ \
                                                    ==s not /* res1.NI != finger.NI */ \
                                                    ifstop /* empty */ \
                                                    $3 /* B */ \
                                                    2 peek /* B N */ \
                                                    4 peek /* B N K */ \
                                                    ()id  /* (B in (N,K)) */ \
                                                    not ifstop /* empty */ \
                                                    $3 3 peek distance dup /* dist(B,K) twice */ \
                                                    2 peek /* newDist newDist oldDist */ \
                                                    <id /* newDist (newDist<oldMin) */ \
                                                    swap /* (old>new?) newDist */ \
                                                    2 peek ifelse /* ((old>new) ? new : old) */ \
                                                    swap /* replace newMin in state where oldMin was */ \
                                                    drop /* only state remains */"),
                                               str("\"BestLookupDistance\" pop $1 pop /* output NI */\
                                                   $2 pop /* output K */\
                                                   $3 pop /* output R */\
                                                   $4 pop /* output E */\
                                                   pop /* output minDist */\
                                                   drop drop drop /* empty the stack */")));

  ElementSpecRef bestDistancePS =
    conf->addElement(New refcounted< Print >(strbuf("BestDistancePrint")));
  ElementSpecRef sinkS =
    conf->addElement(New refcounted< TimedPullSink >("Sink", 0));
  

  conf->hookUp(funkyS, 0, lookupPS, 0);
  conf->hookUp(lookupPS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, bestDistanceScannerS, 0);
  conf->hookUp(bestDistanceScannerS, 0, bestDistancePS, 0);
  conf->hookUp(bestDistancePS, 0, sinkS, 0);


  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized network of reachability flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the router
  router->activate();

  // Schedule kill
  //delaycb(10, 0, wrap(&killJoin));

  // Run the router
  amain();
  
}


/**
 * L1: lookupResults@R(R,K,S,SI,E) :- node@NI(NI,N),
 * lookup@NI(NI,K,R,E), bestSuccessor@NI(NI,S,SI), K in (N,S]
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
  conf->hookUp(matchLookupIntoNodeS, 0, makeRes1S, 0);
  



  // Join with best successor table
  ElementSpecRef matchRes1IntoBestSuccS =
    conf->addElement(New refcounted< UniqueLookup >(strbuf("LookupInBestSucc:") << name,
                                                    bestSuccessorTable,
                                                    1, // Match res1.NI
                                                    1 // with bestSuccessor.NI
                                                    ));
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
  conf->hookUp(matchRes1IntoBestSuccS, 0, 
  //              probeS, 0);
  // conf->hookUp(probeS, 0, 
               selectS, 0);
  

  // Project to create lookupResults(R, K, S, SI, E)
  // from <<res1 NI, K, R, E, N><bestSuccessor NI S SI>>
  ElementSpecRef makeLookupResultS =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenLookupResult:").cat(name),
                                                    "\"LookupResult\" pop \
                                                     $0 3 field pop /* output R */ \
                                                     $0 2 field pop /* output K */ \
                                                     $1 unbox drop drop pop pop /* output S SI */ \
                                                     $0 4 field pop /* output E */ \
                                                     $0 5 field pop /* output N */"));
  conf->hookUp(selectS, 0, makeLookupResultS, 0);
  conf->hookUp(makeLookupResultS, 0, pullLookupResultsOut, pullLookupResultsOutPort);
}

/** L2: bestLookupDistance@NI(NI,K,R,E,min<D>) :- lookup@NI(NI,K,R,E),
    node@NI(NI, N), finger@NI(NI,I,B,BI,ET), B in (N,K), D=f_dist(B,K)
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
  conf->hookUp(matchLookupIntoNodeS, 0, makeRes1S, 0);
  

  // Run aggregation over finger table
  ElementSpecRef findMinInFingerS =
    conf->addElement(New refcounted< PelScan >(str("BestLookupDistance:") << name,
                                               fingerTable, 1,
                                               str("$1 /* res1.NI */ \
                                                    $2 /* NI res1.K */ \
                                                    $5 /* NI K res1.N */ \
                                                    1 ->u32 ->id 0 ->u32 ->id distance /* NI K N maxdist */"),
                                               str("3 peek /* NI */ \
                                                    $1 /* finger.NI */ \
                                                    ==s not /* res1.NI != finger.NI */ \
                                                    ifstop /* empty */ \
                                                    $3 /* B */ \
                                                    2 peek /* B N */ \
                                                    4 peek /* B N K */ \
                                                    ()id  /* (B in (N,K)) */ \
                                                    not ifstop /* empty */ \
                                                    $3 3 peek distance dup /* dist(B,K) twice */ \
                                                    2 peek /* newDist newDist oldDist */ \
                                                    <id /* newDist (newDist<oldMin) */ \
                                                    swap /* (old>new?) newDist */ \
                                                    2 peek ifelse /* ((old>new) ? new : old) */ \
                                                    swap /* swap newMin in state where oldMin was */ \
                                                    drop /* only state remains */"),
                                               str("\"BestLookupDistance\" pop $1 pop /* output NI */\
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
  // ElementSpecRef probeS =
  //   conf->addElement(New refcounted< Print >(strbuf("Probe")));
  conf->hookUp(pushRes1S, 0,
               //probeS, 0);
               //conf->hookUp(probeS, 0,
               findMinInFingerS, 0);

  conf->hookUp(findMinInFingerS, 0, pullLookupDistanceOut, pullLookupDistanceOutPort);
}


/** Test lookups. */
void testRuleL2(LoggerI::Level level)
{
  TableRef fingerTable =
    New refcounted< Table >(strbuf("FingerTable"), 100);
  fingerTable->add_multiple_index(1);
  
  IDRef me = ID::mk((uint32_t) 1);

  // Fill up the table with fingers
  for (uint i = 0;
       i < FINGERS;
       i++) {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("Finger"));

    str myAddress = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(myAddress));

    IDRef target = ID::mk((uint32_t) 0X200)->shift(10 * i)->add(me);
    tuple->append(Val_ID::mk(target));

    IDRef best = ID::mk()->add(target)->add(ID::mk((uint32_t) i*10));
    tuple->append(Val_ID::mk(best));
  
    str address = str(strbuf() << FINGERIP << ":" << i);
    tuple->append(Val_Str::mk(address));
    tuple->freeze();
    fingerTable->insert(tuple);
    warn << tuple->toString() << "\n";
  }
  


  TableRef nodeTable =
    New refcounted< Table >(strbuf("NodeTable"), 1);
  nodeTable->add_unique_index(1);
  

  {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("Node"));
    
    str myAddress = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(myAddress));
    tuple->append(Val_ID::mk(me));
    tuple->freeze();
    nodeTable->insert(tuple);
  }
    

  // Create the data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();


  ElementSpecRef funkyS =
    conf->addElement(New refcounted< FunctorSource >(str("Source"),
                                                     &lookupGenerator));
  ElementSpecRef lookupPS =
    conf->addElement(New refcounted< Print >(strbuf("Lookup")));
  ElementSpecRef timedPullPushS =
    conf->addElement(New refcounted< TimedPullPush >("PullPush", 1));
  conf->hookUp(funkyS, 0, lookupPS, 0);
  conf->hookUp(lookupPS, 0, timedPullPushS, 0);

  ElementSpecRef outputPS =
    conf->addElement(New refcounted< Print >(strbuf("Output")));
  ElementSpecRef sinkS =
    conf->addElement(New refcounted< TimedPullSink >("Sink", 0));
  conf->hookUp(outputPS, 0, sinkS, 0);


  ruleL2(str("L2"),
         conf,
         nodeTable,
         fingerTable,
         timedPullPushS, 0,
         outputPS, 0);

  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized Chord Rule L2.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the router
  router->activate();

  // Schedule kill
  //delaycb(10, 0, wrap(&killJoin));

  // Run the router
  amain();
}






/** L3: lookup@BI(BI,K,R,E) :- bestLookupDistance@NI(NI,K,R,E,D),
    finger@NI(NI,I,B,BI), D=f_dist(B,K), B in (N, K)
*/
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
  conf->hookUp(matchBLDIntoNodeS, 0, makeRes1S, 0);
  

  // Join with finger table
  ElementSpecRef matchRes1IntoFingerS =
    conf->addElement(New refcounted< MultLookup >(strbuf("LookupInFinger:") << name,
                                                  fingerTable,
                                                  1, // Match res1.NI
                                                  1 // with finger.NI
                                                  ));
  // Res1 must be pushed to second join
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name,
                                                     0));
  // Link the two joins together
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, matchRes1IntoFingerS, 0);

  // Select out the outcomes that match the select clause
  // (finger.B in (res1.N, res1.K)) and (res1.D == distance(finger.B,
  // res1.K))
  // from <<Res1 NI K R E D N><Finger NI I B BI>>
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("SelectRes1:").cat(name),
                                                    "$1 3 field /* finger.B */\
                                                     $0 6 field /* B res1.N */\
                                                     $0 2 field /* B N res1.K */\
                                                     ()id /* B in (N,K) */\
                                                     $1 3 field /* B in (N,K) finger.B */\
                                                     $0 2 field /* B in (N,K) B res1.K */\
                                                     distance /* B in (N,K) dist(B,K) */\
                                                     $0 5 field /* B in (N,K) dist(B,K) res1.D */\
                                                     ==id /* B in (N,K) (dist==D) */\
                                                     and not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through */"));
  conf->hookUp(matchRes1IntoFingerS, 0, selectS, 0);

  // Project onto lookup(BI,K,R,E)
  // from <<Res1 NI K R E D N><Finger NI I B BI>>
  ElementSpecRef makeLookupS =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenLookup:").cat(name),
                                                    "\"Lookup\" pop \
                                                     $1 4 field pop /* BI */ \
                                                     $0 2 field pop /* K */ \
                                                     $0 3 field pop /* R */ \
                                                     $0 4 field pop /* E */"));
  conf->hookUp(selectS, 0, makeLookupS, 0);
  conf->hookUp(makeLookupS, 0, pullLookupOut, pullLookupOutPort);
}

/** Put lookup rules together.*/
void lookupRules(str name,
                 Router::ConfigurationRef conf,
                 TableRef nodeTable,
                 TableRef fingerTable,
                 TableRef bestSuccessorTable,
                 ElementSpecRef pushLookupIn,
                 int pushLookupInPort,
                 ElementSpecRef pullLookupOut,
                 int pullLookupOutPort)
{
  ElementSpecRef dupS =
    conf->addElement(New refcounted< Duplicate >(strbuf("LookupDup:") << name, 2));
  ElementSpecRef muxS =
    conf->addElement(New refcounted< Mux >(strbuf("LookupMux:").cat(name), 2));
  ElementSpecRef l1l2S =
    conf->addElement(New refcounted< TimedPullPush >("L1-L2", 0));
  conf->hookUp(pushLookupIn, pushLookupInPort,
               dupS, 0);


  ruleL1(strbuf() << name << "::L1",
         conf, nodeTable, bestSuccessorTable, dupS, 0, muxS, 0);
  ruleL2(strbuf() << name << "::L2",
         conf, nodeTable, fingerTable, dupS, 1, l1l2S, 0);
  ruleL3(strbuf() << name << "::L3",
         conf, nodeTable, fingerTable, l1l2S, 0, muxS, 1);
  conf->hookUp(muxS, 0, pullLookupOut, pullLookupOutPort);
}



/** SU1: bestSuccessorID@NI(NI,min<D>) :- node@NI(NI,N),
    successor@NI(NI,S,SI), D=f_dist(S,N)
*/
void ruleSU1(str name,
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
  conf->hookUp(matchBLDIntoNodeS, 0, makeRes1S, 0);
  

  // Join with finger table
  ElementSpecRef matchRes1IntoFingerS =
    conf->addElement(New refcounted< MultLookup >(strbuf("LookupInFinger:") << name,
                                                  fingerTable,
                                                  1, // Match res1.NI
                                                  1 // with finger.NI
                                                  ));
  // Res1 must be pushed to second join
  ElementSpecRef pushRes1S =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushRes1:") << name,
                                                     0));
  // Link the two joins together
  conf->hookUp(makeRes1S, 0, pushRes1S, 0);
  conf->hookUp(pushRes1S, 0, matchRes1IntoFingerS, 0);

  // Select out the outcomes that match the select clause
  // (finger.B in (res1.N, res1.K)) and (res1.D == distance(finger.B,
  // res1.K))
  // from <<Res1 NI K R E D N><Finger NI I B BI>>
  ElementSpecRef selectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("SelectRes1:").cat(name),
                                                    "$1 3 field /* finger.B */\
                                                     $0 6 field /* B res1.N */\
                                                     $0 2 field /* B N res1.K */\
                                                     ()id /* B in (N,K) */\
                                                     $1 3 field /* B in (N,K) finger.B */\
                                                     $0 2 field /* B in (N,K) B res1.K */\
                                                     distance /* B in (N,K) dist(B,K) */\
                                                     $0 5 field /* B in (N,K) dist(B,K) res1.D */\
                                                     ==id /* B in (N,K) (dist==D) */\
                                                     and not ifstop /* select clause */\
                                                     $0 pop $1 pop /* pass through */"));
  conf->hookUp(matchRes1IntoFingerS, 0, selectS, 0);

  // Project onto lookup(BI,K,R,E)
  // from <<Res1 NI K R E D N><Finger NI I B BI>>
  ElementSpecRef makeLookupS =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenLookup:").cat(name),
                                                    "\"Lookup\" pop \
                                                     $1 4 field pop /* BI */ \
                                                     $0 2 field pop /* K */ \
                                                     $0 3 field pop /* R */ \
                                                     $0 4 field pop /* E */"));
  conf->hookUp(selectS, 0, makeLookupS, 0);
  conf->hookUp(makeLookupS, 0, pullLookupOut, pullLookupOutPort);
}



/** Test lookups. */
void testLookups(LoggerI::Level level)
{
  TableRef fingerTable =
    New refcounted< Table >(strbuf("FingerTable"), 100);
  fingerTable->add_multiple_index(1);
  
  IDRef me = ID::mk((uint32_t) 1);

  // Fill up the table with fingers
  for (uint i = 0;
       i < FINGERS;
       i++) {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("Finger"));

    str myAddress = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(myAddress));

    IDRef target = ID::mk((uint32_t) 0X200)->shift(10 * i)->add(me);
    tuple->append(Val_ID::mk(target));

    IDRef best = ID::mk()->add(target)->add(ID::mk((uint32_t) i*10));
    tuple->append(Val_ID::mk(best));
  
    str address = str(strbuf() << FINGERIP << ":" << i);
    tuple->append(Val_Str::mk(address));
    tuple->freeze();
    fingerTable->insert(tuple);
    warn << tuple->toString() << "\n";
  }
  


  TableRef nodeTable =
    New refcounted< Table >(strbuf("NodeTable"), 1);
  nodeTable->add_unique_index(1);
  

  {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("Node"));
    
    str myAddress = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(myAddress));
    tuple->append(Val_ID::mk(me));
    tuple->freeze();
    nodeTable->insert(tuple);
  }
    

  TableRef bestSuccessorTable =
    New refcounted< Table >(strbuf("BestSuccessorTable"), 1);
  bestSuccessorTable->add_unique_index(1);
  

  {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("BestSuccessor"));
    
    str myAddress = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(myAddress));

    IDRef target = ID::mk((uint32_t) 0X200)->add(me);
    IDRef best = ID::mk()->add(target)->add(ID::mk((uint) 10));
    tuple->append(Val_ID::mk(best));
  
    str address = str(strbuf() << FINGERIP << ":" << 0);
    tuple->append(Val_Str::mk(address));
    tuple->freeze();
    bestSuccessorTable->insert(tuple);
  }
    


  // Create the data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();


  ElementSpecRef funkyS =
    conf->addElement(New refcounted< FunctorSource >(str("Source"),
                                                     &lookupGenerator));
  ElementSpecRef lookupPS =
    conf->addElement(New refcounted< Print >(strbuf("Lookup")));
  ElementSpecRef timedPullPushS =
    conf->addElement(New refcounted< TimedPullPush >("PullPush", 1));
  conf->hookUp(funkyS, 0, lookupPS, 0);
  conf->hookUp(lookupPS, 0, timedPullPushS, 0);

  ElementSpecRef outputPS =
    conf->addElement(New refcounted< Print >(strbuf("Output")));
  ElementSpecRef sinkS =
    conf->addElement(New refcounted< TimedPullSink >("Sink", 0));
  conf->hookUp(outputPS, 0, sinkS, 0);


  lookupRules(str("lookup"),
              conf,
              nodeTable,
              fingerTable,
              bestSuccessorTable,
              timedPullPushS, 0,
              outputPS, 0);

  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized network of chord lookup flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the router
  router->activate();

  // Schedule kill
  //delaycb(10, 0, wrap(&killJoin));

  // Run the router
  amain();
}


int main(int argc, char **argv)
{
  std::cout << "\nChord\n";

  LoggerI::Level level = LoggerI::WARN;
  if (argc > 1) {
    str levelName(argv[1]);
    level = LoggerI::levelFromName[levelName];
  }

  int seed = 0;
  if (argc > 2) {
    seed = atoi(argv[2]);
  }
  srand(seed);

  // testBestLookupDistance(level);
  testLookups(level);
  // testRuleL2(level);
  return 0;
}
  

/*
 * End of file 
 */
