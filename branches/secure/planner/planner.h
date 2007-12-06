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
#include "val_int64.h"
#include "val_str.h"
#include "print.h"
#include "discard.h"
#include "pelTransform.h"
#include "duplicate.h"
#include "dupElim.h"
#include "timedPullPush.h"
#include "udp.h"
#include "marshalField.h"
#include "unmarshalField.h"
#include "unboxField.h"
#include "mux.h"
#include "demux.h"
#include "strToSockaddr.h"
#include "slot.h"
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
#include "delete.h"
#include "tupleSource.h"
#include "printWatch.h"
#include "aggregate.h"
#include "duplicateConservative.h"
#include "aggwrap2.h"
#include "tupleseq.h"
#include "loop.h"

#include "tableStore.h"
#include "eca_context.h"
#include "ruleStrand.h"
#include "stageStrand.h"
#include "netPlanner.h"

class Planner
{
public:
  Planner(Plumber::DataflowPtr conf, TableStore* catalog, 
	  bool debug, string nodeID); 

  ~Planner() { delete _netPlanner; }


  /** Generate a vector of all rule strands drawn from ECA rules */
  std::vector< RuleStrand* >
  generateRuleStrands(ECA_ContextPtr ectxt);


  /** Generate a vector of all external stage strands. */
  std::vector< StageStrand* >
  generateStageStrands(OL_Context* ctxt);

  /** Generate a single stage strand from a stage specification */
  StageStrand*
  generateStageStrand(const OL_Context::ExtStageSpec* spec,
                      std::string strandID);


  void
  registerRuleStrand(RuleStrand* rs);

  void
  registerAllRuleStrands(std::vector<RuleStrand*>,
                         std::vector<StageStrand*>);  

  void
  generatePlumberConfig(Plumber::DataflowPtr conf);
  
  void
  setupNetwork(boost::shared_ptr<Udp> udp);
  
  NetPlanner*
  getNetPlanner() { return _netPlanner; }

  

private:
  bool _debug;
  Plumber::DataflowPtr _conf;
  string _nodeID;
  int _ruleCount;
  NetPlanner* _netPlanner;
  TableStore* _tableStore;
};


#define PLANNER_LOG(_reportingLevel,_pc,_rest) "Planner, "       \
  << (_pc->_ruleStrand->getRuleID())                             \
    << ":"                                                       \
       << (_pc->_ruleStrand->getStrandID())                      \
    << ", "                                                      \
       << _reportingLevel                                        \
          << ", "                                                \
             << errno                                            \
                << ", "                                          \
                   << _rest

#define PLANNER_LOG_NOPC(_reportingLevel,_pc,_rest) "Planner, "  \
  << "-:-, "                                                        \
     << _reportingLevel                                          \
        << ", "                                                  \
           << errno                                              \
              << ", "                                            \
                 << _rest

#define PLANNER_ERROR(_pc,_rest) TELL_ERROR       \
  << PLANNER_LOG(Reporting::ERROR,_pc,_rest)      \
    << "\n"

#define PLANNER_WORDY(_pc,_rest) TELL_WORDY       \
  << PLANNER_LOG(Reporting::WORDY,_pc,_rest)      \
    << "\n"

#define PLANNER_INFO(_pc,_rest) TELL_INFO       \
  << PLANNER_LOG(Reporting::INFO,_pc,_rest)      \
    << "\n"

#define PLANNER_WARN(_pc,_rest) TELL_WARN       \
  << PLANNER_LOG(Reporting::WARN,_pc,_rest)      \
    << "\n"

#define PLANNER_OUTPUT(_pc,_rest) TELL_OUTPUT    \
  << PLANNER_LOG(Reporting::OUTPUT,_pc,_rest)      \
    << "\n"


#define PLANNER_ERROR_NOPC(_rest) TELL_ERROR       \
  << PLANNER_LOG_NOPC(Reporting::ERROR,_pc,_rest)  \
    << "\n"

#define PLANNER_WORDY_NOPC(_rest) TELL_WORDY       \
  << PLANNER_LOG_NOPC(Reporting::WORDY,_pc,_rest)  \
    << "\n"

#define PLANNER_INFO_NOPC(_rest) TELL_INFO       \
  << PLANNER_LOG_NOPC(Reporting::INFO,_pc,_rest) \
    << "\n"

#define PLANNER_WARN_NOPC(_rest) TELL_WARN       \
  << PLANNER_LOG_NOPC(Reporting::WARN,_pc,_rest) \
    << "\n"

#define PLANNER_OUTPUT_NOPC(_rest) TELL_OUTPUT       \
  << PLANNER_LOG_NOPC(Reporting::OUTPUT,_pc,_rest)   \
    << "\n"


#endif
