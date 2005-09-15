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
#include "scan.h"

#include "ruleStrand.h"
#include "planner.h"
#include "catalog.h"
#include "eca_context.h"


extern int ol_parser_debug;

//typedef ref<Store> StoreRef;

void killJoin()
{
  exit(0);
}

static int STARTING_PORT = 10000;
static const int NODES = 5;


void initializeLinkTable(ptr< Catalog >* catalog)
{
  for (int i = 0; i < NODES; i++) {
    TupleRef link = Tuple::mk();
    link->append(Val_Str::mk("link"));
    strbuf src("127.0.0.1:"); src << (STARTING_PORT + (i % NODES));   
    strbuf dst("127.0.0.1:"); dst << (STARTING_PORT + (i % NODES));   
    link->append(Val_Str::mk(strbuf() << src << "|" << dst)); // primary key
    link->append(Val_Str::mk(src));
    link->append(Val_Str::mk(dst));
    link->append(Val_Int32::mk(0));
    link->freeze();

    TableRef tableRef 
      = catalog[i]->getTableInfo("link")->_table; 
    tableRef->insert(link);
  }
  // directed chain
  for (int i = 0; i < NODES-1; i++) {
    TupleRef link = Tuple::mk();
    link->append(Val_Str::mk("link"));
    strbuf src("127.0.0.1:"); src << (STARTING_PORT + (i % NODES));   
    strbuf dst("127.0.0.1:"); dst << (STARTING_PORT + ((i + 1) % NODES));   
    link->append(Val_Str::mk(strbuf() << src << "|" << dst)); // primary key
    link->append(Val_Str::mk(src));
    link->append(Val_Str::mk(dst));
    link->append(Val_Int32::mk(1));
    link->freeze();

    TableRef tableRef 
      = catalog[i]->getTableInfo("link")->_table; 
    tableRef->insert(link);
  }
}


/** Build a symmetric link transitive closure. */
void runPathQuery(LoggerI::Level level, ref< OL_Context> ctxt, 
		  str filename, str inputGraph)
{
  warn<< "\nExecute path query " << filename << "\n";
  str outputFile(strbuf(filename << ".out"));
  FILE *plannerOutput = fopen(outputFile.cstr(), "w");

  str ecaFile(strbuf(filename << ".eca"));
  FILE *ecaOutput = fopen(ecaFile.cstr(), "w");

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  ptr< Udp > udps[NODES];
  ptr< Catalog > catalog[NODES];
  ptr< Planner > planner[NODES];
  ptr< ECA_Context > ectxt[NODES];
  
  str nodeID[NODES];
   
  for (int i = 0; i < NODES; i++) {
    nodeID[i] = strbuf("127.0.0.1:") << (STARTING_PORT + i);
    udps[i] = New refcounted< Udp >(strbuf("Udp:") << i, STARTING_PORT + i);

    catalog[i] = New refcounted< Catalog >();  
    catalog[i]->initTables(ctxt); 
  }

  for (int i = 0; i < NODES; i++) {
    ectxt[i] = New refcounted< ECA_Context >(); 
    ectxt[i]->eca_rewrite(ctxt, catalog[i]);
    if (i == 0) {
      fprintf(ecaOutput, "%s\n", ectxt[i]->toString().cstr());
    }
    planner[i] = New refcounted< Planner >(conf, catalog[i], false, nodeID[i]);  
    std::vector<RuleStrand*> ruleStrands = planner[i]->generateRuleStrands(ectxt[i]);

    for (unsigned k = 0; k < ruleStrands.size(); k++) {
      fprintf(plannerOutput, "%s", ruleStrands.at(k)->toString().cstr());
    }
    
    planner[i]->setupNetwork(udps[i]);
    planner[i]->registerAllRuleStrands(ruleStrands);
    fprintf(plannerOutput, "%s\n", planner[i]->getNetPlanner()->toString().cstr());
  }

  fclose(plannerOutput);
  fclose(ecaOutput);

  initializeLinkTable(catalog);

  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    warn << "Correctly initialized network of reachability flows.\n";
  } else {
    warn << "** Failed to initialize correct spec\n";
    return;
  }

  
  // Activate the router
  router->activate();

  // Run the router
  amain();
  
}



int main(int argc, char **argv)
{
  ref< OL_Context > ctxt = New refcounted< OL_Context>();

  if (argc < 5) {
    fatal << "Usage:\n\t paths <datalogFile> <inputgraph> <loggingLevel> <seed>\n";
  }

  str datalogFile(argv[1]);
  std::ifstream istr(datalogFile);
  ctxt->parse_stream(&istr);

  str inputGraph(argv[2]);
  
  str levelName(argv[3]);
  LoggerI::Level level = LoggerI::levelFromName[levelName];

  int seed = atoi(argv[4]);
  srandom(seed);

  std::cout << "Finish parsing (functors / tableInfos) " << ctxt->getRules()->size() 
	    << " " << ctxt->getTableInfos()->size() << "\n";

  runPathQuery(level, ctxt, datalogFile, inputGraph);

  return 0;
}
  

/*
 * End of file 
 */
