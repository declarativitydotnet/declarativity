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
  ElementSpecRef logSpec = New refcounted< ElementSpec >(log);

  ref< PushPrint > print = New refcounted< PushPrint >();
  ElementSpecRef printSpec = New refcounted< ElementSpec >(print);

  ref< Discard > discard = New refcounted< Discard >();
  ElementSpecRef discardSpec = New refcounted< ElementSpec >(discard);

  // Create the configuration
  ref< vec< ElementSpecRef > > elements = New refcounted< vec< ElementSpecRef > >();
  elements->push_back(logSpec);
  elements->push_back(printSpec);
  //elements->push_back(discardSpec);
  
  Router::HookupPtr hookup;
  ref < vec< Router::HookupRef > > hookups =
    New refcounted< vec< Router::HookupRef > >();

  hookups->clear();
  hookup = New refcounted< Router::Hookup >(logSpec, 0,
                                            printSpec, 0);
  hookups->push_back(hookup);

  // Create the router and check it statically
  Router::ConfigurationRef configuration =
    New refcounted< Router::Configuration >(elements, hookups);
  MasterRef master = New refcounted< Master >();
  RouterRef router =
    New refcounted< Router >(configuration, master);
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

  return 0;
}
  

/*
 * End of file 
 */
 
