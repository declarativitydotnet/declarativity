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
 * DESCRIPTION: Phi daemon
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>

#include "plsensor.h"
#include "router.h"

#include "csvparser.h"
#include "print.h"
#include "timedPullSink.h"

const char *path="/snort/tcpconns";
const uint16_t port = 12337;
//const char *path="/slicestat";
//const uint16_t port = 3100;

/** Test the Rx part of the Udp element. */
int main(int argc, char **argv)
{
  std::cout << "\nPhi daemon started\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef pl =
    conf->addElement(New refcounted<PlSensor>(port, path, 30));
  ElementSpecRef csv =
    conf->addElement(New refcounted<CSVParser>());
  ElementSpecRef print =
    conf->addElement(New refcounted<Print>("Printer"));
  ElementSpecRef sink =
    conf->addElement(New refcounted<TimedPullSink>(0));
  conf->hookUp(pl,0,csv,0);
  conf->hookUp(csv,0,print,0);
  conf->hookUp(print,0, sink, 0);

  // Create the router and check it statically
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized configuration.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}
  

/*
 * End of file 
 */
