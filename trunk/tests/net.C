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
 * DESCRIPTION: Tests for UDP
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>

#include "tuple.h"
#include "pullprint.h"
#include "pushprint.h"
#include "print.h"
#include "memoryPull.h"
#include "router.h"
#include "master.h"
#include "timedSource.h"
#include "udp.h"
//#include "plsensor.h"
#include "discard.h"
#include "marshal.h"
#include "route.h"
#include "pelTransform.h"
#include "unmarshal.h"

/** Test an element chain that packages a tuple and sends it over
    Udp. */
void testUdpTx()
{
  std::cout << "\nCHECK UDP Transmit\n";

  ref< TimedSource > timedSource = New refcounted< TimedSource >(0.5);
  ref< Marshal > marshal = New refcounted< Marshal >();
  std::cerr << "No created udps\n";
  Udp udpOut(9999);
  std::cerr << "Created 9999\n";
  Udp udpIn(10000);
  std::cerr << "Created 10000\n";
  str destinationAddr = "127.0.0.1";
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_port = htons(10000);
  inet_pton(AF_INET, destinationAddr.cstr(),
            &addr.sin_addr);
  ref< suio > addressUio = New refcounted< suio >();
  addressUio->copy(&addr, sizeof(addr));
  ref< Route > route = New refcounted< Route >(addressUio);
  ref< PelTransform > unRoute = New refcounted< PelTransform >("$1 pop");
  ref< Unmarshal > unmarshal = New refcounted< Unmarshal >();
  ref< Discard > discard = New refcounted< Discard >();
  ref< Print > print = New refcounted< Print >("Received");
  ref< Print > print2 = New refcounted< Print >("Received");
  ref< Print > print3 = New refcounted< Print >("PostProj");
  ref< Print > print4 = New refcounted< Print >("Created");
  ref< Print > print5 = New refcounted< Print >("Marshalled");
  ref< Print > print6 = New refcounted< Print >("Routed");


  ElementSpecRef timedSourceSpec = New refcounted< ElementSpec >(timedSource);
  ElementSpecRef marshalSpec = New refcounted< ElementSpec >(marshal);
  ElementSpecRef routeSpec = New refcounted< ElementSpec >(route);
  ElementSpecRef udpTxSpec = New refcounted< ElementSpec >(udpOut.get_tx());
  ElementSpecRef udpRxSpec = New refcounted< ElementSpec >(udpIn.get_rx());
  ElementSpecRef unRouteSpec = New refcounted< ElementSpec >(unRoute);
  ElementSpecRef unmarshalSpec = New refcounted< ElementSpec >(unmarshal);
  ElementSpecRef discardSpec = New refcounted< ElementSpec >(discard);
  ElementSpecRef printSpec = New refcounted< ElementSpec >(print);
  ElementSpecRef printSpec2 = New refcounted< ElementSpec >(print2);
  ElementSpecRef printSpec3 = New refcounted< ElementSpec >(print3);
  ElementSpecRef printSpec4 = New refcounted< ElementSpec >(print4);
  ElementSpecRef printSpec5 = New refcounted< ElementSpec >(print5);
  ElementSpecRef printSpec6 = New refcounted< ElementSpec >(print6);

  ref< vec< ElementSpecRef > > elements = New refcounted< vec< ElementSpecRef > >();
  elements->push_back(timedSourceSpec);
  elements->push_back(marshalSpec);
  elements->push_back(routeSpec);
  elements->push_back(udpTxSpec);
  elements->push_back(udpRxSpec);
  elements->push_back(unRouteSpec);
  elements->push_back(unmarshalSpec);
  elements->push_back(discardSpec);
  elements->push_back(printSpec);
  elements->push_back(printSpec2);
  elements->push_back(printSpec3);
  elements->push_back(printSpec4);
  elements->push_back(printSpec5);
  elements->push_back(printSpec6);

  Router::HookupPtr hookup;
  ref < vec< Router::HookupRef > > hookups =
    New refcounted< vec< Router::HookupRef > >();

  hookups->clear();
  hookup = New refcounted< Router::Hookup >(timedSourceSpec, 0,
                                            printSpec4, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(printSpec4, 0,
                                            marshalSpec, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(marshalSpec, 0,
                                            printSpec5, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(printSpec5, 0,
                                            routeSpec, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(routeSpec, 0,
                                            printSpec6, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(printSpec6, 0,
                                            udpTxSpec, 0);
  hookups->push_back(hookup);



  hookup = New refcounted< Router::Hookup >(udpRxSpec, 0,
                                            printSpec2, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(printSpec2, 0, 
                                            unRouteSpec, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(unRouteSpec, 0, 
                                            printSpec3, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(printSpec3, 0, 
                                            unmarshalSpec, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(unmarshalSpec, 0,
                                            printSpec, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(printSpec, 0,
                                            discardSpec, 0);
  hookups->push_back(hookup);



  Router::ConfigurationRef configuration =
    New refcounted< Router::Configuration >(elements, hookups);
  MasterRef master = New refcounted< Master >();
  RouterRef router =
    New refcounted< Router >(configuration, master);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}


/** Test the Rx part of the Udp element. */
void testUdpReceive()
{
  std::cout << "\nCHECK UDP receive\n";

  Router::ConfigurationRef conf = New refcounted<Router::Configuration>();
  Udp udp(9999);

  ElementSpecRef udpRxSpec = conf->addElement(udp.get_rx());
  ElementSpecRef printSpec = conf->addElement(New refcounted<Print>("Printer"));
  ElementSpecRef discardSpec = conf->addElement(New refcounted<Discard>());
  conf->hookUp(udpRxSpec,0,printSpec,0);
  conf->hookUp(printSpec,0,discardSpec,0);

  // Create the router and check it statically
  MasterRef master = New refcounted< Master >();
  RouterRef router = New refcounted< Router >(conf, master);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized timed source to pull print spec.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}
#

#if 0
/** Test the Rx part of the Udp element. */
void testPLSensor()
{
  std::cout << "\nCHECK PL SENSOR\n";

  ref<PlSensor> pl = New refcounted<PlSensor>((uint16_t)80,"/", 5);
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef plSpec = conf->addElement(pl);
  ElementSpecRef printSpec = conf->addElement(New refcounted<PushPrint>());
  conf->hookUp(plSpec,0,printSpec,0);

  // Create the router and check it statically
  MasterRef master = New refcounted< Master >();
  RouterRef router = New refcounted< Router >(conf, master);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized PlSensor to push print spec.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}
#endif 

int main(int argc, char **argv)
{
  std::cout << "\nNET\n";

  testUdpTx();
  //  testPLSensor();

  return 0;
}
  

/*
 * End of file 
 */
