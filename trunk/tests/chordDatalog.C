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
bool CC = false;
bool TEST_SUCCESSOR = false;
bool TEST_LOOKUP = false;
int JOIN_ATTEMPTS = 4;

void killJoin()
{
  exit(0);
}

string LOCAL("127.0.0.1:10000");
string REMOTE("Remote.com");
string FINGERIP("Finger.com");

struct SuccessorGenerator : public FunctorSource::Generator
{
  TuplePtr operator()() 
  {
    
    TuplePtr tuple = Tuple::mk();
    tuple->append(Val_Str::mk("succ"));
    
    string myAddress = LOCAL;
    tuple->append(Val_Str::mk(myAddress));
    
    IDPtr successor = ID::mk((uint32_t) rand());
    tuple->append(Val_ID::mk(successor));

    string succAddress = successor->toString() + "IP";
    tuple->append(Val_Str::mk(succAddress));
    tuple->freeze();
    return tuple;
  }
};


// this allows us to isolate and test just the finger lookup rules
void fakeFingersSuccessors(boost::shared_ptr< OL_Context> ctxt, boost::shared_ptr<Rtr_ConfGen> routerConfigGenerator, string localAddress, IDPtr me)
{
  TablePtr fingerTable = routerConfigGenerator->getTableByName(localAddress, "finger");
  OL_Context::TableInfo* fingerTableInfo = ctxt->getTableInfos()->find("finger")->second;  

  // Fill up the table with fingers
  for (int i = 0; i < fingerTableInfo->size; i++) {
    TuplePtr tuple = Tuple::mk();
    tuple->append(Val_Str::mk("finger"));

    string myAddress = localAddress;
    tuple->append(Val_Str::mk(myAddress));
    tuple->append(Val_Int32::mk(i));

    IDPtr target = ID::mk((uint32_t) 0X1)->shift(i)->add(me);
    IDPtr best = ID::mk()->add(target)->add(ID::mk((uint32_t) i*10));
    tuple->append(Val_ID::mk(best));
  
    ostringstream address;
    address << "Finger:" << (i / 2);
    tuple->append(Val_Str::mk(address.str()));
    tuple->freeze();
    fingerTable->insert(tuple);
    warn << tuple->toString() << "\n";
  }


  // fake the best successor table. Only for testing purposes.  
  TablePtr bestSuccessorTable = routerConfigGenerator->getTableByName(localAddress, "bestSucc");
  TuplePtr tuple = Tuple::mk();
  tuple->append(Val_Str::mk("bestSucc"));
  
  string myAddress = localAddress;
  tuple->append(Val_Str::mk(myAddress));
  
  IDPtr target = ID::mk((uint32_t) 0X1)->add(me);
  tuple->append(Val_ID::mk(target));
  
  ostringstream address;
  address << "Finger:" << 0;
  tuple->append(Val_Str::mk(address.str()));
  tuple->freeze();
  
  bestSuccessorTable->insert(tuple);
  warn << "BestSucc: " << tuple->toString() << "\n";
}


void initializeBaseTables(boost::shared_ptr< OL_Context> ctxt, boost::shared_ptr<Rtr_ConfGen> routerConfigGenerator, 
			  string localAddress, string landmarkAddress)
{

 // create information on the node itself  
  uint32_t random[ID::WORDS];
  for (uint32_t i = 0; i < ID::WORDS; i++) {
    random[i] = rand();
  }
  
  IDPtr myKey = ID::mk(random);
  //IDPtr myKey = ID::mk((uint32_t) 1);

  if (TEST_LOOKUP) {
    // fake fingers and best successors
    fakeFingersSuccessors(ctxt, routerConfigGenerator, localAddress, myKey);  
  }

  TablePtr nodeTable = routerConfigGenerator->getTableByName(localAddress, "node");
  TuplePtr tuple = Tuple::mk();
  tuple->append(Val_Str::mk("node"));
    
  string myAddress = localAddress;
  tuple->append(Val_Str::mk(myAddress));
  tuple->append(Val_ID::mk(myKey));
  tuple->freeze();
  nodeTable->insert(tuple);
  warn << "Node: " << tuple->toString() << "\n";

  TablePtr predecessorTable = routerConfigGenerator->getTableByName(localAddress, "pred");
  TuplePtr predecessorTuple = Tuple::mk();
  predecessorTuple->append(Val_Str::mk("pred"));
  predecessorTuple->append(Val_Str::mk(localAddress));
  predecessorTuple->append(Val_ID::mk(ID::mk()));
  predecessorTuple->append(Val_Str::mk(string("-"))); 
  predecessorTuple->freeze();
  predecessorTable->insert(predecessorTuple);
  warn << "Initial predecessor " << predecessorTuple->toString() << "\n";

  TablePtr nextFingerFixTable = routerConfigGenerator->getTableByName(localAddress, "nextFingerFix");
  TuplePtr nextFingerFixTuple = Tuple::mk();
  nextFingerFixTuple->append(Val_Str::mk("nextFingerFix"));
  nextFingerFixTuple->append(Val_Str::mk(localAddress));
  nextFingerFixTuple->append(Val_UInt32::mk(0));
  nextFingerFixTuple->freeze();
  nextFingerFixTable->insert(nextFingerFixTuple);
  warn << "Next finger fix: " << nextFingerFixTuple->toString() << "\n";

  TablePtr landmarkNodeTable = routerConfigGenerator->getTableByName(localAddress, "landmark");  
  TuplePtr landmark = Tuple::mk();
  landmark->append(Val_Str::mk("landmark"));
  landmark->append(Val_Str::mk(localAddress));
  landmark->append(Val_Str::mk(landmarkAddress));
  landmark->freeze();
  warn << "Insert landmark node " << landmark->toString() << "\n";
  landmarkNodeTable->insert(landmark);
}


void sendSuccessorStream(boost::shared_ptr< Udp> udp, boost::shared_ptr< Router::Configuration > conf, string localAddress)
{
  // have something that populates the table of successors. For testing purposes
  SuccessorGenerator* successorGenerator = new SuccessorGenerator();
  ElementSpecPtr sourceS =
    conf->addElement(ElementPtr(new FunctorSource(string("SuccessorSource:"),
						     successorGenerator)));
  
  ElementSpecPtr print   = conf->addElement(ElementPtr(new Print(string("SuccessorSource"))));
  
  // The timed pusher
  ElementSpecPtr pushS =
    conf->addElement(ElementPtr(new TimedPullPush(string("SuccessorPush:"), 2)));
  
  // And a slot from which to pull
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("SuccessorSlot:"))));
  
  ElementSpecPtr encap = conf->addElement(ElementPtr(new PelTransform("SuccessorEncap",
									 "$1 pop \
                                                     $0 ->t $1 append $2 append $3 append pop"))); // the rest
  ElementSpecPtr marshal = conf->addElement(ElementPtr(new MarshalField("SuccessorMarshalField", 1)));
  ElementSpecPtr route   = conf->addElement(ElementPtr(new StrToSockaddr(string("SuccessorStrToSocket"), 0)));
  
  ElementSpecPtr udpTx = conf->addElement(udp->get_tx());
  
  conf->hookUp(sourceS, 0, print, 0);
  conf->hookUp(print, 0, pushS, 0);
  conf->hookUp(pushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, encap, 0);
  conf->hookUp(encap, 0, marshal, 0);
  conf->hookUp(marshal, 0, route, 0);
  conf->hookUp(route, 0, udpTx, 0);
  
}


void initiateJoinRequest(boost::shared_ptr< Rtr_ConfGen > routerConfigGenerator, 
                         boost::shared_ptr< Router::Configuration > conf, 
			 string localAddress, double delay)
{

 // My next finger fix tuple
  TuplePtr joinEventTuple = Tuple::mk();
  joinEventTuple->append(Val_Str::mk("join"));
  joinEventTuple->append(Val_Str::mk(localAddress));
  joinEventTuple->append(Val_Int32::mk(random()));
  joinEventTuple->freeze();

  ElementSpecPtr sourceS =
    conf->addElement(ElementPtr(new TupleSource(string("JoinEventSource:") + localAddress,
                                                   joinEventTuple)));
  
  // The once pusher
  ElementSpecPtr onceS =
    conf->addElement(ElementPtr(new TimedPullPush(string("JoinEventPush:") + localAddress,
                                                     delay, // run immediately
                                                     JOIN_ATTEMPTS // run once
                                                     )));

  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("JoinEventSlot:"))));

  // Link everything
  conf->hookUp(sourceS, 0, onceS, 0);
  conf->hookUp(onceS, 0, slotS, 0);

  routerConfigGenerator->registerUDPPushSenders(slotS);
}


/** Test lookups. */
void startChordInDatalog(LoggerI::Level level, boost::shared_ptr< OL_Context> ctxt, string datalogFile, 
			 string localAddress, int port, string landmarkAddress, 
			 double delay)
{
  // create dataflow for translated chord lookup rules
  Router::ConfigurationPtr conf(new Router::Configuration());
  boost::shared_ptr< Rtr_ConfGen > routerConfigGenerator(new Rtr_ConfGen(ctxt.get(), conf, false, DEBUG, CC, datalogFile));

  routerConfigGenerator->createTables(localAddress);

  boost::shared_ptr< Udp > udp(new Udp(localAddress, port));
  routerConfigGenerator->clear();
  initiateJoinRequest(routerConfigGenerator, conf, localAddress, delay);
  routerConfigGenerator->configureRouter(udp, localAddress);

  initializeBaseTables(ctxt, routerConfigGenerator, localAddress, landmarkAddress);
   
  // synthetically generate stream of successors to test replacement policies
  // at one node
  boost::shared_ptr< Udp > bootstrapUdp(new Udp(localAddress, 20000 + port));
  if (TEST_SUCCESSOR) { 
    sendSuccessorStream(bootstrapUdp, conf, localAddress);
  }

  RouterPtr router(new Router(conf, level));
  if (router->initialize(router) == 0) {
    warn << "Correctly initialized network of chord lookup flows.\n";
  } else {
    warn << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}


