/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Planner program that compiles rules and connect to net planner
 *
 */

#ifndef __PL_PLANNER_H__
#define __PL_PLANNER_H__

#include <list>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <fstream>

#include "ol_context.h"
#include "value.h"
#include "parser_util.h"
#include "ol_lexer.h"
#include "tuple.h"
#include "plumber.h"
#include "val_int32.h"
#include "val_str.h"
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
#include "unboxField.h"
#include "mux.h"
#include "demux.h"
#include "strToSockaddr.h"
#include "slot.h"
#include "timedPullSink.h"
#include "timestampSource.h"
#include "hexdump.h"
#include "table2.h"
#include "refTable.h"
#include "commonTable.h"
#include "lookup2.h"
#include "insert.h"
#include "queue.h"
#include "printTime.h"
#include "roundRobin.h"
#include "noNullField.h"
#include "functorSource.h"
#include "delete.h"
#include "tupleSource.h"
#include "printWatch.h"
#include "aggregate.h"
#include "duplicateConservative.h"
#include "aggwrap.h"
#include "tupleseq.h"
#include "loop.h"
#include "ruleTracer.h"
#include "tap.h"
#include "traceTuple.h"

#include "tableStore.h"
#include "eca_context.h"
#include "ruleStrand.h"
#include "netPlanner.h"

class Planner
{
public:
  Planner(Plumber::DataflowPtr conf, TableStore* catalog, 
	  bool debug, string nodeID, string outputFile); 

  ~Planner() { delete _netPlanner; 
  if (_outputDebugFile != NULL) {fclose(_outputDebugFile); }}

  std::vector<RuleStrand*> generateRuleStrands(ECA_ContextPtr ectxt);
  void registerRuleStrand(RuleStrand* rs);
  void registerAllRuleStrands(std::vector<RuleStrand*>);  
  void generatePlumberConfig(Plumber::DataflowPtr conf);
  void setupNetwork(boost::shared_ptr<Udp> udp);
  NetPlanner* getNetPlanner() { return _netPlanner; }

private:
  bool _debug;
  Plumber::DataflowPtr _conf;
  string _nodeID;
  int _ruleCount;
  NetPlanner* _netPlanner;
  TableStore* _tableStore;
  FILE* _outputDebugFile;
};

#endif
