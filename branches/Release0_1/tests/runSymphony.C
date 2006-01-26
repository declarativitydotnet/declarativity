// -*- c-basic-offset: 2; related-file-name: "" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: A symphony dataflow.
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
#include "ol_lexer.h"
#include "ol_context.h"
#include "udp.h"


extern int ol_parser_debug;


static const int SUCCESSORSIZE = 1;
#include "ring.C"
#include "symphony.C"



/** Created a networked symphony flow. If alone, I'm my own successor.  If
    with landmark, I start with a join. */
void testNetworked(LoggerI::Level level,
                   string myAddress,
                   int port,    // extracted from myAddress for convenience
                   string landmarkAddress, int networkSize,
                   double delay = 0)
{
  eventLoopInitialize();

  // Create the data flow
  Plumber::ConfigurationPtr conf(new Plumber::Configuration());
  Udp udp(myAddress+":Udp", port);

  // FIX ME createSymNode(myAddress, landmarkAddress, conf, &udp, networkSize, delay);

  PlumberPtr plumber(new Plumber(conf, level));
  if (plumber->initialize(plumber) == 0) {
    std::cout << "Correctly initialized network of symphony lookup flows.\n";
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
  if (argc < 7) {
    fatal << "Usage:\n\t runSymphony <datalogFile> <loggingLevel> <seed> <myipaddr:port> <startDelay> <networkSize> [<landmark_ipaddr:port>]\n";
    exit(-1);
  }

  string datalogFile(argv[1]);
  bool runDatalogVersion = false;
  if (datalogFile == "0") {
      std::cout << "Manual translated symphony\n";
  } else {
      runDatalogVersion = true;
      std::cout << "Running from translated file " << datalogFile << "\n";
  }

  string levelName(argv[2]);
  LoggerI::Level level = LoggerI::levelFromName[levelName];

  int seed = atoi(argv[3]);
  srandom(seed);
  string myAddress(argv[4]);
  
  const char * theString = argv[4];
  std::cout << theString << "\n";
  char * theColon = strchr(theString, ':');
  if (theColon == NULL) {
    // Couldn't find the correct format
    fatal << "Usage:\n\trunSymphony <seed> <myipaddr:port> [<landmark_ipaddr:port>]\n\
              \tMy address is malformed\n";
    exit(-1);
  }
  string thePort(theColon + 1);
  int port = atoi(thePort.c_str());

  double delay = atof(argv[5]);
  int networkSize = atoi(argv[6]);

  if (runDatalogVersion) {
    if (argc > 7) {
      assert(false);
    } else {
      assert(false);
    }
    return 0;
  }

  if (argc > 7) {
    string landmark(argv[7]);
    testNetworked(level,
                  myAddress,
                  port,
                  landmark,
		  networkSize,
                  delay);
  } else {
    testNetworked(level,
                  myAddress,
                  port,
                  string("-"),
		  networkSize,
                  delay);
  }
  return 0;
}
  
/*
 * End of file 
 */
