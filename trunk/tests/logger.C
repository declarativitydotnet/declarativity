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
 * DESCRIPTION: Tests for logger element
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>

#include "logger.h"
#include "print.h"

#include "elementSpec.h"
#include "router.h"
#include "print.h"
#include "discard.h"
#include "slot.h"


/** Test the Logger. */
void testLogger()
{
  std::cout << "\nCHECK LOGGER\n";

  ref<Logger> log = New refcounted<Logger>();
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef logSpec = conf->addElement(log);
  ElementSpecRef sinkPrintS = conf->addElement(New refcounted< Print >("BeforeSink"));
  ElementSpecRef sinkS = conf->addElement(New refcounted< Discard >());
  conf->hookUp(logSpec, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);

  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized spec.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();
  router->logger(log);

  std::cout << "Router activated, captain.\n";

  for( int i=0; i<5; i++) {
    log->log( "test class",
	      "test instance",
	      Logger::WARN,
	      i, 
	      "Test message");
  }
}

int main(int argc, char **argv)
{
  std::cout << "\nLOGGER\n";

  testLogger();

  return 0;
}
  

/*
 * End of file 
 */
 
