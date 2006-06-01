// -*- c-basic-offset: 2; related-file-name: "" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: A chord dataflow.
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
#include "aggregate.h"
#include "insert.h"
#include "delete.h"
#include "functorSource.h"
#include "tupleSource.h"
#include "queue.h"
#include "noNull.h"
#include "noNullField.h"
#include "ol_lexer.h"
#include "ol_context.h"
#include "udp.h"


extern int ol_parser_debug;


static const int SUCCESSORSIZE = 4;
#include "chordDatalog.C"





void testNetworkedDatalog(LoggerI::Level level,
			  string myAddress,
			  int port,    // extracted from myAddress for convenience
			  string landmarkAddress, 
			  string filename,
			  double delay,
			  bool ifTrace)
{
  boost::shared_ptr< OL_Context > ctxt(new OL_Context());
  std::ifstream istr(filename.c_str());
  if (!istr.is_open()) {
    // Failed to open the file
    std::cerr << "Could not open file " << filename << "\n";
    exit(-1);
  }
  ctxt->parse_stream(&istr);
 
  if(ifTrace){
    string debug_rules = "doc/debugging-rules.olg";
    std::ifstream dstr(debug_rules.c_str());
    ctxt->parse_stream(&dstr);
  }
   
  startChordInDatalog(level, ctxt, filename, myAddress, port,
                      landmarkAddress, delay, ifTrace);
}





int main(int argc, char **argv)
{
  if (argc < 7) {
    fatal << "Usage:\n\t runChord <datalogFile> <loggingLevel> <seed> "
          << "<myipaddr:port> <startDelay> <TRACE|NO-TRACE> "
          << "[<landmark_ipaddr:port>]\n";
    exit(-1);
  }
  
  string datalogFile(argv[1]);
  if (datalogFile == "0") {
    fatal << "Must have a datalog file\n";
    exit(-1);
  } else {
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
    fatal << "Usage:\n\t runChord <datalogFile> <loggingLevel> <seed> "
          << "<myipaddr:port> <startDelay> <TRACE|NO-TRACE> "
          << "[<landmark_ipaddr:port>]\n"
          << "\tMy address is malformed\n";
    exit(-1);
  }
  string thePort(theColon + 1);
  int port = atoi(thePort.c_str());


  double delay = atof(argv[5]);

  string ifTrace(argv[6]);
  bool enableTracing = false;
  if(ifTrace == "TRACE")
    enableTracing = true;


  if (argc > 7) {
    string landmark(argv[7]);
    testNetworkedDatalog(level,
                         myAddress,
                         port,
                         landmark, 
                         datalogFile, 
                         delay,
                         enableTracing);
  } else {
    testNetworkedDatalog(level,
                         myAddress,
                         port,
                         string("-"),
                         datalogFile,
                         delay,
                         enableTracing);
  }
  return 0;
}
  

/*
 * End of file 
 */
