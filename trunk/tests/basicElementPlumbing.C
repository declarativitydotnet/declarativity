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
#include "pullprint.h"
#include "memoryPull.h"
#include "router.h"
#include "master.h"

TupleRef create_tuple(int i) {
  TupleRef t = New refcounted< Tuple >;
  t->append(*New TupleField());
  t->append(*New TupleField((int32_t)i));
  t->append(*New TupleField((uint64_t)i));
  t->append(*New TupleField(i));
  strbuf myStringBuf;
  myStringBuf << "This is string '" << i << "'";
  str myString = myStringBuf;
  t->append(*New TupleField(myString));
  t->freeze();
  return t;
}

/** Test the portion of router initialization that checks for sanity of
    element hookups and lists */
void testCheckHookupElements(MasterRef master)
{
  std::cout << "CHECK ELEMENT HOOKUP\n";

  // Create the elements
  std::cout << "Creating elements\n";

  TupleRef t = create_tuple(1);
  ref< vec< TupleRef > > tupleRefBuffer =
    New refcounted< vec< TupleRef > >();
  tupleRefBuffer->push_back(t);
  ref< MemoryPull > memoryPull = New refcounted< MemoryPull >(tupleRefBuffer, 1);
  ref< PullPrint > pullPrint = New refcounted< PullPrint >();
  ElementSpecRef memoryPullSpec = New refcounted< ElementSpec >(memoryPull);
  ElementSpecRef pullPrintSpec = New refcounted< ElementSpec >(pullPrint);


  //////////////////////////////////////////////////////
  // Configuration that references non-existent elements
  ref< vec< ElementSpecRef > > elements = New refcounted< vec< ElementSpecRef > >();
  elements->push_back(pullPrintSpec);

  // Create the hookups
  Router::HookupRef hookup =
    New refcounted< Router::Hookup >(memoryPullSpec, 0,
                                     pullPrintSpec, 0);
  ref < vec< Router::HookupRef > > hookups =
    New refcounted< vec< Router::HookupRef > >();
  hookups->push_back(hookup);

  // Incorrectly referenced elements
  Router::ConfigurationRef configuration =
    New refcounted< Router::Configuration >(elements, hookups);

  RouterRef router = New refcounted< Router >(configuration, master);
  if (router->initialize() == 0) {
    std::cerr << "** Failed to catch hookup reference to unknown to element\n";
  } else {
    std::cout << "Caught hookup reference to unknown to element\n";
  }

  elements->clear();
  elements->push_back(memoryPullSpec);
  router = New refcounted< Router >(configuration, master);
  if (router->initialize() == 0) {
    std::cerr << "** Failed to catch hookup reference to unknown from element\n";
  } else {
    std::cout << "Caught hookup reference to unknown from element\n";
  }


  // With the correct configuration
  elements->clear();
  elements->push_back(memoryPullSpec);
  elements->push_back(pullPrintSpec);

  router = New refcounted< Router >(configuration, master);
  if (router->initialize() != 0) {
    std::cerr << "** Failed to initialize correct configuration\n";
  } else {
    std::cout << "Correctly accepted a correct configuration\n";
  }

  // With some bad port numbers
  hookup = New refcounted< Router::Hookup >(memoryPullSpec, -4,
                                            pullPrintSpec, 0);
  hookups->push_back(hookup);
  router = New refcounted< Router >(configuration, master);
  if (router->initialize() == 0) {
    std::cerr << "** Failed to catch negative from port\n";
  } else {
    std::cout << "Correctly caught negative from port\n";
  }

  hookup = New refcounted< Router::Hookup >(memoryPullSpec, 0,
                                            pullPrintSpec, -4);
  hookups->clear();
  hookups->push_back(hookup);

  router = New refcounted< Router >(configuration, master);
  if (router->initialize() == 0) {
    std::cerr << "** Failed to catch negative to port\n";
  } else {
    std::cout << "Correctly caught negative to port\n";
  }
}


/** Test the portion of router initialization that checks for port
    number range correctness. */
void testCheckHookupRange(MasterRef master)
{
  std::cout << "CHECK ELEMENT HOOKUP RANGE\n";

  TupleRef t = create_tuple(1);
  ref< vec< TupleRef > > tupleRefBuffer =
    New refcounted< vec< TupleRef > >();
  tupleRefBuffer->push_back(t);
  ref< MemoryPull > memoryPull = New refcounted< MemoryPull >(tupleRefBuffer, 1);
  ref< PullPrint > pullPrint = New refcounted< PullPrint >();
  ElementSpecRef memoryPullSpec = New refcounted< ElementSpec >(memoryPull);
  ElementSpecRef pullPrintSpec = New refcounted< ElementSpec >(pullPrint);

  ref< vec< ElementSpecRef > > elements = New refcounted< vec< ElementSpecRef > >();
  elements->push_back(memoryPullSpec);
  elements->push_back(pullPrintSpec);

  // Bad from port

  Router::HookupRef hookup =
    New refcounted< Router::Hookup >(memoryPullSpec, 1,
                                     pullPrintSpec, 0);
  ref < vec< Router::HookupRef > > hookups =
    New refcounted< vec< Router::HookupRef > >();
  hookups->push_back(hookup);

  Router::ConfigurationRef configuration =
    New refcounted< Router::Configuration >(elements, hookups);

  RouterRef router = New refcounted< Router >(configuration, master);
  if (router->initialize() == 0) {
    std::cerr << "** Failed to catch incorrect from port\n";
  } else {
    std::cout << "Caught incorrect from port\n";
  }

  hookups->clear();
  hookup = New refcounted< Router::Hookup >(memoryPullSpec, 0,
                                            pullPrintSpec, 1);
  hookups->push_back(hookup);
  router = New refcounted< Router >(configuration, master);
  if (router->initialize() == 0) {
    std::cerr << "** Failed to catch incorrect to port\n";
  } else {
    std::cout << "Correctly caught incorrect to port\n";
  }

  hookups->clear();
  hookup = New refcounted< Router::Hookup >(memoryPullSpec, 2,
                                            pullPrintSpec, 3);
  hookups->push_back(hookup);
  router = New refcounted< Router >(configuration, master);
  if (router->initialize() == 0) {
    std::cerr << "** Failed to catch incorrect from/to ports\n";
  } else {
    std::cout << "Correctly caught incorrect from/to ports\n";
  }

  hookups->clear();
  hookup = New refcounted< Router::Hookup >(pullPrintSpec, 0,
                                            memoryPullSpec, 0);
  hookups->push_back(hookup);
  router = New refcounted< Router >(configuration, master);
  if (router->initialize() == 0) {
    std::cerr << "** Failed to catch portless hookup\n";
  } else {
    std::cout << "Correctly caught portless hookup\n";
  }
}


int main(int argc, char **argv)
{
  std::cout << "BASIC ELEMENT PLUMBING\n";

  // Common master
  MasterRef master = New refcounted< Master >();

  testCheckHookupElements(master);
  testCheckHookupRange(master);

  return 0;
}
  

/*
 * End of file 
 */
