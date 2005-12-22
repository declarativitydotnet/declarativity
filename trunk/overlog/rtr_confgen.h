// -*- c-basic-offset: 2; related-file-name: "ol_context.h" -*-
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
 * DESCRIPTION: Takes as input the system environment (udp send / receive, dups) and 
 * the Overlog parsing context, and then generate the Router Configuration
 *              
 */

#ifndef __OL_RTR_CONFGEN_H__
#define __OL_RTR_CONFGEN_H__

//#if HAVE_CONFIG_H
//#include <config.h>
//#endif /* HAVE_CONFIG_H */

#include <list>
#include <map>
#include <arpc.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>

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
#include "loop.h"

class Rtr_ConfGen {
 private:
  class FieldNamesTracker;
  struct ReceiverInfo;

 public:
  Rtr_ConfGen(OL_Context* ctxt, Router::ConfigurationRef conf, 
	      bool _dups, bool debug, bool cc, str filename);
  Rtr_ConfGen::~Rtr_ConfGen();

  void configureRouter(ref< Udp > udp, str nodeID);

  TableRef getTableByName(str nodeID, str tableName);

  void createTables(str nodeID);

  void clear();

  // allow driver program to push data into dataflow
  void registerUDPPushSenders(ElementSpecRef elementSpecRef);
  
private:
  static const str SEL_PRE, AGG_PRE, ASSIGN_PRE, TABLESIZE;
  typedef std::map<str, TableRef> TableMap;
  typedef std::map<str, str> PelFunctionMap;
  typedef std::map<str, ReceiverInfo> ReceiverInfoMap;

  PelFunctionMap pelFunctions;
  TableMap _tables;
  OL_Context* _ctxt; // the context after parsing
  bool _dups; // do we care about duplicates in our dataflow? 
  bool _debug; // do we stick debug elements in?
  bool _cc; // are we using congestion control
  FILE *_output;
  Router::ConfigurationRef _conf; 
  std::map<str, str> _multTableIndices;

  // counter to determine how many muxers and demuxers are needed
  str _curType;
  std::vector<ElementSpecRef> _udpSenders;
  std::vector<int> _udpSendersPos;
  std::vector<ElementSpecRef> _currentElementChain;
   
  ReceiverInfoMap _udpReceivers; // for demuxing
  bool _pendingRegisterReceiver;
  str  _pendingReceiverTable;
  ElementSpecPtr _pendingReceiverSpec;
  OL_Context::Rule* _currentRule;
  ElementSpecPtr _ccTx, _ccRx, _roundRobinCC;
  bool _isPeriodic;
  int _currentPositionIndex;
  
  // Relational -> P2 elements
  void processRule(OL_Context::Rule *r, str nodeID);
  
  void genJoinElements(OL_Context::Rule* curRule, 
		       str nodeID,
		       FieldNamesTracker* namesTracker,
		       ptr<Aggwrap> agg_el);

  void genProbeElements(OL_Context::Rule* curRule, 
			Parse_Functor* eventFunctor, 
			Parse_Term* baseTerm, 
			str nodeID, 	     
			FieldNamesTracker* probeNames, 
			FieldNamesTracker* baseProbeNames, 
			int joinOrder,
			b_cbv comp_cb);

  void genProjectHeadElements(OL_Context::Rule* curRule,
 			      str nodeID,
 			      FieldNamesTracker* curNamesTracker);
    
  void genAllSelectionAssignmentElements(OL_Context::Rule* curRule,
					 str nodeID,
					 FieldNamesTracker* curNamesTracker);
    
  void genDupElimElement(str header);
  
  void genSingleTermElement(OL_Context::Rule* curRule, 
			    str nodeID, 
			    FieldNamesTracker* namesTracker);
  
  void genSingleAggregateElements(OL_Context::Rule* curRule, 
				  str nodeID, 
				  FieldNamesTracker* curNamesTracker);  


  // Debug elements
  void genPrintElement(str header);

  void genPrintWatchElement(str header);

  void genFunctorSource(OL_Context::Rule* rule, 
			str nodeID,
			FieldNamesTracker* namesTracker);
 
  // Network elements
  ElementSpecRef genSendElements(ref< Udp> udp, str nodeID);

  void genReceiveElements(ref< Udp> udp, 
			  str nodeID, 
			  ElementSpecRef wrapAroundDemux);

  void registerReceiverTable(OL_Context::Rule* rule, 
			     str tableName);

  void registerReceiver(str tableName, 
			ElementSpecRef elementSpecRef);



  // Pel Generation functions
  str pelRange(FieldNamesTracker* names, 
	       Parse_Bool *expr);

  str pelMath(FieldNamesTracker* names, 
	      Parse_Math *expr);

  str pelBool(FieldNamesTracker* names, 
	      Parse_Bool *expr);

  str pelFunction(FieldNamesTracker* names, 
		  Parse_Function *expr);

  void pelSelect(OL_Context::Rule* rule, 
		 FieldNamesTracker *names, 
		 Parse_Select *expr, 
                 str nodeID, 
		 int selectionID);

  void pelAssign(OL_Context::Rule* rule, 
		 FieldNamesTracker *names, 
		 Parse_Assign *expr, 
                 str nodeID, 
		 int assignID);

  // Other helper functions
  void hookUp(ElementSpecRef firstElement, 
	      int firstPort,
	      ElementSpecRef secondElement, 
	      int secondPort);  

  void hookUp(ElementSpecRef secondElement, 
	      int secondPort);  
  
  void addMultTableIndex(TableRef table, 
			 int fn, 
			 str nodeID);
  
  int numFunctors(OL_Context::Rule* rule);

  bool hasEventTerm(OL_Context::Rule* rule);
  
  Parse_Functor* getEventTerm(OL_Context::Rule* curRule);

  bool hasPeriodicTerm(OL_Context::Rule* curRule);

  void debugRule(OL_Context::Rule* curRule, 
		 str debugMsg) { 
    std::cout << curRule->ruleID << ": " << debugMsg; 
  }


  // convince placeholder to figure out the cur fields in a tuple in flight
  class FieldNamesTracker {
  public:
    std::vector<str> fieldNames;    
    FieldNamesTracker();   
    FieldNamesTracker(Parse_Term* pf);

    void initialize(Parse_Term* pf);
    std::vector<int> matchingJoinKeys(std::vector<str> names);    
    void mergeWith(std::vector<str> names);
    void mergeWith(std::vector<str> names, int numJoinKeys);
    int fieldPosition(str var);
    str toString();
  };

  
  // keep track of where joins need to be performed
  struct JoinKey {
    str _firstTableName;
    str _firstFieldName;
    str _secondTableName;
    str _secondFieldName;   

    JoinKey(str firstTableName, str firstFieldName, str secondTableName, str secondFieldName) {
      _firstTableName = firstTableName;
      _firstFieldName = firstFieldName;
      _secondTableName = secondTableName;
      _secondFieldName = secondFieldName;
    }
  };

  struct ReceiverInfo {
    std::vector<ElementSpecRef> _receivers;
    str _name;
    u_int _arity;
    ReceiverInfo(str name, u_int arity) {
      _name = name;
      _arity = arity;
    }      
    void addReceiver(ElementSpecRef elementSpecRef) { 
      _receivers.push_back(elementSpecRef);
    }
  };
};

#endif /* __OL_RTR_CONFGEN_H_ */
