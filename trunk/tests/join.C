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
 * DESCRIPTION: Tests for joins
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
#include "val_int32.h"

#include "print.h"
#include "timedPushSource.h"
#include "discard.h"
#include "pelTransform.h"
#include "duplicate.h"

/** Static join test.  Create random tuples, split them into two
    transformation flows, each dropping different tuples, and
    constructing the different "table" streams.  Then join them again at
    the join element producing the result. */
void testSimpleJoin()
{
  std::cout << "\nCHECK SIMPLE JOIN\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  // The source dataflow.  Produce random tuples.
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(1));
  ElementSpecRef sourcePrintS = conf->addElement(New refcounted< Print >("AfterSource"));
  ElementSpecRef dupS = conf->addElement(New refcounted< Duplicate >(2));
  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, dupS, 0);


  // The A transformation dataflow.  Drop tuples with seconds ending in
  // 7.  Prepend the table name "A"
  ElementSpecRef transAS =
    conf->addElement(New refcounted< PelTransform >("\"A\" pop \
                                                     $1 10 % 7 ==i not ifpoptuple"));
  ElementSpecRef transAPrintS = conf->addElement(New refcounted< Print >("ATuples"));
  ElementSpecRef sinkAS = conf->addElement(New refcounted< Discard >());
  conf->hookUp(dupS, 0, transAS, 0);
  conf->hookUp(transAS, 0, transAPrintS, 0);
  conf->hookUp(transAPrintS, 0, sinkAS, 0);

  // The B transformation dataflow.  Drop tuples with seconds ending in
  // 3.  Prepend the table name "B"
  ElementSpecRef transBS =
    conf->addElement(New refcounted< PelTransform >("\"B\" pop \
                                                     $1 10 % 3 ==i not ifpoptuple"));
  ElementSpecRef transBPrintS = conf->addElement(New refcounted< Print >("BTuples"));
  ElementSpecRef sinkBS = conf->addElement(New refcounted< Discard >());
  conf->hookUp(dupS, 1, transBS, 0);
  conf->hookUp(transBS, 0, transBPrintS, 0);
  conf->hookUp(transBPrintS, 0, sinkBS, 0);

  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized simple join.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}

int main(int argc, char **argv)
{
  std::cout << "\nJOIN\n";

  testSimpleJoin();

  return 0;
}
  

/*
 * End of file 
 */
