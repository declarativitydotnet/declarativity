
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
#include <vector>

#include "tuple.h"
#include "groupby.h"
#include "print.h"
#include "timedPushSource.h"
#include "randomPushSource.h"
#include "pelTransform.h"
#include "timedPullSink.h"
#include "router.h"
#include "queue.h"


/** Test queue */

void agg()
{
    std::cout << "\n[Agg]\n";

    Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

    ElementSpecRef randomPushSourceSpec = conf->addElement(new refcounted<RandomPushSource>("randSource", 3, 0, 5));
    ElementSpecRef sourcePrintS = conf->addElement(New refcounted< Print >("AfterSource"));
    
    std::vector<int> primaryFields; primaryFields.push_back(1); primaryFields.push_back(2);
    std::vector<int> groupByFields; groupByFields.push_back(1);
    std::vector<int> aggFields; aggFields.push_back(3);
    std::vector<int> aggTypes; aggTypes.push_back(GroupBy::MIN_AGG);

    ElementSpecRef groupBySpec = conf->addElement(New refcounted<GroupBy>("groupBy", "testAgg", primaryFields, 
									  groupByFields, aggFields, aggTypes, 
									  1, false)); 

    ElementSpecRef queueSpec = conf->addElement(New refcounted<Queue>("queue", 10));
    ElementSpecRef sinkPrintS = conf->addElement(New refcounted< Print >("BeforeSink"));
    ElementSpecRef sinkS = conf->addElement(New refcounted< TimedPullSink >("sink", 1));

    conf->hookUp(randomPushSourceSpec, 0, sourcePrintS ,0);
    conf->hookUp(sourcePrintS, 0, groupBySpec, 0);
    conf->hookUp(groupBySpec, 0, queueSpec, 0);
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
  std::cout << "\nTest Agg Start\n";

  agg();
  amain();
  std::cout << "\nTest Agg End\n";
  return 0;
}
  

