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

struct LookupGenerator : public FunctorSource::Generator
{
  virtual ~LookupGenerator() {};
  TupleRef operator()() const {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("Lookup"));
    str address = str(strbuf() << "127.0.0.2:10000");
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

    str req = str(strbuf() << "127.0.0.100:10000");
    tuple->append(Val_Str::mk(req));

    tuple->append(Val_UInt32::mk(rand())); // the event ID

    IDRef me = ID::mk((uint32_t) 1);
    tuple->append(Val_ID::mk(me));

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
  fingers->add_multiple_index(2);
  
  IDRef me = ID::mk((uint32_t) 1);

  // Fill up the table with fingers
  for (uint i = 0;
       i < FINGERS;
       i++) {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("Finger"));

    str myAddress = str(strbuf() << "127.0.0.1:1");
    tuple->append(Val_Str::mk(myAddress));

    IDRef target = ID::mk((uint32_t) 0X200)->shift(10 * i)->add(me);
    tuple->append(Val_ID::mk(target));

    IDRef best = ID::mk()->add(target)->add(ID::mk((uint32_t) i*10));
    tuple->append(Val_ID::mk(best));
  
    str address = str(strbuf() << "127.0.0.1:" << i);
    tuple->append(Val_Str::mk(address));
    tuple->freeze();
    fingers->insert(tuple);
    warn << tuple->toString() << "\n";
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
  ElementSpecRef bestDistanceScannerS =
    conf->addElement(New refcounted< PelScan >(str("BestDistance"), fingers, 2,
                                               str("$2 $5 1 ->u32 ->id 0 ->u32 ->id distance"),
                                               str("$3 /* B */ \
                                                    2 peek /* B N */ \
                                                    4 peek /* B N K */ \
                                                    ()id  /* (B in (N,K)) */ \
                                                    not ifstop /* empty */ \
                                                    $3 3 peek distance dup /* dist(B,N) twice */ \
                                                    2 peek /* newDist newDist oldDist */ \
                                                    <id /* newDist (newDist<oldMin) */ \
                                                    swap /* (old>new?) newDist */ \
                                                    2 peek ifelse /* ((old>new) ? new : old) */ \
                                                    swap /* replace newMin in state where oldMin was */ \
                                                    drop /* only state remains */"),
                                               str("\"BestDistance\" pop swap pop swap pop pop")));

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




int main(int argc, char **argv)
{
  std::cout << "\nChord\n";

  LoggerI::Level level = LoggerI::ALL;
  if (argc > 1) {
    str levelName(argv[1]);
    level = LoggerI::levelFromName[levelName];
  }

  int seed = 0;
  if (argc > 2) {
    seed = atoi(argv[2]);
  }
  srand(seed);

  testBestLookupDistance(level);
  return 0;
}
  

/*
 * End of file 
 */
