// -*- c-basic-offset: 2; related-file-name: "" -*-
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
#include "routerConfigGenerator.h"
#include "udp.h"


extern int ol_parser_debug;


#include "chord.C"
#include "chordDatalog.C"





/** Created a networked chord flow. If alone, I'm my own successor.  If
    with landmark, I start with a join. */
void testNetworked(LoggerI::Level level,
                   str myAddress,
                   int port,    // extracted from myAddress for convenience
                   str landmarkAddress,
                   double delay = 0)
{
  // Create the data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  Udp udp(strbuf(myAddress) << ":Udp", port);

  createNode(myAddress, landmarkAddress,
             conf, &udp, delay);

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


void testNetworkedDatalog(LoggerI::Level level,
			  str myAddress,
			  int port,    // extracted from myAddress for convenience
			  str landmarkAddress, str filename,
			  double delay)
{
  ref< OL_Context > ctxt = New refcounted< OL_Context>();
  std::ifstream istr(filename);
  ctxt->parse_stream(&istr);
   
  startChordInDatalog(level, ctxt, filename, myAddress, landmarkAddress, port, delay);
}





int main(int argc, char **argv)
{
  if (argc < 6) {
    fatal << "Usage:\n\t runChord <datalogFile> <loggingLevel> <seed> <myipaddr:port> <startDelay> [<landmark_ipaddr:port>]\n";
  }

  str datalogFile(argv[1]);
  bool runDatalogVersion = false;
  if (datalogFile == "0") {
      std::cout << "Manual translated chord\n";
  } else {
      runDatalogVersion = true;
      std::cout << "Running from translated file " << datalogFile << "\n";
  }

  str levelName(argv[2]);
  LoggerI::Level level = LoggerI::levelFromName[levelName];

  int seed = atoi(argv[3]);
  srandom(seed);
  str myAddress(argv[4]);
  
  const char * theString = argv[4];
  std::cout << theString << "\n";
  char * theColon = strchr(theString, ':');
  if (theColon == NULL) {
    // Couldn't find the correct format
    fatal << "Usage:\n\trunChord <seed> <myipaddr:port> [<landmark_ipaddr:port>]\n\
              \tMy address is malformed\n";
  }
  str thePort(theColon + 1);
  int port = atoi(thePort);

  double delay = atof(argv[5]);


  if (runDatalogVersion) {
    if (argc > 6) {
      str landmark(argv[6]);
      testNetworkedDatalog(level,
			   myAddress,
			   port,
			   landmark, 
			   datalogFile, 
			   delay);
    } else {
      testNetworkedDatalog(level,
			   myAddress,
			   port,
			   str("0"),
			   datalogFile,
			   delay);
    }
    return 0;
  }

  if (argc > 6) {
    str landmark(argv[6]);
    testNetworked(level,
                  myAddress,
                  port,
                  landmark,
                  delay);
  } else {
    testNetworked(level,
                  myAddress,
                  port,
                  str("-"),
                  delay);
  }
  return 0;
}
  

/*
 * End of file 
 */
