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
#include "router.h"

#include "print.h"
#include "timedPushSource.h"
#include "timedPullSink.h"
#include "pelTransform.h"
#include "slot.h"

/** Test push chain with fast source. */
void testFastSourcePush()
{
  std::cout << "\nCHECK FAST SOURCE PUSH\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted<
                                            TimedPushSource >("source", 0.2));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink
                                          >("sink", 1));
  ElementSpecRef slotS = conf->addElement(New refcounted< Slot >("slot"));
  ElementSpecRef sinkPrintS = conf->addElement(New refcounted< Print >("BeforeSink"));
  ElementSpecRef sourcePrintS = conf->addElement(New refcounted< Print >("AfterSource"));
  ElementSpecRef transS = conf->addElement(New refcounted< PelTransform
                                           >("trans", "$1 10 % ifpoptuple"));

  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, transS, 0);
  conf->hookUp(transS, 0, slotS, 0);
  conf->hookUp(slotS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized fast source push.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}

/** Test push chain with fast sink. */
void testFastSinkPush()
{
  std::cout << "\nCHECK FAST SINK PUSH\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted<
                                            TimedPushSource >("source", 1));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink
                                          >("sink", .2));
  ElementSpecRef slotS = conf->addElement(New refcounted< Slot >("slot"));
  ElementSpecRef sinkPrintS = conf->addElement(New refcounted< Print >("BeforeSink"));
  ElementSpecRef sourcePrintS = conf->addElement(New refcounted< Print >("AfterSource"));
  ElementSpecRef transS = conf->addElement(New refcounted< PelTransform
                                           >("trans", "$1 10 % ifpoptuple"));

  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, transS, 0);
  conf->hookUp(transS, 0, slotS, 0);
  conf->hookUp(slotS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized fast sink push.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}

/** Test push chain with fast sink. */
void testFastSinkPull()
{
  std::cout << "\nCHECK FAST SINK PULL\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted<
                                            TimedPushSource >("source", 1));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink
                                          >("sink", .2));
  ElementSpecRef slotS = conf->addElement(New refcounted< Slot >("slot"));
  ElementSpecRef sinkPrintS = conf->addElement(New refcounted< Print >("BeforeSink"));
  ElementSpecRef sourcePrintS = conf->addElement(New refcounted< Print >("AfterSource"));
  ElementSpecRef transS = conf->addElement(New refcounted< PelTransform
                                           >("trans", "$1 10 % ifpoptuple"));

  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized fast sink pull.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}

/** Test push chain with fast sink. */
void testFastSourcePull()
{
  std::cout << "\nCHECK FAST SOURCE PULL\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted<
                                            TimedPushSource >("source", .2));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink
                                          >("sink", 1));
  ElementSpecRef slotS = conf->addElement(New refcounted< Slot >("slot"));
  ElementSpecRef sinkPrintS = conf->addElement(New refcounted< Print >("BeforeSink"));
  ElementSpecRef sourcePrintS = conf->addElement(New refcounted< Print >("AfterSource"));
  ElementSpecRef transS = conf->addElement(New refcounted< PelTransform
                                           >("trans", "$1 10 % ifpoptuple"));

  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized fast sink pull.\n";
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

  testFastSourcePull();

  return 0;
}
  

/*
 * End of file 
 */
