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
 * DESCRIPTION: A symmetric reachability data flow
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
#include "val_str.h"

#include "ol_lexer.h"
#include "ol_context.h"
#include "routerConfigGenerator.h"
#include "print.h"
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
#include "strToSockaddr.h"
#include "slot.h"
#include "timedPullSink.h"
#include "timestampSource.h"
#include "hexdump.h"
#include "table.h"
#include "lookup.h"
#include "insert.h"
#include "scan.h"

extern int ol_parser_debug;


void killJoin()
{
  exit(0);
}

static int LINKS = 2;
static int STARTING_PORT = 10000;
static const int nodes = 5;

/** Build a symmetric link transitive closure. */
void testReachability(LoggerI::Level level, ref< OL_Context> ctxt, str filename)
{
  std::cout << "\nCHECK TRANSITIVE REACHABILITY\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  RouterConfigGenerator routerConfigGenerator(ctxt, conf, true, true, filename);

  // Create one data flow per "node"
  ptr< Udp > udps[nodes];
  ValuePtr nodeIds[nodes];
  str names[nodes];
  
  // Create the networking objects
  for (int i = 0; i < nodes; i++) {
    names[i] = strbuf("Node") << i;
    nodeIds[i] = Val_Str::mk(strbuf("127.0.0.1") << ":" << (STARTING_PORT + i));

    udps[i] = New refcounted< Udp >(names[i] << ":Udp", STARTING_PORT + i);

    routerConfigGenerator.createTables(names[i]);
  }

  
  // Create the link tables and links themselves.  Links must be
  // symmetric
  for (int i = 0; i < nodes; i++) {
    std::set< int > others;
    others.insert(i);           // me

    for (int j = 0; j < LINKS; j++) {
      TupleRef t = Tuple::mk();
      t->append(Val_Str::mk("neighbor"));
      // From me
      t->append((ValueRef) nodeIds[i]);
      // To someone at random, but not me. Replacement is discouraged

      int other = i;
      while (others.find(other) != others.end()) {
        other = (int) (1.0 * nodes * rand() / (RAND_MAX + 1.0));
      }
      // Got one
      others.insert(other);

      t->append((ValueRef) nodeIds[other]);
      t->freeze();
      TableRef tableRef = routerConfigGenerator.getTableByName(names[i], "neighbor");
      tableRef->insert(t);

      // Make the symmetric one
      TupleRef symmetric = Tuple::mk();
      symmetric->append(Val_Str::mk("neighbor"));
      symmetric->append((ValueRef) nodeIds[other]);
      symmetric->append((ValueRef) nodeIds[i]);
      symmetric->freeze();
      tableRef = routerConfigGenerator.getTableByName(names[other], "neighbor");
      tableRef->insert(symmetric);
    }
  }

  // And make the data flows
  for (int i = 0; i < nodes; i++) {
      routerConfigGenerator.configureRouter(udps[i], names[i]);
  }

  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized network of reachability flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}



int main(int argc, char **argv)
{
  std::cout << "\nGRAPH REACHABILITY\n";
  srand(0);

  TableRef table = new refcounted< Table >("name", 100);

  LoggerI::Level level = LoggerI::NONE;
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

  testReachability(level, ctxt, filename);

  return 0;
}
  

/*
 * End of file 
 */
