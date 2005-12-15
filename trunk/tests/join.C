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
#include "dupElim.h"
#include "filter.h"
#include "timedPullSink.h"
#include "lookup.h"
#include "insert.h"
#include "table.h"

void killJoin()
{
  exit(0);
}

/** Static join test.  Create random tuples, split them into two
    transformation flows, each dropping different tuples, and
    constructing the different "table" streams.  Then join them again at
    the join element producing the result. */
void testSimpleJoin(LoggerI::Level level)
{
  std::cout << "\nCHECK SIMPLE JOIN\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  // The source dataflow.  Produce random tuples.
  ElementSpecRef sourceS =
    conf->addElement(New refcounted< TimedPushSource >("source", 1));
  ElementSpecRef sourcePrintS = conf->addElement(New refcounted< Print >("AfterSource"));
  ElementSpecRef dupS =
    conf->addElement(New refcounted< Duplicate >("sourceDup", 2));
  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, dupS, 0);


  // The rehash factories
  TableRef tableA = New refcounted< Table >("ATuples", 1000);
  tableA->add_unique_index(1);



  // The A transformation dataflow.  Drop tuples with seconds ending in
  // 7.  Prepend the table name "A"
  ElementSpecRef transAS =
    conf->addElement(New refcounted< PelTransform >("sourceTransform",
                                                    "\"A\" pop /* This is an A tuple */ \
                                                     $1 5 % dup 2 ==i not pop /* This determines keep or drop */ \
                                                     pop /* This pops the join key */ \
						     $2 1000 /i 3 % pop /* And a non-join key */ "));
  ElementSpecRef filterAS =
    conf->addElement(New refcounted< Filter >("filterA", 1));
  ElementSpecRef filterDropAS =
    conf->addElement(New refcounted< PelTransform >("filterDropA", "$0 pop $2 pop $3 pop"));
  ElementSpecRef dupElimAS = conf->addElement(New refcounted< DupElim >("DupElimA"));
  ElementSpecRef transAPrintS =
    conf->addElement(New refcounted< Print >("ATuples"));
  ElementSpecRef rehashAS = conf->addElement(New refcounted< Insert >("InsertA", tableA));
  ElementSpecRef sinkAS =
    conf->addElement(New refcounted< Discard >("discardA"));
  conf->hookUp(dupS, 0, transAS, 0);
  conf->hookUp(transAS, 0, filterAS, 0);
  conf->hookUp(filterAS, 0, filterDropAS, 0);
  conf->hookUp(filterDropAS, 0, dupElimAS, 0);
  conf->hookUp(dupElimAS, 0, transAPrintS, 0);
  conf->hookUp(transAPrintS, 0, rehashAS, 0);
  conf->hookUp(rehashAS, 0, sinkAS, 0);


  // The B transformation dataflow.  Drop tuples with seconds ending in
  // 3.  Prepend the table name "B"
  ElementSpecRef transBS =
    conf->addElement(New refcounted< PelTransform >("transB", 
                                                    "\"B\" pop /* This is a B tuple */ \
                                                     $1 5 % dup 3 ==i not pop /* Keep or drop? */ \
                                                     pop /* Result */ \
                                                     $2 1000 /i 3 % ->str \"z\" strcat pop /* non-join key */"));
  ElementSpecRef filterBS =
    conf->addElement(New refcounted< Filter >("filterB", 1));
  ElementSpecRef filterDropBS =
    conf->addElement(New refcounted< PelTransform >("filterDropB", "$0 pop $2 pop $3 pop"));
  ElementSpecRef dupElimBS = conf->addElement(New refcounted< DupElim >("dupElimB"));
  ElementSpecRef transBPrintS = conf->addElement(New refcounted< Print >("BTuples"));
  ElementSpecRef lookupBS =
    conf->addElement(New refcounted< UniqueLookup >("Lookup", tableA, 1, 1));
  ElementSpecRef lookupBPrintS =
    conf->addElement(New refcounted< Print >("LookupBInA"));
  conf->hookUp(dupS, 1, transBS, 0);
  conf->hookUp(transBS, 0, filterBS, 0);
  conf->hookUp(filterBS, 0, filterDropBS, 0);
  conf->hookUp(filterDropBS, 0, dupElimBS, 0);
  conf->hookUp(dupElimBS, 0, transBPrintS, 0);
  conf->hookUp(transBPrintS, 0, lookupBS, 0);
  conf->hookUp(lookupBS, 0, lookupBPrintS, 0);






  // The joining dataflow
  ElementSpecRef joinerBPrintS = conf->addElement(New refcounted< Print >("BJoinWithA"));
  ElementSpecRef sinkBS =
    conf->addElement(New refcounted< TimedPullSink >("sinkB", 0));
  conf->hookUp(lookupBPrintS, 0, joinerBPrintS, 0);
  conf->hookUp(joinerBPrintS, 0, sinkBS, 0);




  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized simple join.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the router
  router->activate();

  // Schedule kill
  //delayCB(10, wrap(&killJoin));

  // Run the router
  amain();
}

int main(int argc, char **argv)
{
  std::cout << "\nJOIN\n";

  LoggerI::Level level = LoggerI::ALL;
  if (argc > 1) {
    str levelName(argv[1]);
    level = LoggerI::levelFromName[levelName];
  }

  testSimpleJoin(level);

  return 0;
}
  

/*
 * End of file 
 */
