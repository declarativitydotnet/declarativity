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
 * DESCRIPTION: Network planner code
 *
 */

#ifndef __PL_NETPLANNER_H__
#define __PL_NETPLANNER_H__

#include "plumber.h"
#include "elementSpec.h"
#include "element.h"
#include <vector>

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

#include "ruleStrand.h"
#define QUEUESIZE 1000

class NetPlanner
{
public:
  NetPlanner(Plumber::DataflowPtr conf, string nodeID, 
	     FILE* outputDebugFile) :
    _conf(conf)
  { _nodeID = nodeID; _outputDebugFile = outputDebugFile; };

  ~NetPlanner() { }; 
  void generateNetworkElements(boost::shared_ptr<Udp> udp);
  void registerAllRuleStrands(std::vector<RuleStrand*>);
  string toString();

  class ReceiverInfo {
  public: 
    ReceiverInfo(string tableName) 
    { _tableName = tableName; _duplicator.reset(); }
    string _tableName;
    ElementSpecPtr _duplicator;
    std::vector<ElementSpecPtr> _receivers;
    string toString();
  };

  typedef std::map<string, ReceiverInfo*> ReceiverInfoMap;  
  
private:
  void generateNetworkOutElements(boost::shared_ptr<Udp> udp);
  void generateNetworkInElements(boost::shared_ptr<Udp> udp);

  string _nodeID;
  FILE* _outputDebugFile;
  Plumber::DataflowPtr _conf;
  std::vector<ElementSpecPtr> _networkIn, _networkOut;
  ReceiverInfoMap _receiverInfo;  
  std::vector<ElementSpecPtr> _senders;
  ElementSpecPtr _receiveMux, _sendRoundRobin;
};

#endif
