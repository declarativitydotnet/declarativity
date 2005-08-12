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
static char * USAGE = "Usage:\n\t runOverLog <overLogFile> <loggingLevel> <seed> <myipaddr:port> <startDelay> <{key=value;}>\n";



/**
 * Fill in the environment table
 */
void
initializeBaseTables(ref< OL_Context> ctxt,
                     ref<Rtr_ConfGen> routerConfigGenerator, 
                     str localAddress,
                     str environment)
{
  // Put in my own address
  TableRef envTable = routerConfigGenerator->getTableByName(localAddress, "env");
  TupleRef tuple = Tuple::mk();
  ValueRef envName = Val_Str::mk("env");
  tuple->append(envName);
  ValueRef myAddress = Val_Str::mk(strbuf() << localAddress);
  tuple->append(myAddress);
  tuple->append(Val_Str::mk("hostname"));
  tuple->append(myAddress);
  tuple->freeze();
  envTable->insert(tuple);

  // Now rest of the environment
  warn << "Environment is " << environment << "\n";
  const char * current = environment.cstr();
  while (strlen(current) > 0) {
    // Find a semicolon
    char * theSemi = strchr(current, ';');
    if (theSemi == NULL) {
      fatal << "Malformed environment string. No semicolon\n";
      exit(-1);
    }

    *theSemi = 0;

    // Find an =
    char * theEqual = strchr(current, '=');
    if (theEqual == NULL) {
      fatal << "Malformed environment entry [" << current << "]. No equals sign\n";
      exit (-1);
    }
    
    // Register the part from current to = and from = to semicolon
    *theEqual = 0;
    str attribute(current);
    *theEqual = '=';

    str value(theEqual + 1);
    warn << "[" << attribute << "=" << value << "]\n";

    TupleRef tuple = Tuple::mk();
    tuple->append(envName);
    tuple->append(myAddress);
    tuple->append(Val_Str::mk(attribute));
    tuple->append(Val_Str::mk(value));
    tuple->freeze();
    envTable->insert(tuple);

    // Move after semicolon
    *theSemi = ';';
    current = theSemi+1;
  }
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
                     double delay,
                     str environment)
{
  // create dataflow for translated OverLog
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ref< Rtr_ConfGen > routerConfigGenerator =
    New refcounted< Rtr_ConfGen >(ctxt, conf, false, DEBUG, CC, overLogFile);

  routerConfigGenerator->createTables(localAddress);

  ref< Udp > udp = New refcounted< Udp > (localAddress, port);
  routerConfigGenerator->clear();
  routerConfigGenerator->configureRouter(udp, localAddress);

  initializeBaseTables(ctxt, routerConfigGenerator, localAddress, environment);
   
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
                 double delay,
                 str environment)
{
  ref< OL_Context > ctxt = New refcounted< OL_Context>();
  std::ifstream istr(filename);
  if (!istr.is_open()) {
    // Failed to open the file
    std::cerr << "Could not open file " << filename << "\n";
    exit(-1);
  }
  ctxt->parse_stream(&istr);
  
  startOverLogDataflow(level, ctxt, filename, myAddress, port, delay, environment);
}






/**
 */
int
main(int argc, char **argv)
{
  if (argc < 7) {
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

  str environment(argv[6]);

  testOverLog(level,
              myAddress,
              port,
              overLogFile, 
              delay,
              environment);
  return 0;
}

/**
 * End of File
 */
