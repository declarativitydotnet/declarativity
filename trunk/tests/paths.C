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
#include <iostream>
#include <stdlib.h>

#include "tuple.h"
#include "router.h"
#include "val_int32.h"
#include "val_str.h"
#include "val_double.h"

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
#include "loop.h"

#if 0

#include "pathsOpt.h"
#include "cacheMgr.h"
#include "checkCache.h"
#include "reversePath.h"

#endif

extern int ol_parser_debug;

static int STARTING_PORT = 20000;
static string LOCAL("127.0.0.1");
static int nodeNo;
Planner *planner;
FILE* plannerOutput;
static int numNodes;
static int port;
static int seed;
static int duration;
static double periodicPush;
static int metric;
static std::vector<int> updatePeriodSeconds;
static double percentUpdate;
static int updateTimes = 0;
static int correlate; // 0 - not correlated, 1 - correlated no share, 2 - correlated share
static int magicIntervalSeconds;
static int magicNumQueries;
static int magicCache; // 0 - no use cache, 1 - use cache
static double percentMagicDsts;

static int updateRoundRobin = 0;
int queries = 0;
static int numLinks = 0;

void defaultRing(Catalog* catalog, string address)
{
  TuplePtr link = Tuple::mk();
  link->append(Val_Str::mk("link"));
  ostringstream src(address); src << ":" << port;   
  int dstPort = port + 1;
  if (dstPort == (numNodes + port)) {
    dstPort = STARTING_PORT;;
  }

  ostringstream dst(address); dst << ":" << dstPort;   
  link->append(Val_Str::mk(src.str() + "|" + dst.str())); // primary key
  link->append(Val_Str::mk(src.str()));
  link->append(Val_Str::mk(dst.str()));
  link->append(Val_Int32::mk(1));
  link->freeze();

  TuplePtr reverseLink = Tuple::mk();
  reverseLink->append(Val_Str::mk("link"));
  ostringstream src1(LOCAL); src1 << ":" << port;   
  dstPort = port - 1;
  if (dstPort == (STARTING_PORT - 1)) {
    dstPort += numNodes;
  }
  ostringstream dst1(LOCAL); dst1 << ":" << dstPort;   
  reverseLink->append(Val_Str::mk(src1.str() + "|" + dst1.str())); // primary key
  reverseLink->append(Val_Str::mk(src1.str()));
  reverseLink->append(Val_Str::mk(dst1.str()));
  reverseLink->append(Val_Int32::mk(1));
  reverseLink->freeze();
  
  TablePtr tablePtr 
    = catalog->getTableInfo("link")->_table; 
  
  tablePtr->insert(link);
  tablePtr->insert(reverseLink);

}


void loadLinkTable(Catalog* catalog, string inputGraph, string address)
{
  // src, dst, metric1, metric2, ...
  FILE* inputFile = fopen(inputGraph.c_str(), "r");
  char c[200];
  while (fgets(c, 200, inputFile) != NULL) {
    if (address == LOCAL) {
      std::vector<double> metrics;
      int srcI = atoi(strtok(c," "));
      int dstI = atoi(strtok(NULL," "));
      char* st = NULL;
      st = strtok(NULL," ");
      double linkMetric = atof(st);
      if (correlate != 0) {	
	while (st != NULL) {
	  metrics.push_back(atof(st));
	  st = strtok(NULL, " ");
	}
      } else {
	for (int k = 0; k < metric; k++) {
	  linkMetric = atof(strtok(NULL," "));
	}
      }
      if (nodeNo != srcI) {
	continue;
      }

      TuplePtr link = Tuple::mk();
      link->append(Val_Str::mk("link"));
      ostringstream src(LOCAL); src << ":" << (STARTING_PORT + srcI);   
      ostringstream dst(LOCAL); dst << ":" << (STARTING_PORT + dstI);   
      link->append(Val_Str::mk(src.str() + "|" + dst.str())); // primary key
      link->append(Val_Str::mk(src.str()));
      link->append(Val_Str::mk(dst.str()));

      if (correlate != 0) {
	// dump all the metrics
	for (unsigned k = 0; k < metrics.size(); k++) {
	  link->append(Val_Double::mk(metrics.at(k)));
	}	
      } else {
	link->append(Val_Double::mk(linkMetric));
      }
      link->freeze();
      TablePtr tablePtr 
	= catalog->getTableInfo("link")->_table; 
      tablePtr->insert(link);
      numLinks++;
    } else {
      string s = strtok(c," ");
      string d = strtok(NULL," ");
      std::vector<double> metrics;

      char* st = NULL;
      st = strtok(NULL," ");
      double linkMetric = atof(st);
      if (correlate != 0) {	
	while (st != NULL) {
	  metrics.push_back(atof(st));
	  st = strtok(NULL, " ");
	}
      } else {
	for (int k = 0; k < metric; k++) {
	  linkMetric = atof(strtok(NULL," "));
	}
      }

      if (address != s) {
	continue;
      }

      TuplePtr link = Tuple::mk();
      link->append(Val_Str::mk("link"));
      ostringstream src(s); src << ":" << port;  
      ostringstream dst(d); dst << ":" << port;
      link->append(Val_Str::mk(src.str() + "|" + dst.str())); // primary key
      link->append(Val_Str::mk(src.str()));
      link->append(Val_Str::mk(dst.str()));
      if (correlate != 0) {
	// dump all the metrics
	for (unsigned k = 0; k < metrics.size(); k++) {
	  link->append(Val_Double::mk(metrics.at(k)));
	}	
      } else {
	link->append(Val_Double::mk(linkMetric));
      }
      link->freeze();
      TablePtr tablePtr 
	= catalog->getTableInfo("link")->_table; 
      tablePtr->insert(link);
      numLinks ++;
    }
  }
}


void initializeLinkTable(Catalog* catalog, string address, string inputGraph)
{

  warn << "Initialize link table\n";
  // read from input file
  if (inputGraph == "0") {
    defaultRing(catalog, address);
  } else {
    loadLinkTable(catalog, inputGraph, address);
    warn << "Number of links " << numLinks << "\n";
  }
}  

void quit()
{
  warn << "Query has completed\n";
  //fclose(plannerOutput);
  delete planner;
  exit(0);
}


//typedef std::map<str, int> IntMap;
//static IntMap previousDsts;
static int offset = 1;

void insertMagicSource(boost::shared_ptr< Catalog> catalog, string address)
{
  int nodeVal = rand() % numNodes;
  int dstVal = (nodeVal + offset) % (int) (numNodes * percentMagicDsts);
  
  while (nodeVal == dstVal) {
    dstVal = (nodeVal + offset) % (int) (numNodes * percentMagicDsts);
  }
  queries ++;
  if (queries > magicNumQueries) { return; }

  if (nodeVal == nodeNo) {
    warn << "Issue source query, " << nodeVal << ", " << dstVal << ", " << queries << "\n";

    offset++;
    /*bool prevFlag = false;
    if (magicCache == true) {
      IntMap::iterator iter = previousDsts.find(str(strbuf() << dstVal));
      if (iter != previousDsts.end()) {
	prevFlag = true;
	warn << "Issue before. skip\n";
      }
      previousDsts.insert(std::make_pair(str(strbuf() << dstVal), dstVal));
      // has this been issued before? If so, we can skip
    }
    if (prevFlag == false) {*/
      // pick a random node that is not itself
    TablePtr tablePtr 
      = catalog->getTableInfo("magic")->_table; 
    
    TuplePtr magicSource = Tuple::mk();
    ostringstream src(address); src << ":" << port;  
    ostringstream dst(address); dst << ":" << (STARTING_PORT + dstVal);  
    magicSource->append(Val_Str::mk("magic"));
    magicSource->append(Val_Str::mk(src.str()));
    magicSource->append(Val_Str::mk(dst.str()));
    magicSource->append(Val_Int32::mk(queries));
    tablePtr->insert(magicSource);
    //}
  }
  delayCB(magicIntervalSeconds, boost::bind(&insertMagicSource, catalog, address));   
}

// XXX: not used for correlated experiment
void updateLinks(boost::shared_ptr< Catalog> catalog)
{
  // go through all the links. Pick X% to change by Y\%
  // insert into the table
  // generate a number from 
  warn << "Update links\n";
  TablePtr tablePtr 
    = catalog->getTableInfo("link")->_table; 

  Table::UniqueScanIterator linkScan = tablePtr->uniqueScanAll(1, false);
  std::vector<TuplePtr> newTuples;
  while (linkScan->done() == false) {
    TuplePtr linkTuple = linkScan->next();
    int randomVal = rand() % 100;
    warn << "Update link " << randomVal << " " << (int) (percentUpdate * 100) << "\n";
    if (randomVal < (100 * percentUpdate)) {
      // increase or decrease latency metric by X\%
      warn << "Update link old " << linkTuple->toString() << "\n";
      // inserto into table.
      TuplePtr newLink = Tuple::mk();
      //newLink->append(Val_Str::mk("link"));

      for (unsigned k = 0; k < linkTuple->size(); k++) {
	// insert all fields back, except for latency (field 4)
	ValuePtr nextVal = (*linkTuple)[k];       
	if (k == 4) {
	  double val = Val_Double::cast(nextVal);
	  int maxIncr = 10;
	  int maxDecr = 5;
	  if (updateTimes % 2 == 0) {	   	    
	    double changePercent = ((double) (rand() % maxIncr)) / 100;
	    newLink->append(Val_Double::mk(val * (1 + changePercent)));
	  } else {
	    double changePercent = ((double) (rand() % maxDecr)) / 100;
	    newLink->append(Val_Double::mk(val * (1 - changePercent)));
	  }
	} else {
	  newLink->append(nextVal);
	}
      }
      newLink->freeze();      
      warn << "Update link new " << newLink->toString() << "\n";      
      newTuples.push_back(newLink);
    }       
  }
  for (unsigned k = 0; k < newTuples.size(); k++) {
    tablePtr->insert(newTuples.at(k));
  }

  updateTimes++;
  delayCB(updatePeriodSeconds.at(updateRoundRobin % updatePeriodSeconds.size()), bind(updateLinks, catalog));
  updateRoundRobin++;
}

#if 0
void generateReversePathElement(Router::ConfigurationPtr conf, ptr< Catalog> catalog)
{
  // update bestPathReverse event element -> printWatch 
  // -> cacheGenerator -> printWatch -> Insert
  warn << "Generate cache element\n";
  Catalog::TableInfo* ti =  catalog->getTableInfo("bestPathReverse");
  if (ti == NULL) {
    warn << "Table bestPathReverse is not found\n";
    exit(-1);
    return;
  }
  TablePtr tablePtr = ti->_table;

  Catalog::TableInfo* tc =  catalog->getTableInfo("sendBestPathReverse");
  TablePtr sendTablePtr = tc->_table;
  
  unsigned primaryKey = ti->_tableInfo->primaryKeys.at(0);
  Table::UniqueScanIterator uniqueIterator = tablePtr->uniqueScanAll(primaryKey, true);
  ElementSpecPtr updateTable =
    conf->addElement(New refcounted< Scan >(strbuf("ScanUpdateCacheTwo|") 						  
					    << "|bestPathReverse" 
					    << "|" << nodeNo,
					    uniqueIterator, 
					    true));
  
  ElementSpecPtr printOne = 
    conf->addElement(New refcounted< 
		     PrintWatch >(strbuf("PrintWatchRBPReverseCacheTwo"),
				  catalog->getWatchTables()));
    
  ElementSpecPtr pullPushBPReverse = 
    conf->addElement(New refcounted< 
		     TimedPullPush >(strbuf("BPReversePullPushTwo|"), 0));    
  
  ElementSpecPtr reversePath
    = conf->addElement(New refcounted< ReversePath >(strbuf("ReversePath") << nodeNo));
  
  ElementSpecPtr insertElement
    = conf->addElement(New refcounted< 
		       Insert >(strbuf("InsertSendReversePath|") 
				<< "rBPCache|" << nodeNo,
				sendTablePtr));
  
  ElementSpecPtr printTwo = 
    conf->addElement(New refcounted< 
		     PrintWatch >(strbuf("PrintWatchAddCacheTwo"),
				  catalog->getWatchTables()));
  
  ElementSpecPtr pullPushCache = 
    conf->addElement(New refcounted< 
		     TimedPullPush >(strbuf("RBPCachePullPush|"), 0));  
  
  
  ElementSpecPtr sinkS 
    = conf->addElement(New refcounted< Discard >("DiscardInsertTwo"));
  
  conf->hookUp(updateTable, 0, printOne, 0);
  conf->hookUp(printOne, 0, pullPushBPReverse, 0);
  conf->hookUp(pullPushBPReverse, 0, reversePath, 0);
  conf->hookUp(reversePath, 0, insertElement, 0);
  conf->hookUp(insertElement, 0, printTwo, 0);
  conf->hookUp(printTwo, 0, pullPushCache, 0);
  conf->hookUp(pullPushCache, 0, sinkS, 0);
}


void generateCacheElement(Router::ConfigurationPtr conf, ptr< Catalog> catalog)
{
  // update bestPathReverse event element -> printWatch 
  // -> cacheGenerator -> printWatch -> Insert
  warn << "Generate cache element\n";
  Catalog::TableInfo* ti =  catalog->getTableInfo("bestPathReverse");
  if (ti == NULL) {
    warn << "Table bestPathReverse is not found\n";
    exit(-1);
    return;
  }
  TablePtr tablePtr = ti->_table;

  Catalog::TableInfo* tc =  catalog->getTableInfo("bestPathCache");
  TablePtr tableCachePtr = tc->_table;

  unsigned primaryKey = ti->_tableInfo->primaryKeys.at(0);
  Table::UniqueScanIterator uniqueIterator = tablePtr->uniqueScanAll(primaryKey, true);
  ElementSpecPtr updateTable =
    conf->addElement(New refcounted< Scan >(strbuf("ScanUpdateCache|") 						  
					    << "|bestPathReverse" 
					    << "|" << nodeNo,
					    uniqueIterator, 
					    true));

  ElementSpecPtr printOne = 
    conf->addElement(New refcounted< 
		     PrintWatch >(strbuf("PrintWatchBPReverseCache"),
					 catalog->getWatchTables()));
  
  ElementSpecPtr pullPushBPReverse = 
    conf->addElement(New refcounted< 
		     TimedPullPush >(strbuf("BPReversePullPush|"), 0));    

  ElementSpecPtr cacheMgr 
    = conf->addElement(New refcounted< CacheMgr >(strbuf("CacheMgr") << nodeNo,
						  numLinks));
  
  ElementSpecPtr insertElement
    = conf->addElement(New refcounted< 
		       Insert >(strbuf("InsertBPCache|") 
				<< "bPCache|" << nodeNo,
				tableCachePtr));
  
  ElementSpecPtr printTwo = 
    conf->addElement(New refcounted< 
		     PrintWatch >(strbuf("PrintWatchAddCache"),
					 catalog->getWatchTables()));
  
  ElementSpecPtr pullPushCache = 
   conf->addElement(New refcounted< 
		    TimedPullPush >(strbuf("BPCachePullPush|"), 0));  
  
  
  ElementSpecPtr sinkS 
    = conf->addElement(New refcounted< Discard >("DiscardInsert"));
  
  conf->hookUp(updateTable, 0, printOne, 0);
  conf->hookUp(printOne, 0, pullPushBPReverse, 0);
  conf->hookUp(pullPushBPReverse, 0, cacheMgr, 0);
  conf->hookUp(cacheMgr, 0, insertElement, 0);
  conf->hookUp(insertElement, 0, printTwo, 0);
  conf->hookUp(printTwo, 0, pullPushCache, 0);
  conf->hookUp(pullPushCache, 0, sinkS, 0);
    
}
#endif

/** Build a symmetric link transitive closure. */
void runPathQuery(Router::ConfigurationPtr conf, 
		  LoggerI::Level level, boost::shared_ptr< OL_Context> ctxt, 
		  string address, std::vector<string> filenames, 
		  string inputGraph, string debugFile)
{
  string filename = filenames.at(0);
  warn << "Execute path query " << filename << "\n";
  warn << "Start node " << address << " " << port << "\n";
  string outputFile(filename + ".out");
  plannerOutput = fopen(outputFile.c_str(), "w");

  string ecaFile(debugFile + ".eca");
  FILE *ecaOutput = fopen(ecaFile.c_str(), "w");

  boost::shared_ptr< Udp > udp;
  boost::shared_ptr< Catalog > catalog;
  boost::shared_ptr< ECA_Context > ectxt;
  
  ostringstream nodeID;
  nodeID << address << ":" << port;
  udp.reset(new Udp("Udp:"+nodeID.str(), port));

  // for each filename

  catalog.reset(new Catalog());  
  catalog->initTables(ctxt.get()); 
  catalog->setWatchTables(ctxt->getWatchTables());  

  ectxt.reset(new ECA_Context()); 
  ectxt->eca_rewrite(ctxt.get(), catalog.get());  
  fprintf(ecaOutput, "%s\n", ectxt->toString().c_str());
   
  ostringstream oss;
  oss << debugFile << "-" << nodeNo;
  planner = new Planner (conf, catalog.get(), false, nodeID.str(), oss.str());      
  std::vector<RuleStrand*> ruleStrands = 
    planner->generateRuleStrands(ectxt);

  for (unsigned k = 0; k < ruleStrands.size(); k++) {
    fprintf(plannerOutput, "%s", ruleStrands.at(k)->toString().c_str());
  }

  
  // set timer on quitting, initialize link table, periodic updates
  delayCB(duration, boost::bind(quit));
  delayCB(10, boost::bind(initializeLinkTable, catalog.get(), address, inputGraph));
  if (updatePeriodSeconds.at(0) != -1) {
    delayCB(10 + updatePeriodSeconds.at(updateRoundRobin % updatePeriodSeconds.size()), boost::bind(updateLinks, catalog));
    updateRoundRobin++;
  }

  // add a path optimizer, timedpullpush, 
  /*
  if (periodicPush > 0 || magicCache != 0) {    
    std::vector<ElementSpecPtr> optimizeSend;

    if (periodicPush > 0) {
      std::vector<unsigned int> groupByFields;
      groupByFields.push_back(2);
      groupByFields.push_back(3);
      
      std::vector<str> pathNames;
      if (correlate != 0) {
	pathNames.push_back(str("pathZero"));
	pathNames.push_back(str("pathOne"));
	pathNames.push_back(str("pathTwo"));
      } else {
	pathNames.push_back(str("path"));
      }
      
      bool correlateShare = (correlate == 2);
      
      ElementSpecPtr pathsOpt =
	conf->addElement(New refcounted< PathsOpt >(strbuf("PathsOpt|") 
						    << address << ":" << port,
						    pathNames,
						    groupByFields, 
						    5, 6, 
						    periodicPush, correlateShare));
      ElementSpecPtr pullPush = 
	conf->addElement(New refcounted< 
			 TimedPullPush >(strbuf("PathsOptTimedPullPush|")
					 << address << ":" << port, 0));
      
      optimizeSend.push_back(pathsOpt);
      optimizeSend.push_back(pullPush);
    } 
    
    if (magicCache != 0) {

      // find the cache table
      // create a checkCache table
      Catalog::TableInfo* tc =  catalog->getTableInfo("bestPathCache");
      TablePtr tableCachePtr = tc->_table;

      ElementSpecPtr pullPush = 
	conf->addElement(New refcounted< 
			 TimedPullPush >(strbuf("CheckCachePullPush|")
					 << address << ":" << port, 0));      

      Catalog::TableInfo* tc1 =  catalog->getTableInfo("cacheHit");
      TablePtr cacheHitPtr = tc1->_table;

      ElementSpecPtr checkCache = 
	conf->addElement(New refcounted< 
			 CheckCache >(strbuf("CheckCache|")
				      << address << ":" << port, 
				      tableCachePtr, cacheHitPtr));      
      
      optimizeSend.push_back(checkCache);
      optimizeSend.push_back(pullPush);
    }

    planner->registerOptimizeSend(optimizeSend);    
    }
  */
  
  planner->setupNetwork(udp);
  planner->registerAllRuleStrands(ruleStrands);
  fprintf(plannerOutput, "%s\n", planner->getNetPlanner()->toString().c_str());
  fclose(plannerOutput);
  fclose(ecaOutput);    

  /*
  if (magicCache != 0) {
    // set up a special element 
    // takes as input update events, generate cache elements
    generateCacheElement(conf, catalog);
  }
    
  if (magicIntervalSeconds != -1) {
    generateReversePathElement(conf, catalog);
  }
  */

  RouterPtr router(new Router(conf, level));
  if (router->initialize(router) == 0) {
    warn << "Correctly initialized network of reachability flows.\n";
  } else {
    warn << "** Failed to initialize correct spec\n";
    exit(-1);
  }

  if (magicIntervalSeconds != -1) {
    delayCB(10 + magicIntervalSeconds, boost::bind(insertMagicSource, catalog, address));
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();

}



int main(int argc, char **argv)
{
  boost::shared_ptr< OL_Context > ctxt(new OL_Context());

  if (argc < 20) {
    fatal << "Usage:\n\t paths <numNodes> <nodeNo> <address> <port> <datalogFile> <inputgraph> <outputFile> <loggingLevel> <seed> <duration> <periodicSend> <metric> <updatePeriodSeconds> <percentUpdate> <correlate> <magicIntervalSeconds> <magicNumQueries> <magicCache> <percentMagicDsts>\n";
  }

  numNodes = atoi(argv[1]);
  nodeNo = atoi(argv[2]);
  string address(argv[3]);
  port = atoi(argv[4]);
  string datalogFile(argv[5]);
  string inputGraph(argv[6]);  
  string debugFile(argv[7]);
  string levelName(argv[8]);
  seed = atoi(argv[9]);
  duration = atoi(argv[10]);
  periodicPush = atof(argv[11]);
  metric = atoi(argv[12]);
  
  // put the update periods into an array
  char* st = strtok(argv[13],",");
  while (st != NULL) {
    updatePeriodSeconds.push_back(atoi(st));
    st = strtok(NULL,",");    
  }
   
  percentUpdate = atof(argv[14]);
  correlate = atoi(argv[15]);
  magicIntervalSeconds = atoi(argv[16]);
  magicNumQueries = atoi(argv[17]);
  magicCache = atoi(argv[18]);
  percentMagicDsts = atof(argv[19]);
  
  std::cout << "Parameters: ";
  for (int k = 1; k < argc; k++) {
    std::cout << argv[k] << " ";
  }
  std::cout << "\n";

  std::ifstream istr(datalogFile.c_str());
  ctxt->parse_stream(&istr);
  LoggerI::Level level = LoggerI::levelFromName[levelName];

  if (updatePeriodSeconds.at(0) == -1) {
    srandom(0);
  } else {
    srandom(seed);
  }

  std::cout << "Finish parsing (functors / tableInfos) " << ctxt->getRules()->size() 
	    << " " << ctxt->getTableInfos()->size() << "\n";

  Router::ConfigurationPtr conf(new Router::Configuration());

  if (correlate != 0) {
    std::vector<string> filenames;
    filenames.push_back(datalogFile);
    // for each datalogFile, call runPathQuery
    runPathQuery(conf, level, ctxt, address, filenames, inputGraph, debugFile);
  } else {
    std::vector<string> filenames;
    filenames.push_back(datalogFile);
    // for each datalogFile, call runPathQuery
    runPathQuery(conf, level, ctxt, address, filenames, inputGraph, debugFile);
  }

  return 0;
}
  

/*
 * End of file 
 */
