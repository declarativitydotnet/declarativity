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

  testUdpReceive();
  //  testPLSensor();

  return 0;
}
  

/*
 * End of file 
 */
