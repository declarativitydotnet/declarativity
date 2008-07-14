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
 * AUTHOR: 
 *
 * DESCRIPTION: A poor man's netcat (nc) for P2. It allows
 * sending/receiving simple text messages using P2. It also serves as a
 * simple example of a C++ application's binding with P2 via the C++
 * interface.
 *
 */

#include <string>
#include <iostream>
#include <reporting.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <p2.h>
#include <udp.h>
#include <val_str.h>



/** My Callback for message tuples */
void
messageCB(P2::DataflowHandle handle,
          TuplePtr tp)
{
  std::cout << "Got tuple: "
            << tp->toString()
            << "\n";

  // Construct a response tuple
  TuplePtr response = Tuple::mk();
  response->append(Val_Str::mk("localResponse"));
  response->append((*tp)[1]);
  response->append((*tp)[2]);
  response->append((*tp)[3]);
  response->append((*tp)[4]);
  response->freeze();

  int result = 
    P2::injectTuple(handle, response, &P2::nullCallback);
  std::cout << "Injected tuple "
            << response->toString()
            << " and got back "
            << result
            << "\n";
}


/** My Callback for response tuples */
void
responseCB(P2::DataflowHandle handle,
           TuplePtr tp)
{
  std::cout << "Got response tuple: "
            << tp->toString()
            << "\n";
}








/**
   My usage string
*/
static char* USAGE = "Usage:\n\t p2nc <options>\n"
                     "\t\t[-p <my port> (default: 10000)]\n"
                     "\t\t[-s <my host> (default: localhost)]\n"
                     "\t\t[-d <destination port> (default: 10000)]\n"
                     "\t\t[-n <destination host> (no default)]\n"
                     "\t\t[-m <message string> (default: \"message\", "
                     "empty means don't send)]\n"
                     "\t\t[-r <loggingLevel> (default: ERROR)]\n"
                     "\t\t[-h (gets usage help)]\n";




int
main(int argc, char** argv)
{
  std::string myHostname = "localhost";
  int myPort = 10000;
  std::string destHostname = "";
  int destPort = 10000;
  std::string message = "message";
  Reporting::setLevel(Reporting::P2_ERROR);


  // Parse command line options
  int c;
  while ((c = getopt(argc, argv, "p:s:d:n:m:r:h")) != -1) {
    switch (c) {
    case 'p':
      myPort = atoi(optarg);
      break;

    case 's':
      myHostname = optarg;
      break;

    case 'd':
      destPort = atoi(optarg);
      break;

    case 'n':
      destHostname = optarg;
      break;

    case 'm':
      message = optarg;
      break;

    case 'r':
      {
        // My minimum reporting level is optarg
        std::string levelName(optarg);
        Reporting::Level level =
          Reporting::levelFromName()[levelName];
        Reporting::setLevel(level);
      }
      break;

    case 'h':
    default:
      TELL_ERROR << USAGE;
      exit(-1);
    }
  }      

  
  // Construct a dataflow for a simple program that captures "message"
  // and generates "localMessage"
  std::string overlogProgram("a1 localMessage(@X, Message) :- \
                              message(@X, Message, E, Y). \
                              a2 response(@Y, Message, E, X) :- \
                              localResponse(@X, Message, E, Y). \
                              a3 incomingResponse(@X, Message, E, Y) :- \
                              response(@X, Message, E, Y).");
  std::ostringstream oss;
  oss << myHostname << ":" << myPort;
  std::string myAddress = oss.str();

  // Get a temporary file name for the derivatives
  char derivativeFileName[17] = "";
  int fd = -1;
  strncpy(derivativeFileName, "/tmp/p2nc.XXXXXX", sizeof derivativeFileName);
  if ((fd = mkstemp(derivativeFileName)) == -1) {
    TELL_ERROR << "Could not generate temporary filename '"
               << derivativeFileName
               << "' due to error "
               << strerror(errno)
               << "\n";
    exit(-1);
  }

  // Kill this particular file
  unlink(derivativeFileName);
  close(fd);
  std::string dfn(derivativeFileName);

  try {
    std::string dataflowName("p2nc Dataflow");
    P2::DataflowHandle df =
      P2::createDataflow(dataflowName,
                         myAddress,
                         myPort,
                         derivativeFileName,
                         overlogProgram,
                         false,
                         false,
                         false);
    
    // Now subscribe to message and remoteResponse tuples
    P2::subscribe(df,
                  "message",
                  boost::bind(&messageCB, df, _1));
    P2::subscribe(df,
                  "response",
                  boost::bind(&responseCB, df, _1));
    
    df.plumber()->toDot(std::string(derivativeFileName) + "-subscribed.dot");
    
    P2::run();
    
  } catch (Element::Exception e) {
    TELL_ERROR << "Caught an Element exception '"
               << e.toString()
               << "'\n";
    exit(-1);
  }
}
