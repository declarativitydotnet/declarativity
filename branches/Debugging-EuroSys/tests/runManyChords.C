/*
 * @(#)$Id$
 *
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Many chord dataflows
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <stdlib.h>

#include "tuple.h"
#include "plumber.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_str.h"
#include "val_id.h"

#include "print.h"
#include "printTime.h"
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


static const int SUCCESSORSIZE = 4;
#include "ring.C"
#include "chord.C"
#include "chordDatalog.C"







/** Create many chord dataflows joining via the same gateway. */
void testNetworked(LoggerI::Level level,
                   int nodes,
                   double interarrival)
{
  eventLoopInitialize();

  // Create the data flow
  Plumber::ConfigurationPtr conf(new Plumber::Configuration());




  // The first
  Udp* udp = new Udp("127.0.0.1:10000", 10000);
  createNode("127.0.0.1:10000", "-",
             conf, udp);

  int port = 10001;
  for (int i = 1;
       i < nodes;
       i++, port++) {
    ostringstream name;
    name << port;
    udp = new Udp(name.str(), port);

    ostringstream myAddress;
    myAddress <<  string("127.0.0.1:") << port;
    ostringstream landmarkAddress;
    landmarkAddress << string("127.0.0.1:") << ((port - 1));
    createNode(myAddress.str(), landmarkAddress.str(), conf, udp, i * interarrival);
  }

  PlumberPtr plumber(new Plumber(conf, level));
  if (plumber->initialize(plumber) == 0) {
    std::cout << "Correctly initialized network of chord lookup flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the plumber
  plumber->activate();

  // Run the plumber
  eventLoop();
}





int main(int argc, char **argv)
{
  if (argc != 5) {
    fatal << "Usage:\n\t runManyChord <loggingLevel> <seed> <noNodes> <interarrival>\n";
    exit(-1);
  }

  string levelName(argv[1]);
  LoggerI::Level level = LoggerI::levelFromName[levelName];

  int seed = atoi(argv[2]);
  srandom(seed);

  int noNodes = atoi(argv[3]);

  double interarrival = atof(argv[4]);
  testNetworked(level,
                noNodes,
                interarrival);
  return 0;
}
  

/*
 * End of file 
 */
