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

#ifndef __OL_ROUTERCONFIG_H__
#define __OL_ROUTERCONFIG_H__

//#if HAVE_CONFIG_H
//#include <config.h>
//#endif /* HAVE_CONFIG_H */

#include <async.h>
#include <list>
#include <map>
#include <arpc.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>

#include "ol_context.h"
#include "value.h"
#include "parser_stuff.h"
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

class RouterConfigGenerator {
  // takes as input the udp send / receive, the router config, accept each other code or not
public:

  class FieldNamesTracker;
  struct ReceiverInfo;

  RouterConfigGenerator(OL_Context* ctxt, Router::ConfigurationRef conf, 
			bool _dups, bool debug, str filename);
  RouterConfigGenerator::~RouterConfigGenerator();

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
  FILE *_output;
  Router::ConfigurationRef _conf; 
  std::map<str, str> _multTableIndices;

  // counter to determine how many muxers and demuxers are needed
  str _currentType;
  std::vector<ElementSpecRef> _udpPushSenders, _udpPullSenders;
   
  ReceiverInfoMap _udpReceivers; // for demuxing
  bool _pendingRegisterReceiver;
  str _pendingReceiverTable;

  // Relational -> P2 elements
  void processFunctor(OL_Context::Functor* currentFunctor, 
		      str nodeID);

  ElementSpecRef createScanElements(OL_Context::Functor* currentFunctor, 
				    OL_Context::Rule* rule,
				    OL_Context::Term term, 
				    OL_Context::TableInfo* tableInfo, 
				    str nodeID);
  
  ElementSpecRef generateJoinElements(OL_Context::Functor* currentFunctor, 
				      OL_Context::Rule* currentRule, 
				      str nodeID,
				      FieldNamesTracker* namesTracker);

  ElementSpecRef generateProbeElements(OL_Context::Rule* currentRule, 
				       OL_Context::Term eventTerm, 
				       OL_Context::Term baseTerm, 
				       str nodeID, 
				      FieldNamesTracker* namesTracker, 					    
				      ElementSpecRef priorElement,
				      int joinOrder);

  ElementSpecRef generateSelectionElements(OL_Context::Rule* currentRule, 
					   OL_Context::Term nextSelection, 
					   str nodeID, 
					   FieldNamesTracker* tracker,
					   ElementSpecRef priorElement, 
					   int selectionID); 

  ElementSpecRef generateProjectHeadElements(OL_Context::Functor* currentFunctor, 
					     OL_Context::Rule* currentRule,
					     str nodeID,
					     FieldNamesTracker* currentNamesTracker,
					     ElementSpecRef connectingElement);

  ElementSpecRef generateAllSelectionElements(OL_Context::Rule* currentRule,
					      str nodeID,
					      FieldNamesTracker* currentNamesTracker,
					      ElementSpecRef connectingElement);

  ElementSpecRef generateAllAssignmentElements(OL_Context::Rule* currentRule,
					       str nodeID,
					       FieldNamesTracker* currentNamesTracker,
					       ElementSpecRef connectingElement);

  ElementSpecRef generateAssignmentElements(OL_Context::Rule* currentRule,
					    OL_Context::Term currentTerm, 
					    str nodeID,
					    FieldNamesTracker* currentNamesTracker,
					    ElementSpecRef connectingElement,
					    int assignmentID); 

  ElementSpecRef generatePrintElement(str header, ElementSpecRef connectingElement);

  ElementSpecRef generatePrintWatchElement(str header, ElementSpecRef connectingElement);

  ElementSpecRef generateDupElimElement(str header, ElementSpecRef connectingElement);
  
  void generateSingleTermElement(OL_Context::Functor* currentFunctor, 
				 OL_Context::Rule* currentRule, 
				 str nodeID, 
				 FieldNamesTracker* namesTracker);

  void generateSingleAggregateElements(OL_Context::Functor* currentFunctor, 
				       OL_Context::Rule* currentRule, 
				       str nodeID, 
				       FieldNamesTracker* currentNamesTracker);
  
  // Network elements
  void generateSendElements(ref< Udp> udp, str nodeID);
  void generateReceiveElements(ref< Udp> udp, str nodeID);
  void registerReceiverTable(str tableName);
  void registerReceiver(str tableName, ElementSpecRef elementSpecRef);
  ElementSpecRef generateSendMarshalElements(OL_Context::Rule* rule, str nodeID, 
					     int arity, ElementSpecRef toSend);
  ElementSpecRef generateFunctorSource(OL_Context::Functor* functor, OL_Context::Rule* rule, str nodeID);

  // Helper functions
  void hookUp(ElementSpecRef firstElement, int firstPort,
	      ElementSpecRef secondElement, int secondPort);  
  str getRuleStr(OL_Context::Functor* currentFunctor, 
		 OL_Context::Rule* currentRule);
  void addMultTableIndex(TableRef table, int fn, str nodeID);
  bool isSelection(OL_Context::Term term);  
  bool isAssignment(OL_Context::Term term);
  bool isAggregation(OL_Context::Term term);
  bool hasSingleAggTerm(OL_Context::Rule* rule);
  bool hasPeriodicTerm(OL_Context::Rule* currentRule);

  // convince placeholder to figure out the current fields in a tuple in flight
  class FieldNamesTracker {
  public:
    std::vector<str> fieldNames;
    
    FieldNamesTracker();   
    FieldNamesTracker(std::vector<str> names, int arity);    
    void initialize(std::vector<str> names, int arity);    
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

  void setJoinKeys(OL_Context::Rule* rule, 
		   std::vector<JoinKey>* joinKeys);

  struct ReceiverInfo {
    std::vector<ElementSpecRef> _receivers;
    str _name;
    ReceiverInfo(str name) {
      _name = name;
    }      
    void addReceiver(ElementSpecRef elementSpecRef) { 
      _receivers.push_back(elementSpecRef);
    }
  };
};

#endif /* __OL_ROUTERCONFIG_H_ */



