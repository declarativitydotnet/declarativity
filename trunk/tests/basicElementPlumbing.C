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
 * DESCRIPTION: Tests for simple element plumbing
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
#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"

#include "print.h"
#include "timedPushSource.h"
#include "timedPullSink.h"
#include "slot.h"

TupleRef create_tuple(int i) {
  TupleRef t = Tuple::mk();
  t->append(Val_Null::mk());
  t->append(Val_Int32::mk(i));
  t->append(Val_UInt64::mk(i));
  t->append(Val_Int32::mk(i));
  strbuf myStringBuf;
  myStringBuf << "This is string '" << i << "'";
  str myString = myStringBuf;
  t->append(Val_Str::mk(myString));
  t->freeze();
  std::cout << "Created tuple " << (t->toString()) << "\n";
  return t;
}










////////////////////////////////////////////////////////////
// CHECK HOOKUP ELEMENTS
////////////////////////////////////////////////////////////

/** Hookup referring to non-existent element. */
void testCheckHookupElements_NonExistentToElement()
{
  std::cout << "[Testing non-existent TO element in hookup]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = New refcounted< ElementSpec >(New refcounted< TimedPullSink >(0));
  conf->hookUp(sourceS, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cerr << "** Failed to catch hookup reference to unknown to element\n";
  } else {
    std::cout << "Caught hookup reference to unknown to element\n";
  }
}


/** Non existent from element */
void testCheckHookupElements_NonExistentFromElement()
{
  std::cout << "\n[Non existent FROM element]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = New refcounted< ElementSpec >(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  conf->hookUp(sourceS, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cerr << "** Failed to catch hookup reference to unknown from element\n";
  } else {
    std::cout << "Caught hookup reference to unknown from element\n";
  }
}


/** From port is negative */
void testCheckHookupElements_NegativeFromPort()
{
  std::cout << "\n[Negative From Port]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  conf->hookUp(sourceS, -1, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cerr << "** Failed to catch negative from port\n";
  } else {
    std::cout << "Correctly caught negative from port\n";
  }
}


/** To port is negative */
void testCheckHookupElements_NegativeToPort()
{
  std::cout << "\n[Negative to port]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  conf->hookUp(sourceS, 0, sinkS, -1);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cerr << "** Failed to catch negative to port\n";
  } else {
    std::cout << "Correctly caught negative to port\n";
  }
}

/** Test the portion of router initialization that checks for sanity of
    element hookups and lists */
void testCheckHookupElements()
{
  std::cout << "\nCHECK ELEMENT HOOKUP\n";

  testCheckHookupElements_NonExistentToElement();
  testCheckHookupElements_NonExistentFromElement();
  testCheckHookupElements_NegativeFromPort();
  testCheckHookupElements_NegativeToPort();
}





















////////////////////////////////////////////////////////////
// CHECK HOOKUP RANGE
////////////////////////////////////////////////////////////

/** Incorrect from port. */
void testCheckHookupRange_IncorrectFromPort()
{
  std::cout << "\n[Incorrect From Port]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  conf->hookUp(sourceS, 1, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cerr << "** Failed to catch incorrect from port\n";
  } else {
    std::cout << "Caught incorrect from port\n";
  }
}

/** Incorrect to port. */
void testCheckHookupRange_IncorrectToPort()
{
  std::cout << "\n[Incorrect To Port]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  conf->hookUp(sourceS, 0, sinkS, 1);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cerr << "** Failed to catch incorrect to port\n";
  } else {
    std::cout << "Correctly caught incorrect to port\n";
  }
}

/** Incorrect ports (both). */
void testCheckHookupRange_IncorrectPorts()
{
  std::cout << "\n[Incorrect Ports (Both)]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  conf->hookUp(sourceS, 1, sinkS, 1);
  
  RouterRef router = New refcounted< Router >(conf);

  if (router->initialize(router) == 0) {
    std::cerr << "** Failed to catch incorrect from/to ports\n";
  } else {
    std::cout << "Correctly caught incorrect from/to ports\n";
  }
}

/** Portless hookup. */
void testCheckHookupRange_Portless()
{
  std::cout << "\n[Portless Hookup]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  conf->hookUp(sinkS, 1, sourceS, 1);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cerr << "** Failed to catch portless hookup\n";
  } else {
    std::cout << "Correctly caught portless hookup\n";
  }
}

/** Test the portion of router initialization that checks for port
    number range correctness. */
void testCheckHookupRange()
{
  std::cout << "\nCHECK ELEMENT HOOKUP RANGE\n";

  testCheckHookupRange_IncorrectFromPort();
  testCheckHookupRange_IncorrectToPort();
  testCheckHookupRange_IncorrectPorts();
  testCheckHookupRange_Portless();
}
























////////////////////////////////////////////////////////////
// PUSH and PULL semantics
////////////////////////////////////////////////////////////



/** Pull to push. */
void testCheckPushPull_PullToPush()
{
  std::cout << "\n[Pull to Push]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  ElementSpecRef slot1S = conf->addElement(New refcounted< Slot >());
  ElementSpecRef slot2S = conf->addElement(New refcounted< Slot >());
  conf->hookUp(sourceS, 0, slot1S, 0);
  conf->hookUp(slot1S, 0, slot2S, 0);
  conf->hookUp(slot2S, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cerr << "** Failed to catch pull output hooked up with push input\n";
  } else {
    std::cout << "Caught incorrect pull-to-push hookup\n";
  }
}

/** Pull to Push. */
void testCheckPushPull_PullToPushHop()
{
  std::cout << "\n[Pull to Push with hop]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  ElementSpecRef slot1S = conf->addElement(New refcounted< Slot >());
  ElementSpecRef slot2S = conf->addElement(New refcounted< Slot >());
  ElementSpecRef printS = conf->addElement(New refcounted< Print >("Printer"));

  conf->hookUp(sourceS, 0, slot1S, 0);
  conf->hookUp(slot1S, 0, printS, 0);
  conf->hookUp(printS, 0, slot2S, 0);
  conf->hookUp(slot2S, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cerr << "** Failed to catch incorrect pull-push hookup via a/a element\n";
  } else {
    std::cout << "Correctly caught incorrect transitive pull-push hookup\n";
  }
}

/** Push to pull, multi hop. */
void testCheckPushPull_PullToPushMultiHop()
{
  std::cout << "\n[Pull to Push multi hop]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  ElementSpecRef slot1S = conf->addElement(New refcounted< Slot >());
  ElementSpecRef slot2S = conf->addElement(New refcounted< Slot >());
  ElementSpecRef printS = conf->addElement(New refcounted< Print >("Printer"));
  ElementSpecRef print2S = conf->addElement(New refcounted< Print >("Printer Two"));

  conf->hookUp(sourceS, 0, slot1S, 0);
  conf->hookUp(slot1S, 0, printS, 0);
  conf->hookUp(printS, 0, print2S, 0);
  conf->hookUp(print2S, 0, slot2S, 0);
  conf->hookUp(slot2S, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cerr << "** Failed to catch incorrect pull-push hookup via multiple a/a elements\n";
  } else {
    std::cout << "Correctly caught incorrect transitive (multi hop) pull-push hookup\n";
  }
}

/** Pull to pull multi hop (correct). */
void testCheckPushPull_PullToPullMultiHop()
{
  std::cout << "\n[Pull to pull multi hop]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  ElementSpecRef slot1S = conf->addElement(New refcounted< Slot >());
  ElementSpecRef printS = conf->addElement(New refcounted< Print >("Printer"));
  ElementSpecRef print2S = conf->addElement(New refcounted< Print >("Printer Two"));

  conf->hookUp(sourceS, 0, slot1S, 0);
  conf->hookUp(slot1S, 0, printS, 0);
  conf->hookUp(printS, 0, print2S, 0);
  conf->hookUp(print2S, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);

  if (router->initialize(router) == 0) {
    std::cerr << "Correctly allowed pull-pull hookup via multiple a/a elements\n";
  } else {
    std::cout << "** Caught incorrectly a pull-pull hookup via multiple a/a elements\n";
  }
}

/** Test the portion of router initialization that checks semantic
    consistency. */
void testCheckPushPull()
{
  std::cout << "\nCHECK ELEMENT PUSH and PULL\n";

  testCheckPushPull_PullToPush();
  testCheckPushPull_PullToPushHop();
  testCheckPushPull_PullToPushMultiHop();
  testCheckPushPull_PullToPullMultiHop();
}
















////////////////////////////////////////////////////////////
// Check Duplicates
////////////////////////////////////////////////////////////




/** Unused port */
void testDuplicates_UnusedPort()
{
  std::cout << "\n[Unused Port]\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  ElementSpecRef slot1S = conf->addElement(New refcounted< Slot >());
  ElementSpecRef printS = conf->addElement(New refcounted< Print >("Printer"));
  ElementSpecRef print2S = conf->addElement(New refcounted< Print >("Printer Two"));

  conf->hookUp(sourceS, 0, slot1S, 0);
  conf->hookUp(slot1S, 0, print2S, 0);
  conf->hookUp(print2S, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cerr << "** Incorrectly allowed unused port of an element\n";
  } else {
    std::cout << "Correctly caught unused port of an element\n";
  }
}


/** Reused port */
void testDuplicates_ReusedPort()
{
  std::cout << "\n[Reused Port]\n";
 
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(0));
  ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(0));
  ElementSpecRef slot1S = conf->addElement(New refcounted< Slot >());
  ElementSpecRef printS = conf->addElement(New refcounted< Print >("Printer"));
  ElementSpecRef print2S = conf->addElement(New refcounted< Print >("Printer Two"));

  conf->hookUp(sourceS, 0, slot1S, 0);
  conf->hookUp(slot1S, 0, print2S, 0);
  conf->hookUp(print2S, 0, slot1S, 0);
  conf->hookUp(slot1S, 0, sinkS, 0);
  
  RouterRef router = New refcounted< Router >(conf);

  if (router->initialize(router) == 0) {
    std::cerr << "** Incorrectly allowed port reuse\n";
  } else {
    std::cout << "Correctly caught port reuse\n";
  }
}



/** Test the portion of router initialization that checks
    duplicate/unused ports */
void testDuplicates()
{
  std::cout << "\nCHECK PORT REUSE / NON-USE\n";

  testDuplicates_UnusedPort();
  testDuplicates_ReusedPort();
}





























int main(int argc, char **argv)
{
  std::cout << "\nBASIC ELEMENT PLUMBING\n";

  testCheckHookupElements();
  testCheckHookupRange();
  testCheckPushPull();
  testDuplicates();

  std::cout << "\nBASIC ELEMENT PLUMBING END\n";
  return 0;
}
  

/*
 * End of file 
 */
