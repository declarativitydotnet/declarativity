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
#include "queue.h"







static const int SUCCESSORSIZE = 16;





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
    tuple->append(Val_Str::mk("lookup"));
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
    tuple->append(Val_Str::mk("lookup"));
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


struct SuccessorGenerator : public FunctorSource::Generator
{
  virtual ~SuccessorGenerator() {};
  TupleRef operator()() const {
    
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("successor"));
    
    str myAddress = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(myAddress));
    
    IDRef successor = ID::mk((uint32_t) rand());
    tuple->append(Val_ID::mk(successor));
  
    str succAddress = str(strbuf() << successor->toString() << "IP");
    tuple->append(Val_Str::mk(succAddress));
    tuple->freeze();
    return tuple;
  }
};

struct SuccessorGenerator successorGenerator;


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
    conf->addElement(New refcounted< Print >(strbuf("lookup")));
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
                                               str("\"bestLookupDistance\" pop $1 pop /* output NI */\
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
    conf->addElement(New refcounted< PelScan >(str("bestLookupDistance:") << name,
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
                                               str("\"bestLookupDistance\" pop $1 pop /* output NI */\
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
    conf->addElement(New refcounted< Print >(strbuf("lookup")));
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
                                                    "\"lookup\" pop \
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
    conf->addElement(New refcounted< RoundRobin >(strbuf("LookupRoundRobin:").cat(name), 2));
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



  // Produce intermediate ephemeral result
  // res1(NI, N) from
  // <<Successor NI S SI><node NI N>>
  ElementSpecRef makeRes1S =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenRes1:").cat(name),
                                                    "\"Res1\" pop \
                                                     $0 1 field pop /* output successor.NI */ \
                                                     $1 2 field pop /* output node.N */"));
  conf->hookUp(matchSuccessorIntoNodeS, 0, makeRes1S, 0);
  

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
                                                    distance dup /* dist(N,S) twice */ \
                                                    2 peek /* newDist newDist oldDist */ \
                                                    <id /* newDist (newDist<oldDist) */ \
                                                    swap /* (newDist<oldDist) newDist */ \
                                                    2 peek ifelse /* ((newDist<oldDist) ? newDist : oldDist) */ \
                                                    swap /* swap newMin in state where oldMin was */ \
                                                    drop /* only state remains */"),
                                               str("\"bestSuccessorDist\" pop 1 peek /* output NI */\
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
  conf->hookUp(matchBSDIntoNodeS, 0, makeRes1S, 0);
  

  // Join with successor table
  ElementSpecRef matchRes1IntoSuccessorS =
    conf->addElement(New refcounted< MultLookup >(strbuf("Res1InSuccessor:") << name,
                                                  successorTable,
                                                  1, // Match res1.NI
                                                  1 // with successor.NI
                                                  ));
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
                                                     distance /* dist(N,S) */\
                                                     $0 3 field /* dist(N,S) res1.D */\
                                                     ==id /* (dist==D) */\
                                                     not ifstop /* drop if not equal */\
                                                     $0 pop $1 pop /* pass through */"));
  conf->hookUp(matchRes1IntoSuccessorS, 0, selectS, 0);

  // Turn into bestSuccessor
  // from <<Res1...><Successor...>>
  ElementSpecRef makeBSS =
    conf->addElement(New refcounted< PelTransform >(strbuf("FlattenSuccessor:").cat(name),
                                                    "$1 unbox drop /* replace with BestSuccessor */\
                                                     \"BestSuccessor\" pop /* new literal */\
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
  conf->hookUp(matchEvictSuccessorIntoNodeS, 0, makeRes1S, 0);
  

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
                                                    distance dup /* dist(N,S) twice */ \
                                                    2 peek /* newDist newDist oldDist */ \
                                                    >id /* newDist (newDist>oldDist) */ \
                                                    swap /* (newDist>oldDist) newDist */ \
                                                    2 peek ifelse /* ((newDist>oldDist) ? newDist : oldDist) */ \
                                                    swap /* swap newMax in state where oldMax was */ \
                                                    drop /* only state remains */"),
                                               str("\"maxSuccessorDist\" pop 1 peek /* output NI */\
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
  conf->hookUp(matchMSDIntoNodeS, 0, makeRes1S, 0);
  

  // Join with successor table
  ElementSpecRef matchRes1IntoSuccessorS =
    conf->addElement(New refcounted< MultLookup >(strbuf("Res1InSuccessor:") << name,
                                                  successorTable,
                                                  1, // Match res1.NI
                                                  1 // with successor.NI
                                                  ));
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
                                                     distance /* dist(N,S) */\
                                                     $0 3 field /* dist(N,S) res1.D */\
                                                     ==id /* (dist==D) */\
                                                     not ifstop /* drop if not equal */\
                                                     $0 pop $1 pop /* pass through */"));
  conf->hookUp(matchRes1IntoSuccessorS, 0, selectS, 0);

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
    conf->addElement(New refcounted< Print >(strbuf("lookup")));
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



/** Test SR1. */
void testRuleSR1(LoggerI::Level level)
{
  // successor(currentNodeIP, successorID, successorIP),
  TableRef successorTable =
    New refcounted< Table >(strbuf("SuccessorTable"), 5);
  successorTable->add_multiple_index(1);

  // Create the count aggregate on the unique index
  std::vector< unsigned > groupBy;
  groupBy.push_back(1);
  Table::MultAggregate u =
    successorTable->add_mult_groupBy_agg(1, groupBy, 1, &Table::AGG_COUNT);

  

  // Create the data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();


  ElementSpecRef funkyS =
    conf->addElement(New refcounted< FunctorSource >(str("Source"),
                                                     &successorGenerator));
  ElementSpecRef lookupPS =
    conf->addElement(New refcounted< Print >(strbuf("NewSuccessor")));
  ElementSpecRef timedPullPushS =
    conf->addElement(New refcounted< TimedPullPush >("PullPush", 1));
  ElementSpecRef insertS =
    conf->addElement(New refcounted< Insert >(strbuf("Insert"),
                                              successorTable));
  ElementSpecRef discardS =
    conf->addElement(New refcounted< Discard >("Discard"));
  conf->hookUp(funkyS, 0, lookupPS, 0);
  conf->hookUp(lookupPS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, insertS, 0);
  conf->hookUp(insertS, 0, discardS, 0);

  ElementSpecRef outputPS =
    conf->addElement(New refcounted< Print >(strbuf("Output")));
  ElementSpecRef sinkS =
    conf->addElement(New refcounted< TimedPullSink >("Sink", 0));
  conf->hookUp(outputPS, 0, sinkS, 0);


  ruleSR1(str("SR1"),
          conf,
          u,
          outputPS, 0);

  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized Chord Rule SR1.\n";
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

void
connectRules(str name,
             str localAddress,
             Router::ConfigurationRef conf,
             TableRef bestSuccessorTable,
             TableRef fingerLookupTable,
             TableRef fingerTable,
             TableRef nextFingerFixTable,
             TableRef notifyTable,
             TableRef nodeTable,
             TableRef predecessorTable,
             TableRef stabilizeTable,
             TableRef successorTable,
             Table::MultAggregate successorCountAggregate,
             ElementSpecRef pushTupleIn,
             int pushTupleInPort,
             ElementSpecRef pullTupleOut,
             int pullTupleOutPort)
{
  // My wraparound mux.  On input 0 comes the outside world. On input 1
  // come tuples that have left locally destined for local rules
  ElementSpecRef wrapAroundMux = conf->addElement(New refcounted< Mux >(strbuf("WrapAroundMux:") <<(name), 2));
  conf->hookUp(pushTupleIn, pushTupleInPort, wrapAroundMux, 0);


  // The demux element for tuples
  ref< vec< ValueRef > > demuxKeys = New refcounted< vec< ValueRef > >;
  demuxKeys->push_back(New refcounted< Val_Str >(str("successor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("lookup")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("bestLookupDistance")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("bestSuccessorDistance")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("maxSuccessorDist")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("evictSuccessor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("successorCount")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("node")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("finger")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("predecessor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("bestSuccessor")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("nextFingerFix")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("fingerLookup")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("stabilize")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("notify")));
  ElementSpecRef demuxS = conf->addElement(New refcounted< Demux >("demux", demuxKeys));
  conf->hookUp(wrapAroundMux, 0, demuxS, 0);

  int nextDemuxOutput = 0;
  // Create the duplicator for each tuple name.  Store the tuple first
  // for materialized tuples
  ElementSpecRef dupSuccessor = conf->addElement(New refcounted< Duplicate >(strbuf("successor") << "Dup:" << name, 1));
  ElementSpecRef insertSuccessor = conf->addElement(New refcounted< Insert >(strbuf("successor") << "Insert:" << name, successorTable));
  conf->hookUp(demuxS, nextDemuxOutput++, insertSuccessor, 0);
  conf->hookUp(insertSuccessor, 0, dupSuccessor, 0);

  ElementSpecRef dupLookup = conf->addElement(New refcounted< Duplicate >(strbuf("lookup") << "Dup:" << name, 2));
  conf->hookUp(demuxS, nextDemuxOutput++, dupLookup, 0);

  ElementSpecRef dupBestLookupDistance = conf->addElement(New refcounted< Duplicate >(strbuf("bestLookupDistance") << "Dup:" << name, 1));
  conf->hookUp(demuxS, nextDemuxOutput++, dupBestLookupDistance, 0);

  ElementSpecRef dupBestSuccessorDistance = conf->addElement(New refcounted< Duplicate >(strbuf("bestSuccessorDistance") << "Dup:" << name, 1));
  conf->hookUp(demuxS, nextDemuxOutput++, dupBestSuccessorDistance, 0);

  ElementSpecRef dupMaxSuccessorDist = conf->addElement(New refcounted< Duplicate >(strbuf("maxSuccessorDist") << "Dup:" << name, 1));
  conf->hookUp(demuxS, nextDemuxOutput++, dupMaxSuccessorDist, 0);

  ElementSpecRef dupEvictSuccessor = conf->addElement(New refcounted< Duplicate >(strbuf("evictSuccessor") << "Dup:" << name, 1));
  conf->hookUp(demuxS, nextDemuxOutput++, dupEvictSuccessor, 0);

  ElementSpecRef dupSuccessorCount = conf->addElement(New refcounted< Duplicate >(strbuf("successorCount") << "Dup:" << name, 1));
  conf->hookUp(demuxS, nextDemuxOutput++, dupSuccessorCount, 0);

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
  ElementSpecRef discardBestSuccessor = conf->addElement(New refcounted< Discard >(strbuf("bestSuccessor") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertBestSuccessor, 0);
  conf->hookUp(insertBestSuccessor, 0, discardBestSuccessor, 0);

  ElementSpecRef insertNextFingerFix = conf->addElement(New refcounted< Insert >(strbuf("nextFingerFix") << "Insert:" << name, nextFingerFixTable));
  ElementSpecRef discardNextFingerFix = conf->addElement(New refcounted< Discard >(strbuf("nextFingerFix") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertNextFingerFix, 0);
  conf->hookUp(insertNextFingerFix, 0, discardNextFingerFix, 0);

  ElementSpecRef insertFingerLookup = conf->addElement(New refcounted< Insert >(strbuf("fingerLookup") << "Insert:" << name, fingerLookupTable));
  ElementSpecRef discardFingerLookup = conf->addElement(New refcounted< Discard >(strbuf("fingerLookup") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertFingerLookup, 0);
  conf->hookUp(insertFingerLookup, 0, discardFingerLookup, 0);

  ElementSpecRef insertStabilize = conf->addElement(New refcounted< Insert >(strbuf("stabilize") << "Insert:" << name, stabilizeTable));
  ElementSpecRef discardStabilize = conf->addElement(New refcounted< Discard >(strbuf("stabilize") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertStabilize, 0);
  conf->hookUp(insertStabilize, 0, discardStabilize, 0);

  ElementSpecRef insertNotify = conf->addElement(New refcounted< Insert >(strbuf("notify") << "Insert:" << name, notifyTable));
  ElementSpecRef discardNotify = conf->addElement(New refcounted< Discard >(strbuf("notify") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertNotify, 0);
  conf->hookUp(insertNotify, 0, discardNotify, 0);


  // Tuples that match nothing
  ElementSpecRef discardDefault = conf->addElement(New refcounted< Discard >(strbuf("DEFAULT") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, discardDefault, 0);



  ElementSpecRef roundRobin = conf->addElement(New refcounted< RoundRobin >(strbuf("RoundRobin:") << name, 8));
  ElementSpecRef wrapAroundPush = conf->addElement(New refcounted< TimedPullPush >(strbuf("WrapAroundPush") << name, 0));

  // The wrap around for locally bound tuples
  ref< vec< ValueRef > > wrapAroundDemuxKeys = New refcounted< vec< ValueRef > >;
  wrapAroundDemuxKeys->push_back(New refcounted< Val_Str >(localAddress));
  ElementSpecRef wrapAroundDemux = conf->addElement(New refcounted< Demux >("wrapAround", wrapAroundDemuxKeys, 1));
  ElementSpecRef outgoingQueue = conf->addElement(New refcounted< Queue >("outgoingQueue", 1000));
  ElementSpecRef wrapAroundPrint = conf->addElement(New refcounted< Print >(strbuf("wrapAroundPrint") << name));
  conf->hookUp(roundRobin, 0, wrapAroundPush, 0);
  conf->hookUp(wrapAroundPush, 0, wrapAroundDemux, 0);
  conf->hookUp(wrapAroundDemux, 0, wrapAroundPrint, 0);
  conf->hookUp(wrapAroundPrint, 0, wrapAroundMux, 1);
  conf->hookUp(wrapAroundDemux, 1, outgoingQueue, 0);
  conf->hookUp(outgoingQueue, 0, pullTupleOut, pullTupleOutPort);




  ruleL1(strbuf(name) << ":L1",
         conf,
         nodeTable,
         bestSuccessorTable,
         dupLookup, 0,
         roundRobin, 0);
  ruleL2(strbuf(name) << ":L2",
         conf,
         nodeTable,
         fingerTable,
         dupLookup, 1,
         roundRobin, 1);
  ruleL3(strbuf(name) << ":L3",
         conf,
         nodeTable,
         fingerTable,
         dupBestLookupDistance, 0,
         roundRobin, 2);
  ruleSU1(strbuf(name) << ":SU1",
          conf,
          nodeTable,
          successorTable,
          dupSuccessor, 0,
          roundRobin, 3);
  ruleSU2(strbuf(name) << ":SU2",
          conf,
          nodeTable,
          successorTable,
          bestSuccessorTable,
          dupBestSuccessorDistance, 0,
          roundRobin, 4);
  ruleSR1(strbuf(name) << ":SR1",
          conf,
          successorCountAggregate,
          roundRobin, 5);
  ruleSR2(strbuf(name) << ":SR2",
          conf,
          SUCCESSORSIZE,
          dupSuccessorCount, 0,
          roundRobin, 6);
  ruleSR3(strbuf(name) << ":SR3",
          conf,
          nodeTable,
          successorTable,
          dupEvictSuccessor, 0,
          roundRobin, 7);
  ruleSR4(strbuf(name) << ":SR4",
          conf,
          nodeTable,
          successorTable,
          dupMaxSuccessorDist, 0);
}




/** Test Chord. */
void testChord(LoggerI::Level level)
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
    
  TableRef successorTable =
    New refcounted< Table >(strbuf("SuccessorTable"), 5);
  successorTable->add_multiple_index(1);

  // Create the count aggregate on the unique index
  std::vector< unsigned > groupBy;
  groupBy.push_back(1);
  Table::MultAggregate successorCountAggregate =
    successorTable->add_mult_groupBy_agg(1, groupBy, 1, &Table::AGG_COUNT);

  // The remaining tables
  TableRef predecessorTable = New refcounted< Table >(strbuf("predecessor"), 3);
  TableRef nextFingerFixTable = New refcounted< Table >(strbuf("nextFingerFix"), 2);
  TableRef fingerLookupTable = New refcounted< Table >(strbuf("fingerLookup"), 3);
  TableRef stabilizeTable = New refcounted< Table >(strbuf("stabilize"), 2);
  TableRef notifyTable = New refcounted< Table >(strbuf("notify"), 2);
  
  

  // Create the data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();


  ElementSpecRef funkyS =
    conf->addElement(New refcounted< FunctorSource >(str("Source"),
                                                     &lookupGenerator));
  ElementSpecRef lookupPS =
    conf->addElement(New refcounted< Print >(strbuf("lookup")));
  ElementSpecRef timedPullPushS =
    conf->addElement(New refcounted< TimedPullPush >("PullPush", 1));
  conf->hookUp(funkyS, 0, lookupPS, 0);
  conf->hookUp(lookupPS, 0, timedPullPushS, 0);

  ElementSpecRef outputPS =
    conf->addElement(New refcounted< Print >(strbuf("Output")));
  ElementSpecRef sinkS =
    conf->addElement(New refcounted< TimedPullSink >("Sink", 0));
  conf->hookUp(outputPS, 0, sinkS, 0);


  connectRules(str("Complete"),
               LOCAL,
               conf,
               bestSuccessorTable,
               fingerLookupTable,
               fingerTable,
               nextFingerFixTable,
               notifyTable,
               nodeTable,
               predecessorTable,
               stabilizeTable,
               successorTable,
               successorCountAggregate,
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
  // testLookups(level);
  // testRuleL2(level);
  testChord(level);
  return 0;
}
  

/*
 * End of file 
 */
