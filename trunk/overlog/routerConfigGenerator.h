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
  
private:
  static const str FUNC_PRE;
  typedef std::map<str, TableRef> TableMap;
  typedef std::map<str, str> PelFunctionMap;
  typedef std::multimap<str, ReceiverInfo> ReceiverInfoMap;

  PelFunctionMap pelFunctions;
  TableMap _tables;
  OL_Context* _ctxt; // the context after parsing
  bool _dups; // do we care about duplicates in our dataflow? 
  bool _debug; // do we stick debug elements in?
  FILE *_output;

  // counter to determine how many muxers and demuxers are needed
  str _currentType;
  std::vector<ElementSpecRef> _udpPushSenders, _udpPullSenders;
   
  ReceiverInfoMap _udpReceivers; // for demuxing
  Router::ConfigurationRef _conf; 

  // Relational -> P2 elements
  void processFunctor(OL_Context::Functor* currentFunctor, 
		      str nodeID);

  ElementSpecRef createScanElements(OL_Context::Functor* currentFunctor, 
				    OL_Context::Rule* rule,
				    OL_Context::Term term, 
				    OL_Context::TableInfo* tableInfo, 
				    str nodeID);

  ElementSpecRef generateSingleTermElement(OL_Context::Functor* currentFunctor, 
					   OL_Context::Rule* currentRule, 
					   str nodeID, 
					   FieldNamesTracker* namesTracker);

  ElementSpecRef generateEventProbeElements(OL_Context::Functor* currentFunctor, 
					    OL_Context::Rule* currentRule, 
					    str nodeID,
					    FieldNamesTracker* namesTracker);

  ElementSpecRef generateJoinElements(OL_Context::Rule* currentRule, 
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

  // Network elements
  void generateSendElements(ref< Udp> udp, str nodeID);
  void generateReceiveElements(ref< Udp> udp, str nodeID);
  ElementSpecRef generateSendMarshalElements(OL_Context::Rule* rule, str nodeID, 
					     int arity, ElementSpecRef toSend);
  ElementSpecRef generateReceiveUnmarshalElements(OL_Context::Functor* currentFunctor, 
						  OL_Context::Rule* currentRule, 
						  OL_Context::Term curentTerm,
						  str nodeID);
  
  // Helper functions
  void hookUp(ElementSpecRef firstElement, int firstPort,
	      ElementSpecRef secondElement, int secondPort);  
  str getRuleStr(OL_Context::Functor* currentFunctor, 
		 OL_Context::Rule* currentRule);
  

  // convince placeholder to figure out the current fields in a tuple in flight
  class FieldNamesTracker {
  public:
    std::vector<str> fieldNames;
    
    FieldNamesTracker();   
    FieldNamesTracker(std::vector<str> names, int arity);    
    void initialize(std::vector<str> names, int arity);    
    int matchingJoinKey(std::vector<str> names);    
    void mergeWith(std::vector<str> names);
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
    ElementSpecRef _receiver;
    int _numFields;
    str _name;
    ReceiverInfo(ElementSpecRef receiver, int numFields, str name): _receiver(receiver) {
      _numFields = numFields;
      _name = name;
    }      
  };
};

#endif /* __OL_ROUTERCONFIG_H_ */



