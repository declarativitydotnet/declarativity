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
#include "pushprint.h"
#include "print.h"
#include "elementSpec.h"
#include "router.h"
#include "discard.h"

/** Test the Logger. */
void testLogger()
{
  std::cout << "\nCHECK LOGGER\n";

  ref<Logger> log = New refcounted<Logger>();
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef logSpec = conf->addElement(log);
  ElementSpecRef printSpec = conf->addElement(New refcounted<PushPrint>());
  conf->hookUp(logSpec,0,printSpec,0);

  MasterRef master = New refcounted< Master >();
  RouterRef router =
    New refcounted< Router >(conf, master);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized spec.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

  std::cout << "Router activated, captain.\n";

  for( int i=0; i<5; i++) {
    log->log( "test class",
	      "test instance",
	      Logger::WARN,
	      i, 
	      "Test message");
  }
}

/** Test the Logger using print and discard. */
void testLoggerWithPrint()
{
  std::cout << "\nCHECK LOGGER with PRINT and DISCARD\n";

  ref<Logger> log = New refcounted<Logger>();
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  ElementSpecRef logSpec = conf->addElement(log);
  ElementSpecRef printSpec = conf->addElement(New refcounted<Print>("Logger"));
  ElementSpecRef discardSpec = conf->addElement(New refcounted<Discard>());
  conf->hookUp(logSpec,0,printSpec,0);
  conf->hookUp(printSpec,0,discardSpec,0);

  MasterRef master = New refcounted< Master >();
  RouterRef router =
    New refcounted< Router >(conf, master);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized spec.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

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
  testLoggerWithPrint();

  return 0;
}
  

/*
 * End of file 
 */
 
