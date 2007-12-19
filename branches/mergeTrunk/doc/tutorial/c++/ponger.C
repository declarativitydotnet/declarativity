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
 * DESCRIPTION: An example of a non-P2 program using P2. This program,
 * depending on input options, generates control messages for ring
 * nodes. It either generates a create(@TargetNodeAddress) message or a
 * join(@TargetNodeAddress, LandmarkNodeAddress) tuple.
 */

#include <iostream>

#include "p2.h"

#include "tuple.h"
#include "plumber.h"
#include "val_str.h"


/**
   My responder function. Just reverse the tuples and send them out
   again.
*/
void
pingHandler(P2::DataflowHandle dataflow,
            TuplePtr incoming)
{
  TuplePtr outgoing = Tuple::mk();
  outgoing->append(Val_Str::mk("pong")); // tuple name
  outgoing->append((*incoming)[2]); // the sender
  outgoing->append((*incoming)[1]); // me
  outgoing->freeze();

  P2::injectTuple(dataflow, outgoing, &P2::nullCallback);
}


/**
   My usage string
*/
static char* USAGE = "Usage:\n\t ponger\n"
                     "\t\t[-n <my host> (default: localhost)]\n"
                     "\t\t[-p <my port> (default: 10000)]\n"
                     "\t\t[-h (gets usage help)]\n";


int
main(int argc,
     char** argv)
{
  Reporting::setLevel(Reporting::ERROR);

  // Defaults
  string myHostname = "localhost";
  int port = 10000;


  // Parse command line options
  int c;
  while ((c = getopt(argc, argv, "n:p:h")) != -1) {
    switch (c) {
    case 'n':
      myHostname = optarg;
      break;

    case 'p':
      port = atoi(optarg);
      break;

    case 'h':
    default:
      TELL_OUTPUT << USAGE;
      exit(-1);
    }
  }      

  string derivativeFile("/tmp/ponger");
  std::ostringstream myAddressBuf;
  myAddressBuf <<  myHostname << ":" << port;
  std::string myAddress = myAddressBuf.str();
  string program("");
  P2::DataflowHandle dataflow =
    P2::createDataflow("ponger",
                       myAddress,
                       port,
                       derivativeFile,
                       program,
                       false,
                       false,
                       false);


  // Subscribe to ping tuples
  P2::subscribe(dataflow,
                "ping",
                boost::bind(&pingHandler, dataflow, _1));

  try {
    P2::run();
  } catch (Element::Exception e) {
    TELL_ERROR << "Caught an Element exception '"
               << e.toString()
               << "'\n";
    exit(-1);
  }

  return 0;
}

/**
 * End of File
 */
