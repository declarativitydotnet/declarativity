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
 * DESCRIPTION: A symmetric reachability data flow
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <stdlib.h>

#include "tuple.h"
#include "plumber.h"
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
void makeReachFlow(Plumber::ConfigurationPtr conf,
                   string name,
                   TablePtr linkTable,
                   ElementSpecPtr outgoingInput,
                   int outgoingInputPort)
{
  // Scanner element over link table
  ElementSpecPtr scanS =
    conf->addElement(ElementPtr(new Scan("Scaner:"+ name, linkTable, 1)));
  ElementSpecPtr scanPrintS =
    conf->addElement(ElementPtr(new Print(string("Scan:") + name)));
  ElementSpecPtr timedPullPushS =
    conf->addElement(ElementPtr(new TimedPullPush(string("PushReach:").append(name),
                                                 1)));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("Slot:").append(name))));
  ElementSpecPtr transS =
    conf->addElement(ElementPtr(new PelTransform(string("MakeReach").append(name),
                                                    "\"Reach\" pop $1 pop $2 pop")));

  // Connect to outgoing hook
  conf->hookUp(scanS, 0, scanPrintS, 0);
  conf->hookUp(scanPrintS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, outgoingInput, outgoingInputPort);
}



void makeTransitiveFlow(Plumber::ConfigurationPtr conf,
                        TablePtr linkTable,
                        TablePtr reachTable,
                        string name,
                        ref< Udp > udp,
                        ElementSpecPtr outgoingInput,
                        int outgoingInputPort)
{
  // My store of reach tuples
  ElementSpecPtr udpRxS = conf->addElement(udp->get_rx());
  ElementSpecPtr unmarshalS =
    conf->addElement(ElementPtr(new UnmarshalField(string("Unmarshal:").append(name), 1)));

  // Drop the source address and decapsulate
  ElementSpecPtr unBoxS =
    conf->addElement(ElementPtr(new PelTransform(string("UnBox:").append(name),
                                                    "$1 unbox pop pop pop")));
  ElementSpecPtr dupElimS =
    conf->addElement(ElementPtr(new DupElim(string("DupElimA:") + name)));
  ElementSpecPtr insertS =
    conf->addElement(ElementPtr(new Insert(string("ReachInsert:") + name, reachTable)));
  ElementSpecPtr recvPS =
    conf->addElement(ElementPtr(new Print(string("Received:") + name)));

  
  conf->hookUp(udpRxS, 0, unmarshalS, 0);
  conf->hookUp(unmarshalS, 0, unBoxS, 0);
  conf->hookUp(unBoxS, 0, dupElimS, 0);
  conf->hookUp(dupElimS, 0, insertS, 0);
  conf->hookUp(insertS, 0, recvPS, 0);

  // Here's where the join happens
  ElementSpecPtr lookupS =
    conf->addElement(ElementPtr(new MultLookup(string("Lookup:") + name,
                                                  linkTable,
                                                  1, 1)));
  conf->hookUp(recvPS, 0, lookupS, 0);

 // Take the joined tuples and produce the resulting path
  ElementSpecPtr joinedS =
    conf->addElement(ElementPtr(new Print(string("Joined:") + name)));
  ElementSpecPtr transS =
    conf->addElement(ElementPtr(new PelTransform(string("JoinReach:").append(name),
                                                    "\"Reach\" pop $1 2 field pop $0 2 field pop")));
  ElementSpecPtr joinedS1 =
    conf->addElement(ElementPtr(new Print(string("JoinedPel:") + name)));

  // Prepend with true if this is a Reach X, X.
  ElementSpecPtr cycleCheckS =
    conf->addElement(ElementPtr(new PelTransform(string("CycleCheck:").append(name), 
                                                    "$1 $2 ==s not pop $0 pop $1 pop $2 pop")));
  // Only pass through those that have different from and to
  ElementSpecPtr filterS =
    conf->addElement(ElementPtr(new Filter(string("filter:").append(name), 0)));
  // And drop the filter field
  ElementSpecPtr filterDropS =
    conf->addElement(ElementPtr(new PelTransform(string("filterDrop:").append(name),
                                                    "$1 pop $2 pop $3 pop")));

  ElementSpecPtr generatedS =
    conf->addElement(ElementPtr(new Print(string("Generated:") + name)));


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
void makeForwarder(Plumber::ConfigurationPtr conf,
                   string name,
                   ref< Udp > udp,
                   TablePtr linkTable,
                   TablePtr reachTable)
{
  ElementSpecPtr muxS =
    conf->addElement(ElementPtr(new RoundRobin(string("RR:").append(name), 2)));
  // ElementSpecPtr muxPrintS =
  //   conf->addElement(ElementPtr(new Print(string("Mux:") + name)));

  // Create the encapsulated version of this tuple, holding the
  // destination address and, encapsulated, the payload containing the
  // reach tuple.
  ElementSpecPtr encapS =
    conf->addElement(ElementPtr(new PelTransform(string("encap:").append(name),
                                                    "$1 pop /* The From address */\
                                                     $0 ->t $1 append $2 append pop"))); // the rest

  // Now marshall the payload (second field)
  ElementSpecPtr marshalS =
    conf->addElement(ElementPtr(new MarshalField(string("Marshal:").append(name), 1)));

  // Hexdump the result
  // ElementSpecPtr hexS =
  //   conf->addElement(ElementPtr(new Hexdump(string("Hex:").append(name),
  //                                              1)));
  // ElementSpecPtr postMarshalS =
  //   conf->addElement(ElementPtr(new Print("PostMarshal:")));


  // And translate the address into a sockaddr.  This is a hack until
  // we have a consistent view of the UDP interface.
  ElementSpecPtr routeS =
    conf->addElement(ElementPtr(new StrToSockaddr(string("Router:").append(name), 0)));
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

  Plumber::ConfigurationPtr conf(new Plumber::Configuration());

  // Create one data flow per "node"
  const int nodes = 10;
  TablePtr tables;
  ValuePtr nodeIds[nodes];
  string names[nodes];
  
  // Create the networking objects
  for (int i = 0;
       i < nodes;
       i++) {
    ostringstream o1;
    ostringstream o2;
    o1 << string("Node") << i;
    names[i] = o1.str();
    o2 << string("127.0.0.1") << ":" << (STARTING_PORT + i);
    nodeIds[i] = Val_Str::mk(o2.str());
  }

  // Create the link tables and links themselves.  Links must be
  // symmetric
  ValuePtr LINK = Val_Str::mk("Link");
  tables.reset(new Table(string("Link:") + names[0], 10000));
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
    conf->addElement(ElementPtr(new Scan(string("Scaner:"), tables, 2)));
  ElementSpecPtr scanPrintS =
    conf->addElement(ElementPtr(new Print(string("Scan:") + names[0])));
  ElementSpecPtr timedPullPushS =
    conf->addElement(ElementPtr(new TimedPullPush(string("PushReach:").append(names[0]),
                                                     1)));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("Slot:").append(names[0]))));
  ElementSpecPtr transS =
    conf->addElement(ElementPtr(new PelTransform(string("MakeReach").append(names[0]),
                                                    "\"Reach\" pop $1 pop $2 pop")));
  ElementSpecPtr sinkPrintS =
    conf->addElement(ElementPtr(new Print(string("SinkPrint:") + names[0])));
  ElementSpecPtr sinkS =
    conf->addElement(ElementPtr(new TimedPullSink(string("Sink:") + names[0], 0)));

  // Connect to outgoing hook
  conf->hookUp(scanS, 0, scanPrintS, 0);
  conf->hookUp(scanPrintS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);





  PlumberPtr plumber(new Plumber(conf, level));
  if (plumber->initialize(plumber) == 0) {
    std::cout << "Correctly initialized network of reachability flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the plumber
  plumber->activate();

  // Schedule kill
  //delayCB(10, wrap(&killJoin));

  // Run the plumber
  eventLoop();



}


void testTransmit(LoggerI::Level level)
{
  eventLoopInitialize();
  Plumber::ConfigurationPtr conf(new Plumber::Configuration());

  // Create one data flow per "node"
  const int nodes = 10;
  TablePtr tables;
  ValuePtr nodeIds[nodes];
  string names[nodes];
  
  // Create the networking objects
  for (int i = 0;
       i < nodes;
       i++) {
    ostringstream o1;
    ostringstream o2;

    o1 << string("Node") << i;
    o2 << string("127.0.0.1") << ":" << (STARTING_PORT + i);
    names[i] = o1.str()
    nodeIds[i] = Val_Str::mk(o2.str());
  }

  // Create the link tables and links themselves.  Links must be
  // symmetric
  ValuePtr LINK = Val_Str::mk("Link");
  tables.reset(new Table(string("Link:") + names[0], 10000));
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
    conf->addElement(ElementPtr(new Scan(string("Scaner:"), tables, 2)));
  ElementSpecPtr scanPrintS =
    conf->addElement(ElementPtr(new Print(string("Scan:") + names[0])));
  ElementSpecPtr timedPullPushS =
    conf->addElement(ElementPtr(new TimedPullPush(string("PushReach:").append(names[0]), 1)));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("Slot:").append(names[0]))));
  ElementSpecPtr transS =
    conf->addElement(ElementPtr(new PelTransform(string("MakeReach").append(names[0]),
                                                    "\"Reach\" pop $1 pop $2 pop")));
  ElementSpecPtr sinkPrintS =
    conf->addElement(ElementPtr(new Print(string("SinkPrint:") + names[0])));

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
    conf->addElement(ElementPtr(new PelTransform(string("encap:").append(names[0]),
                                                    "$2 pop /* The To address */\
                                                     $0 ->t $1 append $2 append pop"))); // the rest

  ElementSpecPtr preMarshalS =
    conf->addElement(ElementPtr(new Print("PreMarshal:")));

  // Now marshall the payload (second field)
  ElementSpecPtr marshalS =
    conf->addElement(ElementPtr(new MarshalField(string("Marshal:").append(names[0]),
                                                    1)));

  ElementSpecPtr postMarshalS =
    conf->addElement(ElementPtr(new Print("PostMarshal:")));

  // And translate the address into a sockaddr.  This is a hack until
  // we have a consistent view of the UDP interface.
  ElementSpecPtr routeS =
    conf->addElement(ElementPtr(new StrToSockaddr(string("Router:").append(names[0]), 0)));
  boost::shared_ptr< Udp > udp(new Udp(names[0] << ":Udp", 10000));
  ElementSpecPtr udpTxS =
    conf->addElement(udp->get_tx());

  ElementSpecPtr muxS =
    conf->addElement(ElementPtr(new RoundRobin(string("RR:").append(names[0]), 2)));
  ElementSpecPtr tsSourceS =
    conf->addElement(ElementPtr(new TimestampSource("TSSource")));
  ElementSpecPtr fakeSlotS =
    conf->addElement(ElementPtr(new Slot(string("FakeSlot:").append(names[0]))));
  ElementSpecPtr fakeHLS =
    conf->addElement(ElementPtr(new TimedPullPush(string("FakeHL:").append(names[0]),
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


  PlumberPtr plumber(new Plumber(conf, level));
  if (plumber->initialize(plumber) == 0) {
    std::cout << "Correctly initialized network of reachability flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the plumber
  plumber->activate();

  // Schedule kill
  //delaycb(10, 0, wrap(&killJoin));

  // Run the plumber
  eventLoop();
}

/** Build a symmetric link transitive closure. */
void testReachability(LoggerI::Level level)
{
  std::cout << "\nCHECK TRANSITIVE REACHABILITY\n";

  Plumber::ConfigurationPtr conf(new Plumber::Configuration());


  // Create one data flow per "node"
  ptr< Udp > udps[nodes];
  TablePtr linkTables[nodes];
  TablePtr reachTables[nodes];
  ValuePtr nodeIds[nodes];
  string names[nodes];
  
  // Create the networking objects
  for (int i = 0;
       i < nodes;
       i++) {
    ostringstream o1;
    ostringstream o2;
    o1 << string("Node") << i;
    o2 << "127.0.0.1" << ":" << (STARTING_PORT + i);
    names[i] = o1.str();
    nodeIds[i] = Val_Str::mk(o2.str());

    udps[i].reset(new Udp(names[i] << ":Udp", STARTING_PORT + i));
  }

  // Create the link tables and links themselves.  Links must be
  // symmetric
  ValuePtr LINK = Val_Str::mk("Link");
  for (int i = 0;
       i < nodes;
       i++) {
    linkTables[i].reset(new Table(string("Link:") + names[i], 10000));
    linkTables[i]->add_multiple_index(1);
    reachTables[i].reset(new Table(string("Reach:") + names[i], 10000));
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

  PlumberPtr plumber(new Plumber(conf, level));
  if (plumber->initialize(plumber) == 0) {
    std::cout << "Correctly initialized network of reachability flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the plumber
  plumber->activate();

  // Schedule kill
  //delaycb(10, 0, wrap(&killJoin));

  // Run the plumber
  eventLoop();
}



/** Test over a ring. */
void testSimpleCycle(LoggerI::Level level)
{
  std::cout << "\nCHECK TRANSITIVE REACHABILITY ON RING\n";

  Plumber::ConfigurationPtr conf(new Plumber::Configuration());


  // Create one data flow per "node"
  ptr< Udp > udps[nodes];
  TablePtr linkTables[nodes];
  TablePtr reachTables[nodes];
  ValuePtr nodeIds[nodes];
  string names[nodes];
  
  // Create the networking objects
  for (int i = 0;
       i < nodes;
       i++) {
    ostringstream o1;
    ostringstream o2;
    o1 << string("Node") << i;
    o2 << string("127.0.0.1") << ":" << (STARTING_PORT + i);
    names[i] = o1.str();
    nodeIds[i] = Val_Str::mk(o2.str());

    udps[i].reset(new Udp(names[i] << ":Udp", STARTING_PORT + i));
  }

  // Create the link tables and links themselves.  Links must be
  // symmetric
  ValuePtr LINK = Val_Str::mk("Link");
  for (int i = 0;
       i < nodes;
       i++) {
    linkTables[i].reset(new Table(string("Link:") + names[i], 10000));
    linkTables[i]->add_multiple_index(1);
    reachTables[i].reset(new Table(string("Reach:") + names[i], 10000));
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

  PlumberPtr plumber(new Plumber(conf, level));
  if (plumber->initialize(plumber) == 0) {
    std::cout << "Correctly initialized network of reachability flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the plumber
  plumber->activate();

  // Schedule kill
  //delaycb(10, 0, wrap(&killJoin));

  // Run the plumber
  eventLoop();
}


int main(int argc, char **argv)
{
  std::cout << "\nGRAPH REACHABILITY\n";

  TablePtr table = new refcounted< Table >("name", 100);

  LoggerI::Level level = LoggerI::ALL;
  if (argc > 1) {
    string levelName(argv[1]);
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
