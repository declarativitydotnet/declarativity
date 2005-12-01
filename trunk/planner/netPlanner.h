
/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Planner code
 *
 */

#ifndef __PL_NETPLANNER_H__
#define __PL_NETPLANNER_H__

#include "router.h"
#include "elementSpec.h"
#include "element.h"
#include <vector>

#include "ol_context.h"
#include "value.h"
#include "parser_util.h"
#include "ol_lexer.h"
#include "tuple.h"
#include "router.h"
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
#include "table.h"
#include "lookup.h"
#include "insert.h"
#include "scan.h"
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
#include "cc.h"
#include "measureBW.h"
//#include "pathsIn.h"
//#include "agg.h"

#include "ruleStrand.h"
#define QUEUESIZE 1000

class NetPlanner
{
public:
  NetPlanner(Router::ConfigurationRef conf, str nodeID, 
	     FILE* outputDebugFile) :
    _conf(conf)
  { _nodeID = nodeID; _outputDebugFile = outputDebugFile;
  _receiveMux = NULL; _sendRoundRobin = NULL;};

  ~NetPlanner() { }; 
  void generateNetworkElements(ref<Udp> udp);
  void registerAllRuleStrands(std::vector<RuleStrand*>);

  void registerOptimizeSend(std::vector<ElementSpecRef> outOptimize) {
    _outOptimize = outOptimize; }

  str toString();

  class ReceiverInfo {
  public: 
    ReceiverInfo(str tableName) 
    { _tableName = tableName; _duplicator = NULL; }
    str _tableName;
    ElementSpecPtr _duplicator;
    std::vector<ElementSpecRef> _receivers;
    str toString();
  };

  typedef std::map<str, ReceiverInfo*> ReceiverInfoMap;  
  
private:
  void generateNetworkOutElements(ref<Udp> udp);
  void generateNetworkInElements(ref<Udp> udp);

  str _nodeID;
  FILE* _outputDebugFile;
  Router::ConfigurationRef _conf;
  std::vector<ElementSpecRef> _networkIn, _networkOut;
  std::vector<ElementSpecRef> _outOptimize;
  ReceiverInfoMap _receiverInfo;  
  std::vector<ElementSpecRef> _senders;
  ElementSpecPtr _receiveMux, _sendRoundRobin;
};

#endif
