// -*- c-basic-offset: 2; related-file-name: "" -*-
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

#include "ol_lexer.h"
#include "ol_context.h"
#include "rtr_confgen.h"
#include "udp.h"



bool DEBUG = false;
bool TEST_SUCCESSOR = false;
bool TEST_LOOKUP = false;


void killJoin()
{
  exit(0);
}

str LOCAL("127.0.0.1:10000");
str REMOTE("Remote.com");
str FINGERIP("Finger.com");

struct SuccessorGenerator : public FunctorSource::Generator
{
  TupleRef operator()() 
  {
    
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("succ"));
    
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


// this allows us to isolate and test just the finger lookup rules
void fakeFingersSuccessors(ref< OL_Context> ctxt, ref<Rtr_ConfGen> routerConfigGenerator, str localAddress, IDRef me)
{
  TableRef fingerTable = routerConfigGenerator->getTableByName(localAddress, "finger");
  OL_Context::TableInfo* fingerTableInfo = ctxt->getTableInfos()->find("finger")->second;  

  // Fill up the table with fingers
  for (int i = 0; i < fingerTableInfo->size; i++) {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("finger"));

    str myAddress = str(strbuf() << localAddress);
    tuple->append(Val_Str::mk(myAddress));
    tuple->append(Val_Int32::mk(i));

    IDRef target = ID::mk((uint32_t) 0X1)->shift(i)->add(me);
    IDRef best = ID::mk()->add(target)->add(ID::mk((uint32_t) i*10));
    tuple->append(Val_ID::mk(best));
  
    str address = str(strbuf() << "Finger:" << (i / 2));
    tuple->append(Val_Str::mk(address));
    tuple->freeze();
    fingerTable->insert(tuple);
    std::cout << tuple->toString() << "\n";
  }


  // fake the best successor table. Only for testing purposes.  
  TableRef bestSuccessorTable = routerConfigGenerator->getTableByName(localAddress, "bestSucc");
  TupleRef tuple = Tuple::mk();
  tuple->append(Val_Str::mk("bestSucc"));
  
  str myAddress = str(strbuf() << localAddress);
  tuple->append(Val_Str::mk(myAddress));
  
  IDRef target = ID::mk((uint32_t) 0X1)->add(me);
  tuple->append(Val_ID::mk(target));
  
  str address = str(strbuf() << "Finger:" << 0);
  tuple->append(Val_Str::mk(address));
  tuple->freeze();
  
  bestSuccessorTable->insert(tuple);
  std::cout << "BestSucc: " << tuple->toString() << "\n";
}


void initializeBaseTables(ref< OL_Context> ctxt, ref<Rtr_ConfGen> routerConfigGenerator, str localAddress)
{

 // create information on the node itself  
  uint32_t random[ID::WORDS];
  for (uint32_t i = 0; i < ID::WORDS; i++) {
    random[i] = rand();
  }
  
  //IDRef myKey = ID::mk(random);
  IDRef myKey = ID::mk((uint32_t) 1);

  if (TEST_LOOKUP) {
    // fake fingers and best successors
    fakeFingersSuccessors(ctxt, routerConfigGenerator, localAddress, myKey);  
  }

  TableRef nodeTable = routerConfigGenerator->getTableByName(localAddress, "node");
  TupleRef tuple = Tuple::mk();
  tuple->append(Val_Str::mk("node"));
    
  str myAddress = str(strbuf() << localAddress);
  tuple->append(Val_Str::mk(myAddress));
  tuple->append(Val_ID::mk(myKey));
  tuple->freeze();
  nodeTable->insert(tuple);
  std::cout << "Node: " << tuple->toString() << "\n";
    
  /*TableRef nextFingerFixTable = routerConfigGenerator->getTableByName(localAddress, "nextFingerFix");
  TupleRef nextFingerFixTuple = Tuple::mk();
  nextFingerFixTuple->append(Val_Str::mk("nextFingerFix"));
  nextFingerFixTuple->append(Val_Str::mk(localAddress));
  nextFingerFixTuple->append(Val_Int32::mk(0));
  nextFingerFixTuple->freeze();
  nextFingerFixTable->insert(nextFingerFixTuple);
  std::cout << "Next finger fix: " << nextFingerFixTuple->toString() << "\n";*/  
}


void sendSuccessorStream(ref< Udp> udp, ref< Router::Configuration > conf, str localAddress)
{
  // have something that populates the table of successors. For testing purposes
  SuccessorGenerator* successorGenerator = new SuccessorGenerator();
  ElementSpecRef sourceS =
    conf->addElement(New refcounted< FunctorSource >(str("SuccessorSource:"),
						     successorGenerator));
  
  ElementSpecRef print   = conf->addElement(New refcounted< Print >(strbuf("SuccessorSource")));
  
  // The timed pusher
  ElementSpecRef pushS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("SuccessorPush:"),
						     2));
  
  // And a slot from which to pull
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("SuccessorSlot:")));
  
  ElementSpecRef encap = conf->addElement(New refcounted< PelTransform >("SuccessorEncap",
									 "$1 pop \
                                                     $0 ->t $1 append $2 append $3 append pop")); // the rest
  ElementSpecRef marshal = conf->addElement(New refcounted< MarshalField >("SuccessorMarshalField", 1));
  ElementSpecRef route   = conf->addElement(New refcounted< StrToSockaddr >(strbuf("SuccessorStrToSocket"), 0));
  
  ElementSpecRef udpTx = conf->addElement(udp->get_tx());
  
  conf->hookUp(sourceS, 0, print, 0);
  conf->hookUp(print, 0, pushS, 0);
  conf->hookUp(pushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, encap, 0);
  conf->hookUp(encap, 0, marshal, 0);
  conf->hookUp(marshal, 0, route, 0);
  conf->hookUp(route, 0, udpTx, 0);
  
}


void initiateJoinRequest(ref< Rtr_ConfGen > routerConfigGenerator, ref< Router::Configuration > conf, 
			 str localAddress, double delay)
{

 // My next finger fix tuple
  TupleRef joinEventTuple = Tuple::mk();
  joinEventTuple->append(Val_Str::mk("join"));
  joinEventTuple->append(Val_Str::mk(localAddress));
  joinEventTuple->append(Val_Int32::mk(random()));
  joinEventTuple->freeze();

  ElementSpecRef sourceS =
    conf->addElement(New refcounted< TupleSource >(str("JoinEventSource:") << localAddress,
                                                   joinEventTuple));
  
  // The once pusher
  ElementSpecRef onceS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("JoinEventPush:") << localAddress,
                                                     delay, // run immediately
                                                     1 // run once
                                                     ));

  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("JoinEventSlot:")));

  // Link everything
  conf->hookUp(sourceS, 0, onceS, 0);
  conf->hookUp(onceS, 0, slotS, 0);

  routerConfigGenerator->registerUDPPushSenders(slotS);
}


/** Test lookups. */
void startChordInDatalog(LoggerI::Level level, ref< OL_Context> ctxt, str datalogFile, 
			 str localAddress, int port, str landmarkAddress, 
			 double delay)
{
  // create dataflow for translated chord lookup rules
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ref< Rtr_ConfGen > routerConfigGenerator 
    = New refcounted< Rtr_ConfGen >(ctxt, conf, false, DEBUG, datalogFile);

  routerConfigGenerator->createTables(localAddress);

  ref< Udp > udp = New refcounted< Udp > (localAddress, port);
  routerConfigGenerator->clear();
  initiateJoinRequest(routerConfigGenerator, conf, localAddress, delay);
  routerConfigGenerator->configureRouter(udp, localAddress);

  initializeBaseTables(ctxt, routerConfigGenerator, localAddress);
   
  // synthetically generate stream of successors to test replacement policies
  // at one node
  ref< Udp > bootstrapUdp = New refcounted< Udp > (localAddress, 9999);
  if (TEST_SUCCESSOR) { 
    sendSuccessorStream(bootstrapUdp, conf, localAddress);
  }

  TableRef landmarkNodeTable = routerConfigGenerator->getTableByName(localAddress, "landmark");  
  TupleRef landmark = Tuple::mk();
  landmark->append(Val_Str::mk("landmark"));
  landmark->append(Val_Str::mk(localAddress));
  landmark->append(Val_Str::mk(landmarkAddress));
  landmark->freeze();
  std::cout << "Insert landmark node " << landmark->toString() << "\n";
  landmarkNodeTable->insert(landmark);
  
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


