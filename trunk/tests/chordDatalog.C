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
 * DESCRIPTION: A chord dataflow.
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
#include "val_uint32.h"
#include "val_str.h"
#include "val_id.h"

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
#include "roundRobin.h"
#include "demux.h"
#include "strToSockaddr.h"
#include "slot.h"
#include "timedPullSink.h"
#include "timestampSource.h"
#include "hexdump.h"
#include "table.h"
#include "lookup.h"
#include "aggregate.h"
#include "insert.h"
#include "scan.h"
#include "delete.h"
#include "pelScan.h"
#include "functorSource.h"
#include "queue.h"
#include "noNull.h"
#include "noNullField.h"
#include "ol_lexer.h"
#include "ol_context.h"
#include "routerConfigGenerator.h"

extern int ol_parser_debug;


static const int SUCCESSORSIZE = 16;





void killJoin()
{
  exit(0);
}


str LOCAL("127.0.0.1:10000");
str REMOTE("Remote.com");
str FINGERIP("Finger.com");

struct SuccessorGenerator : public FunctorSource::Generator
{
  virtual ~SuccessorGenerator() {};
  TupleRef operator()() const {
    
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("successor"));
    
    str myAddress = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(myAddress));
    
    IDRef successor = ID::mk((uint32_t) rand());
    tuple->append(Val_ID::mk(successor));
  
    str succAddress = str(strbuf() << successor->toString() << "IP");
    tuple->append(Val_Str::mk(succAddress));
    tuple->freeze();
    return tuple;
  }
};

struct SuccessorGenerator successorGenerator;


void initializeBaseTables(ref< OL_Context> ctxt, ref< RouterConfigGenerator> routerConfigGenerator)
{
  // create another dataflow to send in the lookups via another udp

  TableRef fingerTable = routerConfigGenerator->getTableByName(LOCAL, "finger");
  OL_Context::TableInfo* fingerTableInfo = ctxt->getTableInfos()->find("finger")->second;

  IDRef me = ID::mk((uint32_t) 1);

  // Fill up the table with fingers
  for (int i = 0;
       i < fingerTableInfo->size;
       i++) {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("finger"));

    str myAddress = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(myAddress));

    IDRef target = ID::mk((uint32_t) 0X200)->shift(10 * i)->add(me);
    //tuple->append(Val_ID::mk(target));

    tuple->append(Val_Int32::mk(i));

    IDRef best = ID::mk()->add(target)->add(ID::mk((uint32_t) i*10));
    tuple->append(Val_ID::mk(best));
  
    str address = str(strbuf() << FINGERIP << ":" << i);
    tuple->append(Val_Str::mk(address));
    tuple->freeze();
    fingerTable->insert(tuple);
    std::cout << tuple->toString() << "\n";
  }
  

  {
    TableRef nodeTable = routerConfigGenerator->getTableByName(LOCAL, "node");
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("node"));
    
    str myAddress = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(myAddress));
    tuple->append(Val_ID::mk(me));
    tuple->freeze();
    nodeTable->insert(tuple);
    std::cout << "Node: " << tuple->toString() << "\n";
  }
    


  {  
    TableRef bestSuccessorTable = routerConfigGenerator->getTableByName(LOCAL, "bestSuccessor");
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("bestSuccessor"));
    
    str myAddress = str(strbuf() << LOCAL);
    tuple->append(Val_Str::mk(myAddress));
    
    IDRef target = ID::mk((uint32_t) 0X200)->add(me);
    IDRef best = ID::mk()->add(target)->add(ID::mk((uint) 10));
    tuple->append(Val_ID::mk(best));
    
    str address = str(strbuf() << FINGERIP << ":" << 0);
    tuple->append(Val_Str::mk(address));
    tuple->freeze();
    
    bestSuccessorTable->insert(tuple);
    std::cout << "BestSuccessor: " << tuple->toString() << "\n";
  }
}


/** Test lookups. */
void startChord(LoggerI::Level level, ref< OL_Context> ctxt, str datalogFile)
{
  // create dataflow for translated chord lookup rules
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ref< RouterConfigGenerator > routerConfigGenerator = New refcounted< RouterConfigGenerator >(ctxt, conf, false, true, datalogFile);

  routerConfigGenerator->createTables(LOCAL);

  ref< Udp > udp = New refcounted< Udp > (LOCAL, 10000);
  routerConfigGenerator->configureRouter(udp, LOCAL);

   
  // populate the finger entries
  initializeBaseTables(ctxt, routerConfigGenerator);
  
  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized network of chord lookup flows.\n";
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
  std::cout << "\nTranslated Chord\n";

  if (argc < 2) {
    std::cerr << "Usage: ./chordDatalog file [loglevel seed]\n";
    return 1;
  }

  ref< OL_Context > ctxt = New refcounted< OL_Context>();
  strbuf filename;
  filename << argv[1];
  str file(filename);
  std::ifstream istr(file);
  ctxt->parse_stream(&istr);
   
  LoggerI::Level level = LoggerI::WARN;

  if (argc > 2) {
    str levelName(argv[2]);
    level = LoggerI::levelFromName[levelName];
  }

  int seed = 0;
  if (argc > 3) {
    seed = atoi(argv[3]);
  }
  srand(seed);

  startChord(level, ctxt, file);
  return 0;
}
  










