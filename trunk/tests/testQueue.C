
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
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>

#include "tuple.h"
#include "queue.h"
#include "print.h"
#include "timedPushSource.h"
#include "timedPullSink.h"
#include "router.h"
#include "slot.h"
/*#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"
*/

/** Test queue */

void testQueue()
{
    std::cout << "\n[Test Queue]\n";

    Router::ConfigurationPtr conf(new Router::Configuration());

    ElementSpecPtr timedPushSourceSpec = conf->addElement(ElementPtr(new TimedPushSource("source", 1)));
    ElementSpecPtr sourcePrintS = conf->addElement(ElementPtr(new Print("AfterSource")));
    ElementSpecPtr queueSpec = conf->addElement(ElementPtr(new Queue("queue", 5)));
    ElementSpecPtr sinkPrintS = conf->addElement(ElementPtr(new Print("BeforeSink")));
    ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", 2)));

    conf->hookUp(timedPushSourceSpec, 0, sourcePrintS ,0);
    conf->hookUp(sourcePrintS, 0, queueSpec, 0);
    conf->hookUp(queueSpec, 0, sinkPrintS, 0);
    conf->hookUp(sinkPrintS, 0, sinkS, 0);
   
    RouterPtr router(new Router(conf));

    if (router->initialize(router) == 0) {
	std::cout << "Correctly initialized configuration.\n";
    } else {
	std::cout << "** Failed to initialize correct spec\n";
    }
    
    // Activate the router
    router->activate();    
}

int main(int argc, char **argv)
{
  std::cout << "\nTest Queue Start\n";

  testQueue();
  amain();
  std::cout << "\nTest Queue End\n";
  return 0;
}
  

