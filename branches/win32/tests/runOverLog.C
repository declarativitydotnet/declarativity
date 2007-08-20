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
 * DESCRIPTION: A runner of arbitrary OverLog code, using the SIGMOD
 * 2006 planner.  It has a pre-processing step.
 */

#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>

#include "p2.h"
#include <stdlib.h>
#include <sys/types.h>
//#include <sys/wait.h>
#include <boost/algorithm/string.hpp>

#include "tuple.h"
#include "plumber.h"

bool DEBUG = false;
bool CC = false;


/**
   My usage string
*/
static char* USAGE = "Usage:\n\t runOverLog\n"
                     "\t\t[-o <overLogFile> (default: standard input)]\n"
                     "\t\t[-r <loggingLevel> (default: ERROR)]\n"
                     "\t\t[-s <seed> (default: 0)]\n"
                     "\t\t[-n <myipaddr> (default: localhost)]\n"
                     "\t\t[-p <port> (default: 10000)]\n"
                     "\t\t[-g (produce a DOT graph)]\n"
                     "\t\t[-c (output canonical form)]\n"
                     "\t\t[-v (show stages of planning)]\n"
                     "\t\t[-x (dry run, don't start dataflow)]\n"
                     "\t\t[-D<key>=<value>]*\n"
                     "\t\t[-m (skip preprocessor)]\n"
                     "\t\t[-h (gets usage help)]\n";


int
main(int argc,
     char** argv)
{
  string overLogFile("-");
  string derivativeFile("stdin");
  int seed = 0;
  string myHostname = "localhost";
  int port = 10000;
  std::vector< std::string > definitions;
  bool outputDot = false;
  bool run = true;
  bool outputCanonicalForm = false;
  bool outputStages = false;
  bool preprocess = true;

  Reporting::setLevel(Reporting::P2_ERROR);
  // Parse command line options
  int c;
  while ((c = getopt(argc, argv, "o:r:s:n:p:D:hgcvxm")) != -1) {
    switch (c) {
    case 'o':
      overLogFile = optarg;
      if (overLogFile == "-") {
        derivativeFile = "stdin";
      } else {
        derivativeFile = overLogFile;
      }
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

    case 'g':
      outputDot = true;
      break;

    case 'c':
      outputCanonicalForm = true;
      break;

    case 'x':
      run = false;
      break;

    case 'm':
      preprocess = false;
      break;

    case 'v':
      outputStages = true;
      break;

    case 's':
      seed = atoi(optarg);
      break;

    case 'n':
      myHostname = optarg;
      break;

    case 'p':
      port = atoi(optarg);
      break;

    case 'D':
      definitions.push_back(std::string("-D") + optarg);
      break;

    case 'h':
    default:
      TELL_ERROR << USAGE;
      exit(-1);
    }
  }      

  if (overLogFile == "-") {
    derivativeFile = "stdin";
  } else {
    derivativeFile = overLogFile;
  }
  
  TELL_INFO << "Running from translated file \"" << overLogFile << "\"\n";

  srandom(seed);
  TELL_INFO << "Seed is \"" << seed << "\"\n";

  std::ostringstream myAddressBuf;
  myAddressBuf <<  myHostname << ":" << port;
  std::string myAddress = myAddressBuf.str();
  TELL_INFO << "My address is \"" << myAddress << "\"\n";
  
  TELL_INFO << "My environment is {\n";
  for (std::vector< std::string>::iterator i =
         definitions.begin();
       i != definitions.end();
       i++) {
    TELL_INFO << (*i) << "\n";
  }
  TELL_INFO << "}\n";

  if (!definitions.empty() && !preprocess) {
    TELL_WARN << "You are suppressed preprocessing (via -m) "
              << "but have also supplied extra macros (via -D). "
              << "All macro definitions will be ignored.\n";
  }

  // Preprocess and/or read in the program
  string program;
  if (preprocess) {
    program = P2::preprocessReadOverLogProgram(overLogFile,
                                               derivativeFile,
                                               definitions);
  } else {
    program = P2::readOverLogProgram(overLogFile);
  }
  
  P2::DataflowHandle dataflow =
    P2::createDataflow("runOverLog",
                       myAddress,
                       port,
                       derivativeFile,
                       program,
                       outputCanonicalForm,
                       outputStages,
                       outputDot);
  if (run) {
    try {
      P2::run();
    } catch (Element::Exception e) {
      TELL_ERROR << "Caught an Element exception '"
                 << e.toString()
                 << "'\n";
      exit(-1);
    }
  }

  return 0;
}

/**
 * End of File
 */
