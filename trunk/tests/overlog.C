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
 * DESCRIPTION: Test harness for datalog....
 *
 */

#include <async.h>
#include <arpc.h>
#include <iostream>
#include <fstream>

#include "ol_lexer.h"
#include "ol_context.h"
#include "routerConfigGenerator.h"
#include "udp.h"

extern int ol_parser_debug;

int main(int argc, char **argv)
{
  std::cout << "OVERLOG\n";

  
  ref< OL_Context > ctxt = New refcounted< OL_Context>();

  strbuf filename;

  for( int i=1; i<argc; i++) { 
    str arg(argv[i]);
    
    if (arg == "-d") { 
      ol_parser_debug = 1;
    } else if (arg == "-h") { 
      std::cerr << "Usage: " << argv[0] << "{option|filename}+\n"
		<< "\t-d: turn on parser debugging\n"
		<< "\t-h: print this help text\n"
		<< "\t- : read from stdin\n";
      exit(0);
    } else if (arg == "-") {
      ctxt->parse_stream(&std::cin);
    } else {      
      filename << argv[i];
      str file(filename);
      std::ifstream istr(file);
      ctxt->parse_stream(&istr);
    }
  }

  std::cout << "Finish parsing (functors / tableInfos) " << ctxt->getFunctors()->size() 
	    << " " << ctxt->getTableInfos()->size() << "\n";

  
  // test a configuration of a router
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  RouterConfigGenerator routerConfigGenerator(ctxt, conf, true, false, filename);
  routerConfigGenerator.createTables("localhost");

  ref< Udp > udp = New refcounted< Udp >("Udp", 10000);
  routerConfigGenerator.configureRouter(udp, "localhost");

  LoggerI::Level level = LoggerI::NONE;
  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized network of reachability flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }
  

  return 0;
}


