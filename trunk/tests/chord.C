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
#include "tupleSource.h"
#include "queue.h"
#include "noNull.h"
#include "noNullField.h"






static const int SUCCESSORSIZE = 16;
static const double FINGERTTL = 4;
static const int FINGERSIZE = 12;





void killJoin()
{
  exit(0);
}


static const uint FINGERS = 16;
str LOCAL("Local.com");
str REMOTE("Remote.com");
str FINGERIP("Finger.com");

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
                                                    $3 4 peek distance dup /* dist(B,K) twice */ \
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
  // res1.K)) and (res1.NI == finger.NI)
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
                                                    distance /* B in (N,K) dist(B,K) */\
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
                                                     distance /* dist(N,S) */\
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
                                                     distance /* dist(N,S) */\
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
                                                     rand pop /* output random */ \
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
                                                     1 ->u32 ->id /* N (1 as ID)*/\
                                                     $0 3 field /* N 1 fingerLookup.I */\
                                                     <<id /* N 2^(I) */\
                                                     +id pop /* output (N+2^I) */\
                                                     $0 1 field pop /* output fixFinger.NI again */ \
                                                     $0 2 field pop /* output fingerLookup.E */"));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, makeRes1S, 0);
  

  conf->hookUp(makeRes1S, 0, pullLookupOut, pullLookupOutPort);
}



/** rule F5 finger@NI(NI, K, B, BI) :- fingerLookup@NI(NI, E, I),
    lookupResults@NI(NI, K, B, BI, E).
*/
void ruleF5(str name,
            Router::ConfigurationRef conf,
            TableRef fingerLookupTable,
            ElementSpecRef pushLookupResultsIn,
            int pushLookupResultsInPort,
            ElementSpecRef pullFingerOut,
            int pullFingerOutPort)
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
                                                     ==i not /* lR.E!=fL.E? */\
                                                     ifstop /* empty */\
                                                     $0 pop $1 pop /* pass through */\
                                                     "));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, selectS, 0);



  // Produce finger
  // finger(NI, I, B, BI) from
  // <<lookupResults NI K B BI E><fingerLookup NI E I>>
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    "\"finger\" pop \
                                                     $0 1 field pop /* out NI */\
                                                     $1 3 field pop /* out I */\
                                                     $0 3 field pop /* out B */\
                                                     $0 4 field pop /* out BI */\
                                                     "));
  conf->hookUp(selectS, 0, projectS, 0);

  conf->hookUp(projectS, 0, pullFingerOut, pullFingerOutPort);
}


/** rule F6 nextFingerFix@NI(NI, I) :- fingerLookup@NI(NI, E, I1),
    lookupResults@NI(NI, K, B, BI, E), I = I1 + 1 mod finger.SIZE.
*/
void ruleF6(str name,
            Router::ConfigurationRef conf,
            TableRef fingerLookupTable,
            int fingerSize,
            ElementSpecRef pushLookupResultsIn,
            int pushLookupResultsInPort,
            ElementSpecRef pullNextFingerFixOut,
            int pullNextFingerFixOutPort)
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
                                                     ==i not /* lR.E!=fL.E? */\
                                                     ifstop /* empty */\
                                                     $0 pop $1 pop /* pass through */\
                                                     "));
  conf->hookUp(join1S, 0, noNullS, 0);
  conf->hookUp(noNullS, 0, selectS, 0);

  // Produce nextFingerFix
  // nextFingerFix(NI, I) from
  // <<lookupResults NI K B BI E><fingerLookup NI E I>>
  ElementSpecRef projectS =
    conf->addElement(New refcounted< PelTransform >(strbuf("project:").cat(name),
                                                    strbuf("\"nextFingerFix\" pop \
                                                     $0 1 field pop /* out lR.NI */\
                                                     $1 3 field /* fL.I */\
                                                     1 +i /* (I+1) */")
                                                    << fingerSize
                                                    << " % /* ((I+1) mod finger.Size)*/\
                                                     pop /* out newI */\
                                                     "));
  conf->hookUp(selectS, 0, projectS, 0);
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
                                                     rand pop /* out random */\
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
                                                     0, // run immediately
                                                     1 // run once
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
    landmarkNode@NI(NI,LI).
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
  conf->hookUp(noNull2S, 0, project2S, 0);


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
    conf->addElement(New refcounted< UniqueLookup >(strbuf("lookupResultsIntoJoinRecord:") << name,
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
                                                     ==i not /* lR.E!=fL.E? */\
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
  predecessorTuple->append(Val_Str::mk("")); // this is "null"
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
  demuxKeys->push_back(New refcounted< Val_Str >(str("fixFinger")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("lookupResults")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("join")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("joinEvent")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("joinRecord")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("landmarkNode")));
  demuxKeys->push_back(New refcounted< Val_Str >(str("startJoin")));
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
  ElementSpecRef dupFingerLookup = conf->addElement(New refcounted< Duplicate >(strbuf("fingerLookup") << "Dup:" << name, 1));
  conf->hookUp(demuxS, nextDemuxOutput++, insertFingerLookup, 0);
  conf->hookUp(insertFingerLookup, 0, dupFingerLookup, 0);

  ElementSpecRef insertStabilize = conf->addElement(New refcounted< Insert >(strbuf("stabilize") << "Insert:" << name, stabilizeTable));
  ElementSpecRef discardStabilize = conf->addElement(New refcounted< Discard >(strbuf("stabilize") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertStabilize, 0);
  conf->hookUp(insertStabilize, 0, discardStabilize, 0);

  ElementSpecRef insertNotify = conf->addElement(New refcounted< Insert >(strbuf("notify") << "Insert:" << name, notifyTable));
  ElementSpecRef discardNotify = conf->addElement(New refcounted< Discard >(strbuf("notify") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertNotify, 0);
  conf->hookUp(insertNotify, 0, discardNotify, 0);

  ElementSpecRef dupFixFinger = conf->addElement(New refcounted< Duplicate >(strbuf("fixFinger") << "Dup:" << name, 1));
  conf->hookUp(demuxS, nextDemuxOutput++, dupFixFinger, 0);

  ElementSpecRef dupLookupResults = conf->addElement(New refcounted< Duplicate >(strbuf("lookupResults") << "Dup:" << name, 3));
  conf->hookUp(demuxS, nextDemuxOutput++, dupLookupResults, 0);

  ElementSpecRef dupJoin = conf->addElement(New refcounted< Duplicate >(strbuf("join") << "Dup:" << name, 2));
  conf->hookUp(demuxS, nextDemuxOutput++, dupJoin, 0);

  ElementSpecRef dupJoinEvent = conf->addElement(New refcounted< Duplicate >(strbuf("joinEvent") << "Dup:" << name, 1));
  conf->hookUp(demuxS, nextDemuxOutput++, dupJoinEvent, 0);

  ElementSpecRef insertJoinRecord = conf->addElement(New refcounted< Insert >(strbuf("joinRecord") << "Insert:" << name, joinRecordTable));
  ElementSpecRef discardJoinRecord = conf->addElement(New refcounted< Discard >(strbuf("joinRecord") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertJoinRecord, 0);
  conf->hookUp(insertJoinRecord, 0, discardJoinRecord, 0);

  ElementSpecRef insertLandmarkNode = conf->addElement(New refcounted< Insert >(strbuf("landmarkNode") << "Insert:" << name, landmarkNodeTable));
  ElementSpecRef discardLandmarkNode = conf->addElement(New refcounted< Discard >(strbuf("landmarkNode") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, insertLandmarkNode, 0);
  conf->hookUp(insertLandmarkNode, 0, discardLandmarkNode, 0);

  ElementSpecRef dupStartJoin = conf->addElement(New refcounted< Duplicate >(strbuf("startJoin") << "Dup:" << name, 1));
  conf->hookUp(demuxS, nextDemuxOutput++, dupStartJoin, 0);








  // Tuples that match nothing
  ElementSpecRef discardDefault = conf->addElement(New refcounted< Discard >(strbuf("DEFAULT") << "Discard:" << name));
  conf->hookUp(demuxS, nextDemuxOutput++, discardDefault, 0);



  ElementSpecRef roundRobin = conf->addElement(New refcounted< RoundRobin >(strbuf("RoundRobin:") << name, 21));
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
  ruleF1(strbuf(name) << ":F1",
         conf,
         localAddress,
         FINGERTTL,
         roundRobin, 8);
  ruleF2(strbuf(name) << ":F2",
         conf,
         localAddress,
         roundRobin, 9);
  ruleF3(strbuf(name) << ":F3",
         conf,
         nextFingerFixTable,
         dupFixFinger, 0,
         roundRobin, 10);
  ruleF4(strbuf(name) << ":F4",
         conf,
         nodeTable,
         dupFingerLookup, 0,
         roundRobin, 11);
  ruleF5(strbuf(name) << ":F5",
         conf,
         fingerLookupTable,
         dupLookupResults, 0,
         roundRobin, 12);
  ruleF6(strbuf(name) << ":F6",
         conf,
         fingerLookupTable,
         FINGERSIZE,
         dupLookupResults, 1,
         roundRobin, 13);
  ruleJ1(strbuf(name) << ":J1",
         conf,
         dupJoinEvent, 0,
         roundRobin, 14);
  ruleJ1a(strbuf(name) << ":J1a",
          conf,
          localAddress,
          roundRobin, 15);
  ruleJ2(strbuf(name) << ":J2",
         conf,
         dupJoin, 0,
         roundRobin, 16);
  ruleJ3(strbuf(name) << ":J3",
         conf,
         landmarkNodeTable,
         nodeTable,
         dupJoin, 1,
         roundRobin, 17);
  ruleJ4(strbuf(name) << ":J4",
         conf,
         dupStartJoin, 0,
         roundRobin, 18);
  ruleJ5(strbuf(name) << ":J5",
         conf,
         joinRecordTable,
         dupLookupResults, 2,
         roundRobin, 19);
  ruleJ6(strbuf(name) << ":J6",
         conf,
         localAddress,
         roundRobin, 20);
}




/** Test Chord. */
void testChord(LoggerI::Level level)
{
  TableRef fingerTable =
    New refcounted< Table >(strbuf("FingerTable"), 100);
  fingerTable->add_unique_index(2);
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

    // finger index
    tuple->append(Val_Int32::mk(i));

    // The ID of the node currently pointed at by this finger
    IDRef best = ID::mk((uint32_t) 0X200)->shift(10 * i)->add(me)->add(ID::mk((uint32_t) i*10));
    tuple->append(Val_ID::mk(best));

    // The address of the node currently pointed at by this finger
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
  TableRef fingerLookupTable = New refcounted< Table >(strbuf("fingerLookup"), 3);
  fingerLookupTable->add_unique_index(2);
  fingerLookupTable->add_multiple_index(1);


  // The remaining tables
  TableRef predecessorTable = New refcounted< Table >(strbuf("predecessor"), 3);
  TableRef stabilizeTable = New refcounted< Table >(strbuf("stabilize"), 2);
  TableRef notifyTable = New refcounted< Table >(strbuf("notify"), 2);

  // The joinRecord table. Singleton
  TableRef joinRecordTable = New refcounted< Table >(strbuf("joinRecord"), 2);
  joinRecordTable->add_unique_index(1);
  
  // The landmarkNode table. Singleton
  TableRef landmarkNodeTable = New refcounted< Table >(strbuf("landmarkNode"), 2);
  landmarkNodeTable->add_unique_index(1);
  
  

  // Create the data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();


  ElementSpecRef funkyS =
    conf->addElement(New refcounted< FunctorSource >(str("Source"),
                                                     &lookupGenerator));
  ElementSpecRef lookupPS =
    conf->addElement(New refcounted< Print >(strbuf("lookup")));
  ElementSpecRef timedPullPushS =
    conf->addElement(New refcounted< TimedPullPush >("PullPush", 1, 1));
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
               joinRecordTable,
               landmarkNodeTable,
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



/** Created a networked chord flow. If alone, I'm my own successor.  If
    with landmark, I start with a join. */
void testNetworked(LoggerI::Level level,
                   str dottedAddress,
                   int port,
                   bool alone,
                   str landmarkAddress)
{
  str myAddress = strbuf(dottedAddress) << ":" << port;

  TableRef fingerTable =
    New refcounted< Table >(strbuf("FingerTable"), 100);
  fingerTable->add_unique_index(2);
  fingerTable->add_multiple_index(1);
  
  uint32_t random[ID::WORDS];
  for (uint32_t i = 0;
       i < ID::WORDS;
       i++) {
    random[i] = rand();
  }
  IDRef myKey = ID::mk(random);

  TableRef nodeTable =
    New refcounted< Table >(strbuf("NodeTable"), 1);
  nodeTable->add_unique_index(1);
  

  {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("Node"));
    
    tuple->append(Val_Str::mk(myAddress));
    tuple->append(Val_ID::mk(myKey));
    tuple->freeze();
    nodeTable->insert(tuple);
  }
    

  TableRef bestSuccessorTable =
    New refcounted< Table >(strbuf("BestSuccessorTable"), 1);
  bestSuccessorTable->add_unique_index(1);
  
  TableRef successorTable =
    New refcounted< Table >(strbuf("SuccessorTable"), 5);
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
  TableRef fingerLookupTable = New refcounted< Table >(strbuf("fingerLookup"), 3);
  fingerLookupTable->add_unique_index(2);
  fingerLookupTable->add_multiple_index(1);

  // The predecessor table, indexed by its first field
  TableRef predecessorTable = New refcounted< Table >(strbuf("predecessor"), 3);
  predecessorTable->add_unique_index(1);
  

  // The joinRecord table. Singleton
  TableRef joinRecordTable = New refcounted< Table >(strbuf("joinRecord"), 2);
  joinRecordTable->add_unique_index(1);
  
  // The landmarkNode table. Singleton
  TableRef landmarkNodeTable = New refcounted< Table >(strbuf("landmarkNode"), 2);
  landmarkNodeTable->add_unique_index(1);
  
  // The remaining tables
  TableRef stabilizeTable = New refcounted< Table >(strbuf("stabilize"), 2);
  TableRef notifyTable = New refcounted< Table >(strbuf("notify"), 2);


  

  

  // Create the data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();


  Udp udp(strbuf(myAddress) << ":Udp", port);


  ////////////////////////////////////////////////////////
  // The front end, UDP to rules engine
  ElementSpecRef udpRxS = conf->addElement(udp.get_rx());
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
  ElementSpecRef udpTxS = conf->addElement(udp.get_tx());
  
  
  conf->hookUp(encapS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, routeS, 0);
  conf->hookUp(routeS, 0, udpTxS, 0);



  // Setup alone vs. joining
  if (alone) {
    // I'm my own successor
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("successor"));
    
    tuple->append(Val_Str::mk(myAddress));
    
    tuple->append(Val_ID::mk(myKey));
    tuple->append(Val_Str::mk(myAddress)); // again
    tuple->freeze();


    successorTable->insert(tuple);

  } else {
    // I have a landmark node to join to

    TupleRef landmark = Tuple::mk();
    landmark->append(Val_Str::mk("landmarkNode"));
    landmark->append(Val_Str::mk(myAddress));
    landmark->append(Val_Str::mk(landmarkAddress));
    landmark->freeze();
    landmarkNodeTable->insert(landmark);
  }
  
  connectRules(str("Complete"),
               myAddress,
               conf,
               bestSuccessorTable,
               fingerLookupTable,
               fingerTable,
               joinRecordTable,
               landmarkNodeTable,
               nextFingerFixTable,
               notifyTable,
               nodeTable,
               predecessorTable,
               stabilizeTable,
               successorTable,
               successorCountAggregate,
               unBoxS, 0,
               encapS, 0);         // not alone



  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized network of chord lookup flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the router
  router->activate();

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

  testChord(level);
  return 0;
}
  

/*
 * End of file 
 */
