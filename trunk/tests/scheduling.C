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
 * DESCRIPTION: Tests for scheduling, timed tasks, repeated tasks,
 * queues, etc.
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
#include "pelTransform.h"
#include "logger.h"

/** Test that a puller runs and stops during inaction. */
void testSinglePuller()
{
  std::cout << "\nCHECK SIMPLE PULLER\n";

  ref< TimedSource > timedSource = New refcounted< TimedSource >(0.8);
  ref< PullPrint > pullPrint = New refcounted< PullPrint >();
  ElementSpecRef timedSourceSpec = New refcounted< ElementSpec >(timedSource);
  ElementSpecRef pullPrintSpec = New refcounted< ElementSpec >(pullPrint);

  ref< vec< ElementSpecRef > > elements = New refcounted< vec< ElementSpecRef > >();
  elements->push_back(timedSourceSpec);
  elements->push_back(pullPrintSpec);

  Router::HookupPtr hookup;
  ref < vec< Router::HookupRef > > hookups =
    New refcounted< vec< Router::HookupRef > >();

  hookups->clear();
  hookup = New refcounted< Router::Hookup >(timedSourceSpec, 0,
                                            pullPrintSpec, 0);
  hookups->push_back(hookup);

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

/** Test a three-element chain with an agnostic in the middle. */
void testChainPuller()
{
  std::cout << "\nCHECK 3-ELEMENT CHAIN\n";

  ref< TimedSource > timedSource = New refcounted< TimedSource >(0.25);
  ref< Print > print = New refcounted< Print >("Before");
  //  ref< PelTransform > project = New refcounted< PelTransform >("$1 $2 +i pop");
  ref< PelTransform > project = New refcounted< PelTransform >("$8 pop");
  ref< Print > print2 = New refcounted< Print >("After");
  ref< PullPrint > pullPrint = New refcounted< PullPrint >();

  ElementSpecRef timedSourceSpec = New refcounted< ElementSpec >(timedSource);
  ElementSpecRef printSpec = New refcounted< ElementSpec >(print);
  ElementSpecRef projectSpec = New refcounted< ElementSpec >(project);
  ElementSpecRef printSpec2 = New refcounted< ElementSpec >(print2);
  ElementSpecRef pullPrintSpec = New refcounted< ElementSpec >(pullPrint);

  // Logger data flow
  ref< Logger > logger = New refcounted< Logger >();
  ElementSpecRef loggerSpec = New refcounted< ElementSpec >(logger);
  ref< Print > logPrinter = New refcounted< Print >("LOGGER");
  ElementSpecRef logPrinterSpec = New refcounted< ElementSpec >(logPrinter);
  ref< Discard > discard = New refcounted< Discard >();
  ElementSpecRef discardSpec = New refcounted< ElementSpec >(discard);

  

  ref< vec< ElementSpecRef > > elements = New refcounted< vec< ElementSpecRef > >();
  elements->push_back(timedSourceSpec);
  elements->push_back(printSpec);
  elements->push_back(printSpec2);
  elements->push_back(projectSpec);
  elements->push_back(pullPrintSpec);

  elements->push_back(loggerSpec);
  elements->push_back(logPrinterSpec);
  elements->push_back(discardSpec);

  Router::HookupPtr hookup;
  ref < vec< Router::HookupRef > > hookups =
    New refcounted< vec< Router::HookupRef > >();

  hookups->clear();
  hookup = New refcounted< Router::Hookup >(timedSourceSpec, 0,
                                            printSpec, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(printSpec, 0,
                                            projectSpec, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(projectSpec, 0,
                                            printSpec2, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(printSpec2, 0,
                                            pullPrintSpec, 0);
  hookups->push_back(hookup);


  hookup = New refcounted< Router::Hookup >(loggerSpec, 0,
                                            logPrinterSpec, 0);
  hookups->push_back(hookup);
  hookup = New refcounted< Router::Hookup >(logPrinterSpec, 0,
                                            discardSpec, 0);
  hookups->push_back(hookup);



  Router::ConfigurationRef configuration =
    New refcounted< Router::Configuration >(elements, hookups);
  MasterRef master = New refcounted< Master >();
  RouterRef router =
    New refcounted< Router >(configuration, master);
  router->logger(logger);
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
  std::cout << "\nSCHEDULING\n";

  testChainPuller();

  return 0;
}
  

/*
 * End of file 
 */
