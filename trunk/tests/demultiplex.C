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
 * DESCRIPTION: Tests for static and live demultiplexing
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
#include "demux.h"

/** Static demux based on first element.  Two dataflows are registered,
    one per demux value (a string).  Then a source dataflow produces
    random sequences of packets switching between demux values. */
void testStaticDemux()
{
  std::cout << "\nCHECK STATIC DEMUX\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  // The source dataflow
  ElementSpecRef sourceS = conf->addElement(New refcounted< TimedPushSource >(1));
  ElementSpecRef sourcePrintS = conf->addElement(New refcounted< Print >("AfterSource"));
  ElementSpecRef transS =       // If the seconds end in 0, produce a
                                // tuple.  Prefix the tuple with a 0 or
                                // 1 based on whether the number of
                                // seconds was odd or even
    conf->addElement(New refcounted< PelTransform >("$1 10 % dup $1 2 % ->i32 ifpop ifpoptuple"));
  ElementSpecRef prefixedPrintS = conf->addElement(New refcounted< Print >("Prefixed"));
  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, transS, 0);
  conf->hookUp(transS, 0, prefixedPrintS, 0);


  // The even destination dataflow
  ElementSpecRef sinkPrintEvenS = conf->addElement(New refcounted< Print >("EvenSink"));
  ElementSpecRef sinkEvenS = conf->addElement(New refcounted< Discard >());
  conf->hookUp(sinkPrintEvenS, 0, sinkEvenS, 0);

  // The odd destination dataflow
  ElementSpecRef sinkPrintOddS = conf->addElement(New refcounted< Print >("OddSink"));
  ElementSpecRef sinkOddS = conf->addElement(New refcounted< Discard >());
  conf->hookUp(sinkPrintOddS, 0, sinkOddS, 0);

  // The demultiplexer
  ref< vec< ValueRef > > demuxKeys = New refcounted< vec< ValueRef > >;
  demuxKeys->push_back(New refcounted< Val_Int32 >(0));
  demuxKeys->push_back(New refcounted< Val_Int32 >(1));
  ElementSpecRef demuxS = conf->addElement(New refcounted< Demux >(demuxKeys));
  conf->hookUp(prefixedPrintS, 0, demuxS, 0);
  conf->hookUp(demuxS, 0, sinkPrintEvenS, 0);
  conf->hookUp(demuxS, 1, sinkPrintOddS, 0);
  
  
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized static demux.\n";
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
  std::cout << "\nSCHEDULING\n";

  testStaticDemux();

  return 0;
}
  

/*
 * End of file 
 */
