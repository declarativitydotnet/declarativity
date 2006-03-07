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
 * DESCRIPTION: A symmetric reachability data flow
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <stdlib.h>

#include "tuple.h"
#include "plumber.h"
#include "val_int32.h"
#include "val_str.h"

#include "ol_lexer.h"
#include "ol_context.h"
#include "plmb_confgen.h"
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

extern int ol_parser_debug;

void killJoin()
{
  exit(0);
}

static int LINKS = 2;
static int STARTING_PORT = 10000;
static const int nodes = 4;



/* Initial sending of links into the p2 dataflow */
void bootstrapData(Plumber::ConfigurationPtr conf,
                   string name,
                   TablePtr linkTable,
 		   boost::shared_ptr< Udp > udp)
{
  // Scanner element over link table
  ElementSpecPtr scanS 
    = conf->addElement(ElementPtr(new Scan(string("ScanLink:" + name + ":"), linkTable, 0)));

  ElementSpecPtr scanPrintS =
    conf->addElement(ElementPtr(new Print(string("PrintScan:") + name)));
  ElementSpecPtr timedPullPushS =
    conf->addElement(ElementPtr(new TimedPullPush(string("PushReach:").append(name), 0)));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("Slot:").append(name))));
  
  ElementSpecPtr encapS =
    conf->addElement(ElementPtr(new PelTransform(string("encap:").append(name),
						    "$1 pop /* The From address */\
                                                     $0 ->t $1 append $2 append pop"))); // the rest
  
  // Now marshall the payload (second field)
  ElementSpecPtr marshalS =
    conf->addElement(ElementPtr(new MarshalField(string("Marshal:").append(name), 1)));
  ElementSpecPtr routeS =
    conf->addElement(ElementPtr(new StrToSockaddr(string("Router:").append(name), 0)));
  ElementSpecPtr udpTxS =
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
void testReachability(LoggerI::Level level, boost::shared_ptr< OL_Context> ctxt, string filename)
{
  std::cout << "\nCHECK TRANSITIVE REACHABILITY\n";
  eventLoopInitialize();

  Plumber::ConfigurationPtr conf(new Plumber::Configuration());
  Plmb_ConfGen plumberConfigGenerator(ctxt, conf, false, false, false, filename);

  // Create one data flow per "node"
  ptr< Udp > udps[nodes];
  ptr< Udp > bootstrapUDP[nodes];
  ValuePtr nodeIds[nodes];
  string names[nodes];
  TablePtr bootstrapTables[nodes];
   
  // Create the networking objects
  for (int i = 0; i < nodes; i++) {
    ostringstream o1;
    ostringstream o2;
    o1 << string("Node") << i;
    o2 << string("127.0.0.1") << ":" << (STARTING_PORT + i);
    names[i] = o1.str();
    nodeIds[i] = Val_Str::mk(o2.str());
    
    ostringstream oss;
    oss << string("bootstrapNeighbor:") << i;

    udps[i].reset(new Udp(names[i] << ":Udp", STARTING_PORT + i));
    bootstrapUDP[i].reset(new Udp(names[i] << ":bootstrapUdp", STARTING_PORT + i + 5000));
    plumberConfigGenerator.createTables(names[i]);
    bootstrapTables[i].reset(new Table(oss.str(), 100));
    bootstrapTables[i]->add_multiple_index(0);  
  }


  // create a random topology
  for (int i = 0; i < nodes; i++) {
    // initial loading of data into p2 dataflows
  
    // Create the link tables and links themselves.  Links must be
    // symmetric
    std::set< int > others;
    others.insert(i);           // me
    
    for (int j = 0; j < LINKS; j++) {
      TuplePtr t = Tuple::mk();
      t->append(Val_Str::mk("neighbor"));
      // From me
      t->append((ValuePtr) nodeIds[i]);
      // To someone at random, but not me. Replacement is discouraged

      int other = i;
      while (others.find(other) != others.end()) {
        other = (int) (1.0 * nodes * rand() / (RAND_MAX + 1.0));
      }
      // Got one
      others.insert(other);

      t->append((ValuePtr) nodeIds[other]);
      t->freeze();
      TablePtr tablePtr = plumberConfigGenerator.getTableByName(names[i], "neighbor");
      std::cout << "Node " << i << " insert tuple " << t->toString() << "\n";
      tablePtr->insert(t);
      bootstrapTables[i]->insert(t);

      // Make the symmetric one
      TuplePtr symmetric = Tuple::mk();
      symmetric->append(Val_Str::mk("neighbor"));
      symmetric->append((ValuePtr) nodeIds[other]);
      symmetric->append((ValuePtr) nodeIds[i]);
      symmetric->freeze();
      tablePtr = plumberConfigGenerator.getTableByName(names[other], "neighbor");
      std::cout << "Node " << other << " insert tuple " << symmetric->toString() << "\n";
      tablePtr->insert(symmetric);
      bootstrapTables[other]->insert(symmetric);
    }
  }

  for (int i = 0; i < nodes; i++) {
    bootstrapData(conf, "bootstrap" << names[i], bootstrapTables[i], bootstrapUDP[i]);
    plumberConfigGenerator.clear(); 
    plumberConfigGenerator.configurePlumber(udps[i], names[i]);      
  }

  // add extra here to initialize the tables

  PlumberPtr plumber(new Plumber(conf, level));
  if (plumber->initialize(plumber) == 0) {
    std::cout << "Correctly initialized network of reachability flows.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the plumber
  plumber->activate();

  // Run the plumber
  eventLoop();
}



int main(int argc, char **argv)
{
  std::cout << "\nGRAPH REACHABILITY\n";
  srand(0);


  LoggerI::Level level = LoggerI::NONE;
  boost::shared_ptr< OL_Context > ctxt(new OL_Context());
  ostringstream filename;

  for( int i=1; i<argc; i++) { 
    string arg(argv[i]);
    
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
      string file(filename.str());
      std::ifstream istr(file);
      ctxt->parse_stream(&istr);
    }
  }

  std::cout << "Finish parsing (functors / tableInfos) " << ctxt->getRules()->size() 
	    << " " << ctxt->getTableInfos()->size() << "\n";

  testReachability(level, ctxt, filename);

  return 0;
}
  

/*
 * End of file 
 */
