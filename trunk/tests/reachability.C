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
#include "timedPullPush.h"
#include "udp.h"
#include "marshalField.h"
#include "unmarshalField.h"
#include "roundRobin.h"
#include "strToSockaddr.h"
#include "slot.h"
#include "timedPullSink.h"
#include "timestampSource.h"
#include "hexdump.h"
#include "table.h"
#include "lookup.h"
#include "insert.h"
#include "scan.h"

void killJoin()
{
  exit(0);
}

static int LINKS = 2; //4;
static int STARTING_PORT = 10000;
static const int nodes = 5;

/** Periodically go over my existing link entries, turn them into reach
    entries and blast them out */
void makeReachFlow(Router::ConfigurationPtr conf,
                   str name,
                   TablePtr linkTable,
                   ElementSpecPtr outgoingInput,
                   int outgoingInputPort)
{
  // Scanner element over link table
  ElementSpecPtr scanS =
    conf->addElement(ElementPtr(new Scan(strbuf("Scaner:") << name, linkTable, 1)));
  ElementSpecPtr scanPrintS =
    conf->addElement(ElementPtr(new Print(strbuf("Scan:") << name)));
  ElementSpecPtr timedPullPushS =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("PushReach:").cat(name),
                                                 1)));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("Slot:").cat(name))));
  ElementSpecPtr transS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("MakeReach").cat(name),
                                                    "\"Reach\" pop $1 pop $2 pop")));

  // Connect to outgoing hook
  conf->hookUp(scanS, 0, scanPrintS, 0);
  conf->hookUp(scanPrintS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, outgoingInput, outgoingInputPort);
}



void makeTransitiveFlow(Router::ConfigurationPtr conf,
                        TablePtr linkTable,
                        TablePtr reachTable,
                        str name,
                        ref< Udp > udp,
                        ElementSpecPtr outgoingInput,
                        int outgoingInputPort)
{
  // My store of reach tuples
  ElementSpecPtr udpRxS = conf->addElement(udp->get_rx());
  ElementSpecPtr unmarshalS =
    conf->addElement(ElementPtr(new UnmarshalField(strbuf("Unmarshal:").cat(name), 1)));

  // Drop the source address and decapsulate
  ElementSpecPtr unBoxS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("UnBox:").cat(name),
                                                    "$1 unbox pop pop pop")));
  ElementSpecPtr dupElimS =
    conf->addElement(ElementPtr(new DupElim(strbuf("DupElimA:") << name)));
  ElementSpecPtr insertS =
    conf->addElement(ElementPtr(new Insert(strbuf("ReachInsert:") << name, reachTable)));
  ElementSpecPtr recvPS =
    conf->addElement(ElementPtr(new Print(strbuf("Received:") << name)));

  
  conf->hookUp(udpRxS, 0, unmarshalS, 0);
  conf->hookUp(unmarshalS, 0, unBoxS, 0);
  conf->hookUp(unBoxS, 0, dupElimS, 0);
  conf->hookUp(dupElimS, 0, insertS, 0);
  conf->hookUp(insertS, 0, recvPS, 0);

  // Here's where the join happens
  ElementSpecPtr lookupS =
    conf->addElement(ElementPtr(new MultLookup(strbuf("Lookup:") << name,
                                                  linkTable,
                                                  1, 1)));
  conf->hookUp(recvPS, 0, lookupS, 0);

 // Take the joined tuples and produce the resulting path
  ElementSpecPtr joinedS =
    conf->addElement(ElementPtr(new Print(strbuf("Joined:") << name)));
  ElementSpecPtr transS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("JoinReach:").cat(name),
                                                    "\"Reach\" pop $1 2 field pop $0 2 field pop")));
  ElementSpecPtr joinedS1 =
    conf->addElement(ElementPtr(new Print(strbuf("JoinedPel:") << name)));

  // Prepend with true if this is a Reach X, X.
  ElementSpecPtr cycleCheckS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("CycleCheck:").cat(name), 
                                                    "$1 $2 ==s not pop $0 pop $1 pop $2 pop")));
  // Only pass through those that have different from and to
  ElementSpecPtr filterS =
    conf->addElement(ElementPtr(new Filter(strbuf("filter:").cat(name), 0)));
  // And drop the filter field
  ElementSpecPtr filterDropS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("filterDrop:").cat(name),
                                                    "$1 pop $2 pop $3 pop")));

  ElementSpecPtr generatedS =
    conf->addElement(ElementPtr(new Print(strbuf("Generated:") << name)));


  conf->hookUp(lookupS, 0, joinedS, 0);
  conf->hookUp(joinedS, 0, transS, 0);
  conf->hookUp(transS, 0, joinedS1, 0);
  conf->hookUp(joinedS1, 0, cycleCheckS, 0);
  conf->hookUp(cycleCheckS, 0, filterS, 0);
  conf->hookUp(filterS, 0, filterDropS, 0);
  conf->hookUp(filterDropS, 0, generatedS, 0);


  // And connect to the outoing element
  conf->hookUp(generatedS, 0,
               outgoingInput, outgoingInputPort);
}




/** Create the entire data flow residing at a single node */
void makeForwarder(Router::ConfigurationPtr conf,
                   str name,
                   ref< Udp > udp,
                   TablePtr linkTable,
                   TablePtr reachTable)
{
  ElementSpecPtr muxS =
    conf->addElement(ElementPtr(new RoundRobin(strbuf("RR:").cat(name), 2)));
  // ElementSpecPtr muxPrintS =
  //   conf->addElement(ElementPtr(new Print(strbuf("Mux:") << name)));

  // Create the encapsulated version of this tuple, holding the
  // destination address and, encapsulated, the payload containing the
  // reach tuple.
  ElementSpecPtr encapS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("encap:").cat(name),
                                                    "$1 pop /* The From address */\
                                                     $0 ->t $1 append $2 append pop"))); // the rest

  // Now marshall the payload (second field)
  ElementSpecPtr marshalS =
    conf->addElement(ElementPtr(new MarshalField(strbuf("Marshal:").cat(name), 1)));

  // Hexdump the result
  // ElementSpecPtr hexS =
  //   conf->addElement(ElementPtr(new Hexdump(strbuf("Hex:").cat(name),
  //                                              1)));
  // ElementSpecPtr postMarshalS =
  //   conf->addElement(ElementPtr(new Print("PostMarshal:")));


  // And translate the address into a sockaddr.  This is a hack until
  // we have a consistent view of the UDP interface.
  ElementSpecPtr routeS =
    conf->addElement(ElementPtr(new StrToSockaddr(strbuf("Router:").cat(name), 0)));
  ElementSpecPtr udpTxS =
    conf->addElement(udp->get_tx());
  
  
  conf->hookUp(muxS, 0, encapS, 0);
  conf->hookUp(encapS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, routeS, 0);
  conf->hookUp(routeS, 0, udpTxS, 0);


  // Create reach scanning flow
  makeReachFlow(conf, name, linkTable,
                muxS, 0);

  // ... and the link/reach joiner
  makeTransitiveFlow(conf, linkTable, reachTable,
                     name, udp,
                     muxS, 1);
}




void testMakeReach(LoggerI::Level level)
{
  std::cout << "\nCHECK MAKE REACH FLOWCHUNK\n";

  Router::ConfigurationPtr conf(new Router::Configuration());

  // Create one data flow per "node"
  const int nodes = 10;
  TablePtr tables;
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
  ValuePtr LINK = Val_Str::mk("Link");
  tables.reset(new Table(strbuf("Link:") << names[0], 10000));
  tables->add_multiple_index(2);
  std::set< int > others;
  others.insert(0);             // me
  for (int j = 0;
       j < LINKS;
       j++) {
    TuplePtr t = Tuple::mk();
    t->append(LINK);
    // From me
    t->append((ValuePtr) nodeIds[0]);
    // To someone at random, but not me. Replacement is possible but
    // discouraged
    int other = 0;
    while (others.find(other) != others.end()) {
      other = (int) (10.0 * rand() / (RAND_MAX + 1.0));
    }
    // Got one
    others.insert(other);

    t->append((ValuePtr) nodeIds[other]);
    t->freeze();
    tables->insert(t);
  }

  // And make the data flow
  ElementSpecPtr scanS =
    conf->addElement(ElementPtr(new Scan(strbuf("Scaner:"), tables, 2)));
  ElementSpecPtr scanPrintS =
    conf->addElement(ElementPtr(new Print(strbuf("Scan:") << names[0])));
  ElementSpecPtr timedPullPushS =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("PushReach:").cat(names[0]),
                                                     1)));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("Slot:").cat(names[0]))));
  ElementSpecPtr transS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("MakeReach").cat(names[0]),
                                                    "\"Reach\" pop $1 pop $2 pop")));
  ElementSpecPtr sinkPrintS =
    conf->addElement(ElementPtr(new Print(strbuf("SinkPrint:") << names[0])));
  ElementSpecPtr sinkS =
    conf->addElement(ElementPtr(new TimedPullSink(strbuf("Sink:") << names[0], 0)));

  // Connect to outgoing hook
  conf->hookUp(scanS, 0, scanPrintS, 0);
  conf->hookUp(scanPrintS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);





  RouterPtr router(new Router(conf, level));
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized network of reachability flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the router
  router->activate();

  // Schedule kill
  //delayCB(10, wrap(&killJoin));

  // Run the router
  amain();



}


void testTransmit(LoggerI::Level level)
{
  Router::ConfigurationPtr conf(new Router::Configuration());

  // Create one data flow per "node"
  const int nodes = 10;
  TablePtr tables;
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
  ValuePtr LINK = Val_Str::mk("Link");
  tables.reset(new Table(strbuf("Link:") << names[0], 10000));
  tables->add_multiple_index(2);
  std::set< int > others;
  others.insert(0);             // me
  for (int j = 0;
       j < LINKS;
       j++) {
    TuplePtr t = Tuple::mk();
    t->append(LINK);
    // From me
    t->append((ValuePtr) nodeIds[0]);
    // To someone at random, but not me. Replacement is possible but
    // discouraged
    int other = 0;
    while (others.find(other) != others.end()) {
      other = (int) (10.0 * rand() / (RAND_MAX + 1.0));
    }
    // Got one
    others.insert(other);

    t->append((ValuePtr) nodeIds[other]);
    t->freeze();
    tables->insert(t);
  }

  // And make the data flow
  ElementSpecPtr scanS =
    conf->addElement(ElementPtr(new Scan(strbuf("Scaner:"), tables, 2)));
  ElementSpecPtr scanPrintS =
    conf->addElement(ElementPtr(new Print(strbuf("Scan:") << names[0])));
  ElementSpecPtr timedPullPushS =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("PushReach:").cat(names[0]), 1)));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(strbuf("Slot:").cat(names[0]))));
  ElementSpecPtr transS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("MakeReach").cat(names[0]),
                                                    "\"Reach\" pop $1 pop $2 pop")));
  ElementSpecPtr sinkPrintS =
    conf->addElement(ElementPtr(new Print(strbuf("SinkPrint:") << names[0])));

  // Connect to outgoing hook
  conf->hookUp(scanS, 0, scanPrintS, 0);
  conf->hookUp(scanPrintS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, sinkPrintS, 0);





  // Create the encapsulated version of this tuple, holding the
  // destination address and, encapsulated, the payload containing the
  // reach tuple.
  ElementSpecPtr encapS =
    conf->addElement(ElementPtr(new PelTransform(strbuf("encap:").cat(names[0]),
                                                    "$2 pop /* The To address */\
                                                     $0 ->t $1 append $2 append pop"))); // the rest

  ElementSpecPtr preMarshalS =
    conf->addElement(ElementPtr(new Print("PreMarshal:")));

  // Now marshall the payload (second field)
  ElementSpecPtr marshalS =
    conf->addElement(ElementPtr(new MarshalField(strbuf("Marshal:").cat(names[0]),
                                                    1)));

  ElementSpecPtr postMarshalS =
    conf->addElement(ElementPtr(new Print("PostMarshal:")));

  // And translate the address into a sockaddr.  This is a hack until
  // we have a consistent view of the UDP interface.
  ElementSpecPtr routeS =
    conf->addElement(ElementPtr(new StrToSockaddr(strbuf("Router:").cat(names[0]), 0)));
  boost::shared_ptr< Udp > udp(new Udp(names[0] << ":Udp", 10000));
  ElementSpecPtr udpTxS =
    conf->addElement(udp->get_tx());

  ElementSpecPtr muxS =
    conf->addElement(ElementPtr(new RoundRobin(strbuf("RR:").cat(names[0]), 2)));
  ElementSpecPtr tsSourceS =
    conf->addElement(ElementPtr(new TimestampSource("TSSource")));
  ElementSpecPtr fakeSlotS =
    conf->addElement(ElementPtr(new Slot(strbuf("FakeSlot:").cat(names[0]))));
  ElementSpecPtr fakeHLS =
    conf->addElement(ElementPtr(new TimedPullPush(strbuf("FakeHL:").cat(names[0]),
                                                     2)));

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


  RouterPtr router(new Router(conf, level));
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

  Router::ConfigurationPtr conf(new Router::Configuration());


  // Create one data flow per "node"
  ptr< Udp > udps[nodes];
  TablePtr linkTables[nodes];
  TablePtr reachTables[nodes];
  ValuePtr nodeIds[nodes];
  str names[nodes];
  
  // Create the networking objects
  for (int i = 0;
       i < nodes;
       i++) {
    names[i] = strbuf("Node") << i;
    nodeIds[i] = Val_Str::mk(strbuf("127.0.0.1") << ":" << (STARTING_PORT + i));

    udps[i].reset(new Udp(names[i] << ":Udp", STARTING_PORT + i));
  }

  // Create the link tables and links themselves.  Links must be
  // symmetric
  ValuePtr LINK = Val_Str::mk("Link");
  for (int i = 0;
       i < nodes;
       i++) {
    linkTables[i].reset(new Table(strbuf("Link:") << names[i], 10000));
    linkTables[i]->add_multiple_index(1);
    reachTables[i].reset(new Table(strbuf("Reach:") << names[i], 10000));
    reachTables[i]->add_multiple_index(2);
  }
  for (int i = 0;
       i < nodes;
       i++) {
    std::set< int > others;
    others.insert(i);           // me

    for (int j = 0;
         j < LINKS;
         j++) {
      TuplePtr t = Tuple::mk();
      t->append(LINK);
      // From me
      t->append((ValuePtr) nodeIds[i]);
      // To someone at random, but not me. Replacement is discouraged

      int other = i;
      while (others.find(other) != others.end()) {
        other = (int) (1.0 * nodes * rand() / (RAND_MAX + 1.0));
      }
      // Got one
      others.insert(other);

      t->append((ValuePtr) nodeIds[other]);
      t->freeze();
      linkTables[i]->insert(t);

      // Make the symmetric one
      TuplePtr symmetric = Tuple::mk();
      symmetric->append(LINK);
      symmetric->append((ValuePtr) nodeIds[other]);
      symmetric->append((ValuePtr) nodeIds[i]);
      symmetric->freeze();
      linkTables[other]->insert(symmetric);
    }
  }

  // And make the data flows
  for (int i = 0;
       i < nodes;
       i++) {
    makeForwarder(conf, names[i], udps[i],
                  linkTables[i], reachTables[i]);
  }

  RouterPtr router(new Router(conf, level));
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

  Router::ConfigurationPtr conf(new Router::Configuration());


  // Create one data flow per "node"
  ptr< Udp > udps[nodes];
  TablePtr linkTables[nodes];
  TablePtr reachTables[nodes];
  ValuePtr nodeIds[nodes];
  str names[nodes];
  
  // Create the networking objects
  for (int i = 0;
       i < nodes;
       i++) {
    names[i] = strbuf("Node") << i;
    nodeIds[i] = Val_Str::mk(strbuf("127.0.0.1") << ":" << (STARTING_PORT + i));

    udps[i].reset(new Udp(names[i] << ":Udp", STARTING_PORT + i));
  }

  // Create the link tables and links themselves.  Links must be
  // symmetric
  ValuePtr LINK = Val_Str::mk("Link");
  for (int i = 0;
       i < nodes;
       i++) {
    linkTables[i].reset(new Table(strbuf("Link:") << names[i], 10000));
    linkTables[i]->add_multiple_index(1);
    reachTables[i].reset(new Table(strbuf("Reach:") << names[i], 10000));
    reachTables[i]->add_multiple_index(2);
  }

  for (int i = 0;
       i < nodes;
       i++) {
    // Forward link
    TuplePtr forward = Tuple::mk();
    forward->append(LINK);
    forward->append((ValuePtr) nodeIds[i]);
    int other = (i + 1) % nodes;
    forward->append((ValuePtr) nodeIds[other]);
    forward->freeze();
    linkTables[i]->insert(forward);

    // Backward link
    TuplePtr backward = Tuple::mk();
    backward->append(LINK);
    backward->append((ValuePtr) nodeIds[i]);
    int backOther = (i + nodes - 1) % nodes;
    backward->append((ValuePtr) nodeIds[backOther]);
    backward->freeze();
    linkTables[i]->insert(backward);
  }

  // And make the data flows
  for (int i = 0;
       i < nodes;
       i++) {
    makeForwarder(conf, names[i], udps[i],
                  linkTables[i], reachTables[i]);
  }

  RouterPtr router(new Router(conf, level));
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

  TablePtr table = new refcounted< Table >("name", 100);

  LoggerI::Level level = LoggerI::ALL;
  if (argc > 1) {
    str levelName(argv[1]);
    level = LoggerI::levelFromName[levelName];
  }

  int seed = 0;
  if (argc > 2) {
    seed = atoi(argv[2]);
  }
  //srand(seed); 
  srand(0);

  //testSimpleCycle(level);
  testReachability(level);
  // testMakeReach(level);
  // testTransmit(level);

  return 0;
}
  

/*
 * End of file 
 */
