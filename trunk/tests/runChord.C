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
 * DESCRIPTION: A chord dataflow.
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>
#include <stdlib.h>

#include "tuple.h"
#include "router.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_str.h"
#include "val_id.h"

#include "print.h"
#include "discard.h"
#include "pelTransform.h"
#include "duplicate.h"
#include "dupElim.h"
#include "filter.h"
#include "timedPullPush.h"
#include "udp.h"
#include "marshalField.h"
#include "unmarshalField.h"
#include "mux.h"
#include "roundRobin.h"
#include "demux.h"
#include "strToSockaddr.h"
#include "slot.h"
#include "timedPullSink.h"
#include "timestampSource.h"
#include "hexdump.h"
#include "table.h"
#include "lookup.h"
#include "aggregate.h"
#include "insert.h"
#include "scan.h"
#include "delete.h"
#include "pelScan.h"
#include "functorSource.h"
#include "tupleSource.h"
#include "queue.h"
#include "noNull.h"
#include "noNullField.h"






















































































































































































































































































































/** Created a networked chord flow. If alone, I'm my own successor.  If
    with landmark, I start with a join. */
void testNetworked(LoggerI::Level level,
                   str myAddress,
                   int port,    // extracted from myAddress for convenience
                   str landmarkAddress)
{
  // Create the data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  Udp udp(strbuf(myAddress) << ":Udp", port);

  createNode(myAddress, landmarkAddress,
             conf, &udp);

  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized network of chord lookup flows.\n";
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
  if (argc < 4) {
    fatal << "Usage:\n\t runChord <loggingLevel> <seed> <myipaddr:port> [<landmark_ipaddr:port>]\n";
  }

  str levelName(argv[1]);
  LoggerI::Level level = LoggerI::levelFromName[levelName];

  int seed = atoi(argv[2]);
  srand(seed);
  str myAddress(argv[3]);
  
  const char * theString = argv[3];
  char * theColon = strchr(theString, ':');
  if (theColon == NULL) {
    // Couldn't find the correct format
    fatal << "Usage:\n\trunChord <seed> <myipaddr:port> [<landmark_ipaddr:port>]\n\
              \tMy address is malformed\n";
  }
  str thePort(theColon + 1);
  int port = atoi(thePort);

  if (argc > 4) {
    str landmark(argv[4]);
    testNetworked(level,
                  myAddress,























































































































  testTimedPullToPull();




















































































































































































                  port,
                  landmark);
  } else {
    testNetworked(level,
                  myAddress,
                  port,
                  str("-"));
  }
  return 0;
}
  

/*
 * End of file 
 */
