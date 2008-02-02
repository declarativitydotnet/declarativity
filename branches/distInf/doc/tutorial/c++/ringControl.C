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
   My usage string
*/
static char* USAGE = "Usage:\n\t ringControl\n"
                     "\t\t[-n <my host> (default: localhost)]\n"
                     "\t\t[-p <my port> (default: 10000)]\n"
                     "\t\t[-c create|join (default: create)]\n"
                     "\t\t[-t <target node> (default: localhost:10001)]\n"
                     "\t\t[-l <landmark node> (default: localhost:10002)]\n"
                     "\t\t[-h (gets usage help)]\n";


int
main(int argc,
     char** argv)
{
  Reporting::setLevel(Reporting::ERROR);

  // Defaults
  string myHostname = "localhost";
  int port = 10000;
  string command("create");
  string target("localhost:10001");
  string landmark("localhost:10002");


  // Parse command line options
  int c;
  while ((c = getopt(argc, argv, "n:p:c:t:l:h")) != -1) {
    switch (c) {
    case 'n':
      myHostname = optarg;
      break;

    case 'p':
      port = atoi(optarg);
      break;

    case 'c':
      command = optarg;
      break;

    case 't':
      target = optarg;
      break;

    case 'l':
      landmark = optarg;
      break;

    case 'h':
    default:
      TELL_OUTPUT << USAGE;
      exit(-1);
    }
  }      

  string derivativeFile("/tmp/ringControl");
  std::ostringstream myAddressBuf;
  myAddressBuf <<  myHostname << ":" << port;
  std::string myAddress = myAddressBuf.str();
  string program("");
  P2::DataflowHandle dataflow =
    P2::createDataflow("sendCreate",
                       myAddress,
                       port,
                       derivativeFile,
                       program,
                       false,
                       false,
                       false);

  // What do I want to send in?
  TuplePtr tuple = Tuple::mk();
  if (command == "create") {
    tuple->append(Val_Str::mk("create")); // tuple name
    tuple->append(Val_Str::mk(target)); // the target node
    tuple->freeze();
  } else if (command == "join") {
    tuple->append(Val_Str::mk("join")); // tuple name
    tuple->append(Val_Str::mk(target)); // the target node
    tuple->append(Val_Str::mk(landmark)); // the node to join through
    tuple->freeze();
  }
  P2::injectTuple(dataflow, tuple, &P2::nullCallback);
  
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
