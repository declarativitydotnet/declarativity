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
 * DESCRIPTION: A runner of arbitrary OverLog code.  It only
 * fills in the env table with the local host.
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

#include "ol_lexer.h"
#include "ol_context.h"
#include "rtr_confgen.h"



bool DEBUG = false;
bool CC = false;


/**
   My usage string
*/
static char * USAGE = "Usage:\n\t runOverLog <overLogFile> <loggingLevel> <seed> <myipaddr:port> <startDelay>\n";



/**
 * Fill in the environment table
 */
void
initializeBaseTables(ref< OL_Context> ctxt,
                     ref<Rtr_ConfGen> routerConfigGenerator, 
                     str localAddress)
{
  // Put in my own address
  TableRef nodeTable = routerConfigGenerator->getTableByName(localAddress, "env");
  TupleRef tuple = Tuple::mk();
  tuple->append(Val_Str::mk("env"));
  str myAddress = str(strbuf() << localAddress);
  tuple->append(Val_Str::mk(myAddress));
  tuple->freeze();
  nodeTable->insert(tuple);
}



/**
   Put together the dataflow.
*/
void
startOverLogDataflow(LoggerI::Level level,
                     ref< OL_Context> ctxt,
                     str overLogFile, 
                     str localAddress,
                     int port,
                     double delay)
{
  // create dataflow for translated OverLog
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ref< Rtr_ConfGen > routerConfigGenerator =
    New refcounted< Rtr_ConfGen >(ctxt, conf, false, DEBUG, CC, overLogFile);

  routerConfigGenerator->createTables(localAddress);

  ref< Udp > udp = New refcounted< Udp > (localAddress, port);
  routerConfigGenerator->clear();
  routerConfigGenerator->configureRouter(udp, localAddress);

  initializeBaseTables(ctxt, routerConfigGenerator, localAddress);
   
  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    warn << "Correctly initialized network of chord lookup flows.\n";
  } else {
    warn << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}


/**
 * Builds a dataflow graph from an OverLog specification
 */
void testOverLog(LoggerI::Level level,
                 str myAddress,
                 int port,    // extracted from myAddress for convenience
                 str filename,
                 double delay)
{
  ref< OL_Context > ctxt = New refcounted< OL_Context>();
  std::ifstream istr(filename);
  ctxt->parse_stream(&istr);
  
  startOverLogDataflow(level, ctxt, filename, myAddress, port, delay);
}






/**
 */
int
main(int argc, char **argv)
{
  if (argc < 6) {
    fatal << USAGE;
  }

  str overLogFile(argv[1]);
  std::cout << "Running from translated file " << overLogFile << "\n";

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
    std::cerr << "Incorrect own address format\n";
    fatal << USAGE;
  }
  str thePort(theColon + 1);
  int port = atoi(thePort);

  // How long should I wait before I begin?
  double delay = atof(argv[5]);

  testOverLog(level,
              myAddress,
              port,
              overLogFile, 
              delay);
  return 0;
}

/**
 * End of File
 */
