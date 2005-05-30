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
#include "rtr_confgen.h"
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
#include "functorSource.h"
#include "store.h"

extern int ol_parser_debug;

typedef ref<Store> StoreRef;

void killJoin()
{
  exit(0);
}

static int LINKS = 2;
static int STARTING_PORT = 10000;
static const int nodes = 4;



/* Initial sending of links into the p2 dataflow */
void bookstrapData(Router::ConfigurationRef conf,
                   str name,
                   Store* linkTable, 
 		   ref< Udp > udp)
{
  // Scanner element over link table
  ElementSpecRef scanS = conf->addElement(linkTable->mkScan());

  ElementSpecRef scanPrintS =
    conf->addElement(New refcounted< Print >(strbuf("Scan:") << name));
  ElementSpecRef timedPullPushS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("PushReach:").cat(name), 1));
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("Slot:").cat(name)));
  
  ElementSpecRef encapS =
    conf->addElement(New refcounted< PelTransform >(strbuf("encap:").cat(name),
						    "$1 pop /* The From address */\
                                                     $0 ->t $1 append $2 append pop")); // the rest
  
  // Now marshall the payload (second field)
  ElementSpecRef marshalS =
    conf->addElement(New refcounted< MarshalField >(strbuf("Marshal:").cat(name),
                                                    1));
  ElementSpecRef routeS =
    conf->addElement(New refcounted< StrToSockaddr >(strbuf("Router:").cat(name), 0));
  ElementSpecRef udpTxS =
    conf->addElement(udp->get_tx());
  
  // Connect to outgoing hook
  conf->hookUp(scanS, 0, scanPrintS, 0);
  conf->hookUp(scanPrintS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, encapS, 0);
  conf->hookUp(encapS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, routeS, 0);
  conf->hookUp(routeS, 0, udpTxS, 0);
}


/** Build a symmetric link transitive closure. */
void testReachability(LoggerI::Level level, ref< OL_Context> ctxt, str filename)
{
  std::cout << "\nCHECK TRANSITIVE REACHABILITY\n";

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  Rtr_ConfGen routerConfigGenerator(ctxt, conf, true, true, filename);

  // Create one data flow per "node"
  ptr< Udp > udps[nodes];
  ptr< Udp > bootstrapUDP[nodes];
  ValuePtr nodeIds[nodes];
  str names[nodes];
  
   
  // Create the networking objects
  for (int i = 0; i < nodes; i++) {
    names[i] = strbuf("Node") << i;
    nodeIds[i] = Val_Str::mk(strbuf("127.0.0.1") << ":" << (STARTING_PORT + i));
    

    udps[i] = New refcounted< Udp >(names[i] << ":Udp", STARTING_PORT + i);
    bootstrapUDP[i] = New refcounted< Udp >(names[i] << ":bootstrapUdp", 
					    STARTING_PORT + i + 5000);
    routerConfigGenerator.createTables(names[i]);
  }


  // And make the data flows
  for (int i = 0; i < nodes; i++) {
    // initial loading of data into p2 dataflows
    Store* bootstrapTable = New Store("neighbor", 2);
        
    bookstrapData(conf, "bootstrap" << names[i], bootstrapTable, bootstrapUDP[i]);
    routerConfigGenerator.clear(); 
    routerConfigGenerator.configureRouter(udps[i], names[i]);
  
    // Create the link tables and links themselves.  Links must be
    // symmetric
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
      std::cout << "Node " << j << " insert tuple " << t->toString() << "\n";
      tableRef->insert(t);
      bootstrapTable->insert(t);

      // Make the symmetric one
      TupleRef symmetric = Tuple::mk();
      symmetric->append(Val_Str::mk("neighbor"));
      symmetric->append((ValueRef) nodeIds[other]);
      symmetric->append((ValueRef) nodeIds[i]);
      symmetric->freeze();
      tableRef = routerConfigGenerator.getTableByName(names[other], "neighbor");
      std::cout << "Node " << j << " insert tuple " << symmetric->toString() << "\n";
      tableRef->insert(symmetric);
      bootstrapTable->insert(symmetric);
    }
  }
  

  // add extra here to initialize the tables

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

  //StoreRef table = new refcounted< Table >("name", 100);

  LoggerI::Level level = LoggerI::ALL;
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
