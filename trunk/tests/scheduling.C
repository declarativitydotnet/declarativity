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

int main(int argc, char **argv)
{
  std::cout << "\nSCHEDULING\n";

  testSinglePuller();

  return 0;
}
  

/*
 * End of file 
 */
