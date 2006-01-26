
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <vector>

#include "tuple.h"
#include "groupby.h"
#include "print.h"
#include "timedPushSource.h"
#include "randomPushSource.h"
#include "pelTransform.h"
#include "timedPullSink.h"
#include "plumber.h"
#include "queue.h"


/** Test queue */

void agg()
{
    std::cout << "\n[Agg]\n";

    Plumber::ConfigurationPtr conf(new Plumber::Configuration());

    ElementSpecPtr randomPushSourceSpec = conf->addElement(ElementPtr(new RandomPushSource("randSource", 3, 0, 5)));
    ElementSpecPtr sourcePrintS = conf->addElement(ElementPtr(new Print("AfterSource")));
    
    std::vector<int> primaryFields; primaryFields.push_back(1); primaryFields.push_back(2);
    std::vector<int> groupByFields; groupByFields.push_back(1);
    std::vector<int> aggFields; aggFields.push_back(3);
    std::vector<int> aggTypes; aggTypes.push_back(GroupBy::MIN_AGG);

    ElementSpecPtr groupBySpec = conf->addElement(ElementPtr(new GroupBy("groupBy", "testAgg", primaryFields, 
									  groupByFields, aggFields, aggTypes, 
									  1, false))); 

    ElementSpecPtr queueSpec = conf->addElement(ElementPtr(new Queue("queue", 10)));
    ElementSpecPtr sinkPrintS = conf->addElement(ElementPtr(new Print("BeforeSink")));
    ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", 1)));

    conf->hookUp(randomPushSourceSpec, 0, sourcePrintS ,0);
    conf->hookUp(sourcePrintS, 0, groupBySpec, 0);
    conf->hookUp(groupBySpec, 0, queueSpec, 0);
    conf->hookUp(queueSpec, 0, sinkPrintS, 0);
    conf->hookUp(sinkPrintS, 0, sinkS, 0);   

    PlumberPtr plumber(new Plumber(conf));

    if (plumber->initialize(plumber) == 0) {
	std::cout << "Correctly initialized configuration.\n";
    } else {
	std::cout << "** Failed to initialize correct spec\n";
    }
    
    // Activate the plumber
    plumber->activate();    
}

int main(int argc, char **argv)
{
  std::cout << "\nTest Agg Start\n";

  eventLoopInitialize();
  agg();
  eventLoop();
  std::cout << "\nTest Agg End\n";
  return 0;
}
  

