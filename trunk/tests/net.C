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
#include "project.h"
#include "udp.h"
#include "discard.h"

/** Test the Rx part of the Udp element. */
void testUdpReceive()
{
  std::cout << "\nCHECK UDP receive\n";

  // Create the UDP element
  Udp udp(9999);
  ref< Udp::Rx > rx = udp.get_rx();
  ElementSpecRef udpRxSpec = New refcounted< ElementSpec >(rx);

  ref< Print > print = New refcounted< Print >("Printer");
  ElementSpecRef printSpec = New refcounted< ElementSpec >(print);

  ref< Discard > discard = New refcounted< Discard >();
  ElementSpecRef discardSpec = New refcounted< ElementSpec >(discard);


  // Create the configuration
  ref< vec< ElementSpecRef > > elements = New refcounted< vec< ElementSpecRef > >();
  elements->push_back(udpRxSpec);
  elements->push_back(printSpec);
  elements->push_back(discardSpec);

  Router::HookupPtr hookup;
  ref < vec< Router::HookupRef > > hookups =
    New refcounted< vec< Router::HookupRef > >();

  hookups->clear();
  hookup = New refcounted< Router::Hookup >(udpRxSpec, 0,
                                            printSpec, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(printSpec, 0,
                                            discardSpec, 0);
  hookups->push_back(hookup);


  // Create the router and check it statically
  Router::ConfigurationRef configuration =
    New refcounted< Router::Configuration >(elements, hookups);
  MasterRef master = New refcounted< Master >();
  RouterRef router =
    New refcounted< Router >(configuration, master);
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

int main(int argc, char **argv)
{
  std::cout << "\nNET\n";

  testUdpReceive();

  return 0;
}
  

/*
 * End of file 
 */
