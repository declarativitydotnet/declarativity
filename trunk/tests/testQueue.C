
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
#include <async.h>
#include <arpc.h>
#include <iostream>

#include "tuple.h"
#include "queue.h"
#include "print.h"
#include "timedPushSource.h"
#include "timedPullSink.h"
#include "router.h"
#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"

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


/** Test queue */

void testQueue()
{
    std::cout << "\n[Test Queue]\n";

    Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

    ElementSpecRef timedPushSourceSpec = conf->addElement(new refcounted<TimedPushSource>(1));
    ElementSpecRef sourcePrintS = conf->addElement(New refcounted< Print >("AfterSource"));
    ElementSpecRef queueSpec = conf->addElement(New refcounted<Queue>(5));
    ElementSpecRef sinkPrintS = conf->addElement(New refcounted< Print >("BeforeSink"));
    ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >(3));

    conf->hookUp(timedPushSourceSpec, 0, sourcePrintS ,0);
    conf->hookUp(sourcePrintS, 0, queueSpec, 0);
    conf->hookUp(queueSpec, 0, sinkPrintS, 0);
    conf->hookUp(sinkPrintS, 0, sinkS, 0);
   
    RouterRef router = New refcounted< Router >(conf);

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
  

