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
 * DESCRIPTION: A symmetric reachability data flow
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
#include "val_str.h"

#include "print.h"
#include "discard.h"
#include "pelTransform.h"
#include "duplicate.h"
#include "dupElim.h"
#include "filter.h"
#include "store.h"
#include "timedPullPush.h"
#include "joiner.h"
#include "udp.h"
#include "marshalField.h"
#include "unmarshalField.h"
#include "mux.h"
#include "strToSockaddr.h"
#include "slot.h"
#include "timedPullSink.h"
#include "timestampSource.h"
#include "hexdump.h"

void killJoin()
{
  exit(0);
}

static int LINKS = 4;
static int STARTING_PORT = 10000;
static const int nodes = 100;

/** Periodically go over my existing link entries, turn them into reach
    entries and blast them out */
void makeReachFlow(Router::ConfigurationRef conf,
                   str name,
                   ref< Store > linkStore,
                   ElementSpecRef outgoingInput,
                   int outgoingInputPort)
{
  // Scanner element over link table
  ElementSpecRef scanS =
    conf->addElement(linkStore->mkScan());
  ElementSpecRef scanPrintS =
    conf->addElement(New refcounted< Print >(strbuf("Scan:") << name));
  ElementSpecRef timedPullPushS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushReach:").cat(name),
                                                 1));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:").cat(name)));
  ElementSpecRef transS =
    conf->addElement(New refcounted< PelTransform >(strbuf("MakeReach").cat(name),
                                                    "\"Reach\" pop $1 pop $2 pop"));

  // Connect to outgoing hook
  conf->hookUp(scanS, 0, scanPrintS, 0);
  conf->hookUp(scanPrintS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, outgoingInput, outgoingInputPort);
}


void makeSemiJoin(Router::ConfigurationRef conf,
                  str name,
                  ref< Store > store,
                  ElementSpecRef incomingOutput,
                  int incomingOutputPort,
                  ElementSpecRef outgoingInput,
                  int outgoingInputPort)
{
  ElementSpecRef splitS =
    conf->addElement(New refcounted< Duplicate >(strbuf("Split:").cat(name), 2));
  ElementSpecRef keyMakerS =
    conf->addElement(New refcounted< PelTransform >(strbuf("KeyMaker:").cat(name), "$1 pop"));
  ElementSpecRef lookupPS =
    conf->addElement(New refcounted< Print >(strbuf("Lookup:") << name));
  ElementSpecRef lookupS =
    conf->addElement(store->mkLookup());
  ElementSpecRef foundPS =
    conf->addElement(New refcounted< Print >(strbuf("Found:") << name));
  ElementSpecRef joinerS =
    conf->addElement(New refcounted< Joiner >(strbuf("Joiner:").cat(name)));
  
  conf->hookUp(splitS, 0, keyMakerS, 0);
  conf->hookUp(keyMakerS, 0, lookupPS, 0);
  conf->hookUp(lookupPS, 0, lookupS, 0);
  conf->hookUp(lookupS, 0, foundPS, 0);
  conf->hookUp(foundPS, 0, joinerS, 0);
  conf->hookUp(splitS, 1, joinerS, 1);
                     
  

  // Hook it up
  conf->hookUp(incomingOutput, incomingOutputPort,
               splitS, 0);
  conf->hookUp(joinerS, 0,
               outgoingInput, outgoingInputPort);
}


void makeTransitiveFlow(Router::ConfigurationRef conf,
                        ref< Store > linkStore,
                        ref< Store > reachStore,
                        str name,
                        ref< Udp > udp,
                        ElementSpecRef outgoingInput,
                        int outgoingInputPort)
{
  // My store of reach tuples
  ElementSpecRef udpRxS = conf->addElement(udp->get_rx());
  ElementSpecRef unmarshalS =
    conf->addElement(New refcounted< UnmarshalField >(strbuf("Unmarshal:").cat(name), 1));

  // Drop the source address and decapsulate
  ElementSpecRef unBoxS =
    conf->addElement(New refcounted< PelTransform >(strbuf("UnBox:").cat(name),
                                                    "$1 unbox pop pop pop"));
  ElementSpecRef dupElimS =
    conf->addElement(New refcounted< DupElim >(strbuf("DupElimA") << name));
  ElementSpecRef insertS =
    conf->addElement(reachStore->mkInsert());
  ElementSpecRef recvPS =
    conf->addElement(New refcounted< Print >(strbuf("Received:") << name));

  
  conf->hookUp(udpRxS, 0, unmarshalS, 0);
  conf->hookUp(unmarshalS, 0, unBoxS, 0);
  conf->hookUp(unBoxS, 0, dupElimS, 0);
  conf->hookUp(dupElimS, 0, insertS, 0);
  conf->hookUp(insertS, 0, recvPS, 0);

  // Here's where the join happens

  // Take the joined tuples and produce the resulting path
  ElementSpecRef joinedS =
    conf->addElement(New refcounted< Print >(strbuf("Joined:") << name));
  ElementSpecRef transS =
    conf->addElement(New refcounted< PelTransform >(strbuf("JoinReach:").cat(name),
                                                    "\"Reach\" pop $5 pop $2 pop"));


  // Prepend with true if this is a Reach X, X.
  ElementSpecRef cycleCheckS =
    conf->addElement(New refcounted< PelTransform >(strbuf("CycleCheck:").cat(name), 
                                                    "$1 $2 ==s not pop $0 pop $1 pop $2 pop"));
  // Only pass through those that have different from and to
  ElementSpecRef filterS =
    conf->addElement(New refcounted< Filter >(strbuf("filter:").cat(name), 0));
  // And drop the filter field
  ElementSpecRef filterDropS =
    conf->addElement(New refcounted< PelTransform >(strbuf("filterDrop:").cat(name),
                                                    "$1 pop $2 pop $3 pop"));

  ElementSpecRef generatedS =
    conf->addElement(New refcounted< Print >(strbuf("Generated:") << name));


  conf->hookUp(joinedS, 0, transS, 0);
  conf->hookUp(transS, 0, cycleCheckS, 0);
  conf->hookUp(cycleCheckS, 0, filterS, 0);
  conf->hookUp(filterS, 0, filterDropS, 0);
  conf->hookUp(filterDropS, 0, generatedS, 0);



  // Hookup the joiner
  makeSemiJoin(conf,
               strbuf("Join:").cat(name),
               linkStore,
               recvPS, 0,
               joinedS, 0);


  // And connect to the outoing element
  conf->hookUp(generatedS, 0,
               outgoingInput, outgoingInputPort);
}




/** Create the entire data flow residing at a single node */
void makeForwarder(Router::ConfigurationRef conf,
                   str name,
                   ref< Udp > udp,
                   ref< Store > linkStore,
                   ref< Store > reachStore)
{
  ElementSpecRef muxS =
    conf->addElement(New refcounted< Mux >(strbuf("RR:").cat(name), 2));
  // ElementSpecRef muxPrintS =
  //   conf->addElement(New refcounted< Print >(strbuf("Mux:") << name));

  // Create the encapsulated version of this tuple, holding the
  // destination address and, encapsulated, the payload containing the
  // reach tuple.
  ElementSpecRef encapS =
    conf->addElement(New refcounted< PelTransform >(strbuf("encap:").cat(name),
                                                    "$1 pop /* The From address */\
                                                     $0 ->t $1 append $2 append pop")); // the rest

  // Now marshall the payload (second field)
  ElementSpecRef marshalS =
    conf->addElement(New refcounted< MarshalField >(strbuf("Marshal:").cat(name),
                                                    1));

  // Hexdump the result
  // ElementSpecRef hexS =
  //   conf->addElement(New refcounted< Hexdump >(strbuf("Hex:").cat(name),
  //                                              1));
  // ElementSpecRef postMarshalS =
  //   conf->addElement(New refcounted< Print >("PostMarshal:"));


  // And translate the address into a sockaddr.  This is a hack until
  // we have a consistent view of the UDP interface.
  ElementSpecRef routeS =
    conf->addElement(New refcounted< StrToSockaddr >(strbuf("Router:").cat(name), 0));
  ElementSpecRef udpTxS =
    conf->addElement(udp->get_tx());
  
  
  conf->hookUp(muxS, 0, encapS, 0);
  conf->hookUp(encapS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, routeS, 0);
  conf->hookUp(routeS, 0, udpTxS, 0);


  // Create reach scanning flow
  makeReachFlow(conf, name, linkStore,
                muxS, 0);

  // ... and the link/reach joiner
  makeTransitiveFlow(conf, linkStore, reachStore,
                     name, udp,
                     muxS, 1);
}




void testMakeReach(LoggerI::Level level)
{
  std::cout << "\nCHECK MAKE REACH FLOWCHUNK\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  // Create one data flow per "node"
  const int nodes = 10;
  ptr< Store > stores;
  ValuePtr nodeIds[nodes];
  str names[nodes];
  
  // Create the networking objects
  for (int i = 0;
       i < nodes;
       i++) {
    names[i] = strbuf("Node") << i;
    nodeIds[i] = Val_Str::mk(strbuf("127.0.0.1") << ":" << (STARTING_PORT + i));
  }

  // Create the link tables and links themselves.  Links must be
  // symmetric
  ValueRef LINK = Val_Str::mk("Link");
  stores = New refcounted< Store >(strbuf("Link:") << names[0], 2);
  std::set< int > others;
  others.insert(0);             // me
  for (int j = 0;
       j < LINKS;
       j++) {
    TupleRef t = Tuple::mk();
    t->append(LINK);
    // From me
    t->append((ValueRef) nodeIds[0]);
    // To someone at random, but not me. Replacement is possible but
    // discouraged
    int other = 0;
    while (others.find(other) != others.end()) {
      other = (int) (10.0 * rand() / (RAND_MAX + 1.0));
    }
    // Got one
    others.insert(other);

    t->append((ValueRef) nodeIds[other]);
    t->freeze();
    stores->insert(t);
  }

  // And make the data flow
  ElementSpecRef scanS =
    conf->addElement(stores->mkScan());
  ElementSpecRef scanPrintS =
    conf->addElement(New refcounted< Print >(strbuf("Scan:") << names[0]));
  ElementSpecRef timedPullPushS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushReach:").cat(names[0]),
                                                     1));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:").cat(names[0])));
  ElementSpecRef transS =
    conf->addElement(New refcounted< PelTransform >(strbuf("MakeReach").cat(names[0]),
                                                    "\"Reach\" pop $1 pop $2 pop"));
  ElementSpecRef sinkPrintS =
    conf->addElement(New refcounted< Print >(strbuf("SinkPrint:") << names[0]));
  ElementSpecRef sinkS =
    conf->addElement(New refcounted< TimedPullSink >(strbuf("Sink:") << names[0], 0));

  // Connect to outgoing hook
  conf->hookUp(scanS, 0, scanPrintS, 0);
  conf->hookUp(scanPrintS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);





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


void testTransmit(LoggerI::Level level)
{
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  // Create one data flow per "node"
  const int nodes = 10;
  ptr< Store > stores;
  ValuePtr nodeIds[nodes];
  str names[nodes];
  
  // Create the networking objects
  for (int i = 0;
       i < nodes;
       i++) {
    names[i] = strbuf("Node") << i;
    nodeIds[i] = Val_Str::mk(strbuf("127.0.0.1") << ":" << (STARTING_PORT + i));
  }

  // Create the link tables and links themselves.  Links must be
  // symmetric
  ValueRef LINK = Val_Str::mk("Link");
  stores = New refcounted< Store >(strbuf("Link:") << names[0], 2);
  std::set< int > others;
  others.insert(0);             // me
  for (int j = 0;
       j < LINKS;
       j++) {
    TupleRef t = Tuple::mk();
    t->append(LINK);
    // From me
    t->append((ValueRef) nodeIds[0]);
    // To someone at random, but not me. Replacement is possible but
    // discouraged
    int other = 0;
    while (others.find(other) != others.end()) {
      other = (int) (10.0 * rand() / (RAND_MAX + 1.0));
    }
    // Got one
    others.insert(other);

    t->append((ValueRef) nodeIds[other]);
    t->freeze();
    stores->insert(t);
  }

  // And make the data flow
  ElementSpecRef scanS =
    conf->addElement(stores->mkScan());
  ElementSpecRef scanPrintS =
    conf->addElement(New refcounted< Print >(strbuf("Scan:") << names[0]));
  ElementSpecRef timedPullPushS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushReach:").cat(names[0]),
                                                     1));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:").cat(names[0])));
  ElementSpecRef transS =
    conf->addElement(New refcounted< PelTransform >(strbuf("MakeReach").cat(names[0]),
                                                    "\"Reach\" pop $1 pop $2 pop"));
  ElementSpecRef sinkPrintS =
    conf->addElement(New refcounted< Print >(strbuf("SinkPrint:") << names[0]));

  // Connect to outgoing hook
  conf->hookUp(scanS, 0, scanPrintS, 0);
  conf->hookUp(scanPrintS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, sinkPrintS, 0);





  // Create the encapsulated version of this tuple, holding the
  // destination address and, encapsulated, the payload containing the
  // reach tuple.
  ElementSpecRef encapS =
    conf->addElement(New refcounted< PelTransform >(strbuf("encap:").cat(names[0]),
                                                    "$2 pop /* The To address */\
                                                     $0 ->t $1 append $2 append pop")); // the rest

  ElementSpecRef preMarshalS =
    conf->addElement(New refcounted< Print >("PreMarshal:"));

  // Now marshall the payload (second field)
  ElementSpecRef marshalS =
    conf->addElement(New refcounted< MarshalField >(strbuf("Marshal:").cat(names[0]),
                                                    1));

  ElementSpecRef postMarshalS =
    conf->addElement(New refcounted< Print >("PostMarshal:"));

  // And translate the address into a sockaddr.  This is a hack until
  // we have a consistent view of the UDP interface.
  ElementSpecRef routeS =
    conf->addElement(New refcounted< StrToSockaddr >(strbuf("Router:").cat(names[0]), 0));
  ref< Udp > udp = New refcounted< Udp >(names[0] << ":Udp",
                                         10000);
  ElementSpecRef udpTxS =
    conf->addElement(udp->get_tx());

  ElementSpecRef muxS =
    conf->addElement(New refcounted< Mux >(strbuf("RR:").cat(names[0]), 2));
  ElementSpecRef tsSourceS =
    conf->addElement(New refcounted< TimestampSource >("TSSource"));
  ElementSpecRef fakeSlotS =
    conf->addElement(New refcounted< Slot >(strbuf("FakeSlot:").cat(names[0])));
  ElementSpecRef fakeHLS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("FakeHL:").cat(names[0]),
                                                     2));

  conf->hookUp(tsSourceS, 0, fakeHLS, 0);
  conf->hookUp(fakeHLS, 0, fakeSlotS, 0);
  conf->hookUp(fakeSlotS, 0, muxS, 1);
  conf->hookUp(sinkPrintS, 0, muxS, 0);
  conf->hookUp(muxS, 0, encapS, 0);
  conf->hookUp(encapS, 0, preMarshalS, 0);
  conf->hookUp(preMarshalS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, postMarshalS, 0);
  conf->hookUp(postMarshalS, 0, routeS, 0);
  conf->hookUp(routeS, 0, udpTxS, 0);


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

/** Build a symmetric link transitive closure. */
void testReachability(LoggerI::Level level)
{
  std::cout << "\nCHECK TRANSITIVE REACHABILITY\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();


  // Create one data flow per "node"
  ptr< Udp > udps[nodes];
  ptr< Store > linkStores[nodes];
  ptr< Store > reachStores[nodes];
  ValuePtr nodeIds[nodes];
  str names[nodes];
  
  // Create the networking objects
  for (int i = 0;
       i < nodes;
       i++) {
    names[i] = strbuf("Node") << i;
    nodeIds[i] = Val_Str::mk(strbuf("127.0.0.1") << ":" << (STARTING_PORT + i));

    udps[i] = New refcounted< Udp >(names[i] << ":Udp",
                                    STARTING_PORT + i);
  }

  // Create the link tables and links themselves.  Links must be
  // symmetric
  ValueRef LINK = Val_Str::mk("Link");
  for (int i = 0;
       i < nodes;
       i++) {
    linkStores[i] = 
      New refcounted< Store >(strbuf("Link:") << names[i], 1);
    reachStores[i] = 
      New refcounted< Store >(strbuf("Reach:") << names[i], 2);
  }
  for (int i = 0;
       i < nodes;
       i++) {
    std::set< int > others;
    others.insert(i);           // me

    for (int j = 0;
         j < LINKS;
         j++) {
      TupleRef t = Tuple::mk();
      t->append(LINK);
      // From me
      t->append((ValueRef) nodeIds[i]);
      // To someone at random, but not me. Replacement is discouraged

      int other = i;
      while (others.find(other) != others.end()) {
        other = (int) (1.0 * nodes * rand() / (RAND_MAX + 1.0));
      }
      // Got one
      others.insert(other);

      t->append((ValueRef) nodeIds[other]);
      t->freeze();
      linkStores[i]->insert(t);

      // Make the symmetric one
      TupleRef symmetric = Tuple::mk();
      symmetric->append(LINK);
      symmetric->append((ValueRef) nodeIds[other]);
      symmetric->append((ValueRef) nodeIds[i]);
      symmetric->freeze();
      linkStores[other]->insert(symmetric);
    }
  }

  // And make the data flows
  for (int i = 0;
       i < nodes;
       i++) {
    makeForwarder(conf, names[i], udps[i],
                  linkStores[i], reachStores[i]);
  }

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



/** Test over a ring. */
void testSimpleCycle(LoggerI::Level level)
{
  std::cout << "\nCHECK TRANSITIVE REACHABILITY ON RING\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();


  // Create one data flow per "node"
  ptr< Udp > udps[nodes];
  ptr< Store > linkStores[nodes];
  ptr< Store > reachStores[nodes];
  ValuePtr nodeIds[nodes];
  str names[nodes];
  
  // Create the networking objects
  for (int i = 0;
       i < nodes;
       i++) {
    names[i] = strbuf("Node") << i;
    nodeIds[i] = Val_Str::mk(strbuf("127.0.0.1") << ":" << (STARTING_PORT + i));

    udps[i] = New refcounted< Udp >(names[i] << ":Udp",
                                    STARTING_PORT + i);
  }

  // Create the link tables and links themselves.  Links must be
  // symmetric
  ValueRef LINK = Val_Str::mk("Link");
  for (int i = 0;
       i < nodes;
       i++) {
    linkStores[i] = 
      New refcounted< Store >(strbuf("Link:") << names[i], 1);
    reachStores[i] = 
      New refcounted< Store >(strbuf("Reach:") << names[i], 2);
  }

  for (int i = 0;
       i < nodes;
       i++) {
    // Forward link
    TupleRef forward = Tuple::mk();
    forward->append(LINK);
    forward->append((ValueRef) nodeIds[i]);
    int other = (i + 1) % nodes;
    forward->append((ValueRef) nodeIds[other]);
    forward->freeze();
    linkStores[i]->insert(forward);

    // Backward link
    TupleRef backward = Tuple::mk();
    backward->append(LINK);
    backward->append((ValueRef) nodeIds[i]);
    int backOther = (i + nodes - 1) % nodes;
    backward->append((ValueRef) nodeIds[backOther]);
    backward->freeze();
    linkStores[i]->insert(backward);
  }

  // And make the data flows
  for (int i = 0;
       i < nodes;
       i++) {
    makeForwarder(conf, names[i], udps[i],
                  linkStores[i], reachStores[i]);
  }

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
  std::cout << "\nGRAPH REACHABILITY\n";

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

  // testSimpleCycle(level);
  testReachability(level);
  // testMakeReach(level);
  // testTransmit(level);

  return 0;
}
  

/*
 * End of file 
 */
