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


  //////////////////////////////////////////////////////
  // Configuration that references non-existent elements
  ref< vec< ElementRef > > elements = New refcounted< vec< ElementRef > >();
  elements->push_back(pullPrint);

  // Create the hookups
  Router::HookupRef hookup =
    New refcounted< Router::Hookup >(memoryPull, 0,
                                     pullPrint, 0);
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
  elements->push_back(memoryPull);
  router = New refcounted< Router >(configuration, master);
  if (router->initialize() == 0) {
    std::cerr << "** Failed to catch hookup reference to unknown from element\n";
  } else {
    std::cout << "Caught hookup reference to unknown from element\n";
  }


  // With the correct configuration
  elements->clear();
  elements->push_back(memoryPull);
  elements->push_back(pullPrint);

  router = New refcounted< Router >(configuration, master);
  if (router->initialize() != 0) {
    std::cerr << "** Failed to initialize correct configuration\n";
  } else {
    std::cout << "Correctly accepted a correct configuration\n";
  }

  // With some bad port numbers
  hookup = New refcounted< Router::Hookup >(memoryPull, -4,
                                            pullPrint, 0);
  hookups->push_back(hookup);
  router = New refcounted< Router >(configuration, master);
  if (router->initialize() == 0) {
    std::cerr << "** Failed to catch negative from port\n";
  } else {
    std::cout << "Correctly caught negative from port\n";
  }

  hookup = New refcounted< Router::Hookup >(memoryPull, 0,
                                            pullPrint, -4);
  hookups->clear();
  hookups->push_back(hookup);

  router = New refcounted< Router >(configuration, master);
  if (router->initialize() == 0) {
    std::cerr << "** Failed to catch negative to port\n";
  } else {
    std::cout << "Correctly caught negative to port\n";
  }
}


int main(int argc, char **argv)
{
  std::cout << "BASIC ELEMENT PLUMBING\n";

  // Common master
  MasterRef master = New refcounted< Master >();

  testCheckHookupElements(master);

  return 0;
}
  

/*
 * End of file 
 */
