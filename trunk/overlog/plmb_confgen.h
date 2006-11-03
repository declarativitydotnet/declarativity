// -*- c-basic-offset: 2; related-file-name: "ol_context.h" -*-
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
 * DESCRIPTION: Takes as input the system environment (udp send / receive, dups) and 
 * the Overlog parsing context, and then generate the Plumber Configuration
 *              
 */

#ifndef __OL_RTR_CONFGEN_H__
#define __OL_RTR_CONFGEN_H__

//#if HAVE_CONFIG_H
//#include <config.h>
//#endif /* HAVE_CONFIG_H */

#include <list>
#include <map>
#include <iostream>
#include <ostream>
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

class Plmb_ConfGen {
 private:
  class FieldNamesTracker;
  struct ReceiverInfo;
  struct PreconditionInfo;

 public:
  Plmb_ConfGen(OL_Context* ctxt, Plumber::DataflowPtr conf, 
               bool _dups, bool debug, bool cc, string filename, 
	       bool rTracing = false,
               std::ostream& s=*(new ostringstream()), bool edit=false);
  ~Plmb_ConfGen();

  void configurePlumber(boost::shared_ptr< Udp > udp, string nodeID);

  Table2Ptr
  getTableByName(string nodeID, string tableName);


  /** creates the tables for a given node */
  void
  createTables(string nodeID);

  void clear();

  // allow driver program to push data into dataflow
  void registerUDPPushSenders(ElementSpecPtr elementSpecPtr,
			      OL_Context::Rule * curRule,
			      string nodeid);
  
private:
  static const string SEL_PRE, AGG_PRE, ASSIGN_PRE, TABLESIZE;
  typedef std::map<string, Table2Ptr> TableMap;
  typedef std::map<string, string> PelFunctionMap;
  typedef std::map<string, ReceiverInfo> ReceiverInfoMap;

  PelFunctionMap pelFunctions;
  TableMap _tables;
  OL_Context* _ctxt; // the context after parsing
  bool _dups; // do we care about duplicates in our dataflow? 
  bool _debug; // do we stick debug elements in?
  bool _cc; // are we using congestion control
  FILE *_output;
  Plumber::DataflowPtr _conf; 
  std::ostream& _p2dl;
  bool          _edit;
  std::map<string, string> _multTableIndices;

  // counter to determine how many muxers and demuxers are needed
  string _curType;
  std::vector<ElementSpecPtr> _udpSenders;
  std::vector<int> _udpSendersPos;
  std::vector<ElementSpecPtr> _currentElementChain;
   
  ReceiverInfoMap _udpReceivers; // for demuxing
  bool _pendingRegisterReceiver;
  string  _pendingReceiverTable;
  ElementSpecPtr _pendingReceiverSpec;
  OL_Context::Rule* _currentRule;
  ElementSpecPtr _ccTx, _ccRx, _roundRobinCC;
  bool _isPeriodic;
  int _currentPositionIndex;
  ElementSpecPtr agg_spec;
  
  // Data-structures for rule tracing [EuroSys]
  // start here **

  // a flag to indicate if we need these data structures
  bool _ruleTracing; 

  // a flag to indicate if we need an input port at the
  // RoundRobin for handling traced tuples
  bool _needTracingPortAtRR;

  typedef std::map<int, PreconditionInfo> PrecondInfoMap;
  
  std::map<int, ElementSpecPtr> * _taps_beg;
  std::map<int, ElementSpecPtr> * _taps_end;
  PrecondInfoMap _taps_for_precond;
  std::vector<ElementSpecPtr>  _taps_beg_vector;
  std::vector<ElementSpecPtr>  _taps_end_vector;
  std::vector<ElementSpecPtr>  _ruleTracers;

  std::vector<ElementSpecPtr> _traceTupleElements;
  std::vector<ElementSpecPtr> _tableTracers;

  void initTracingState(bool);
  void genTappedDataFlow(string nodeid);
  ElementSpecPtr createTapElement(OL_Context::Rule *r);
  ElementSpecPtr find_tap(int ruleId, int beg_or_end);
  void genTraceElement(string header);

  // end here **

  // Relational -> P2 elements
  void processRule(OL_Context::Rule *r, string nodeID);
  void genEditFinalize(string); 
  
  void genJoinElements(OL_Context::Rule* curRule, 
		       string nodeID,
		       FieldNamesTracker* namesTracker,
		       boost::shared_ptr<Aggwrap> agg_el);

  void genProbeElements(OL_Context::Rule* curRule, 
			Parse_Functor* eventFunctor, 
			Parse_Term* baseTerm, 
			string nodeID, 	     
			FieldNamesTracker* probeNames, 
			FieldNamesTracker* baseProbeNames, 
			int joinOrder,
			b_cbv *comp_cb);

  void genProjectHeadElements(OL_Context::Rule* curRule,
 			      string nodeID,
 			      FieldNamesTracker* curNamesTracker);
    
  void genAllSelectionAssignmentElements(OL_Context::Rule* curRule,
					 string nodeID,
					 FieldNamesTracker* curNamesTracker);
    
  void genDupElimElement(string header);
  
  void genSingleTermElement(OL_Context::Rule* curRule, 
			    string nodeID, 
			    FieldNamesTracker* namesTracker);
  
  void genSingleAggregateElements(OL_Context::Rule* curRule, 
				  string nodeID, 
				  FieldNamesTracker* curNamesTracker);  


  // Debug elements
  void genPrintElement(string header);

  void genPrintWatchElement(string header);

  void genFunctorSource(OL_Context::Rule* rule, 
			string nodeID,
			FieldNamesTracker* namesTracker);
 
  // Network elements
  ElementSpecPtr genSendElements(boost::shared_ptr< Udp> udp, string nodeID);

  void genReceiveElements(boost::shared_ptr< Udp> udp, 
			  string nodeID, 
			  ElementSpecPtr wrapAroundDemux);

  void registerReceiverTable(OL_Context::Rule* rule, 
			     string tableName);

  void registerReceiver(string tableName, 
			ElementSpecPtr elementSpecPtr,
			OL_Context::Rule * _curRule);



  // Pel Generation functions
  string pelRange(FieldNamesTracker* names, 
		  Parse_Bool *expr,
		  OL_Context::Rule* rule);		  

  string pelMath(FieldNamesTracker* names, 
		 Parse_Math *expr,
		 OL_Context::Rule* rule);

  string pelBool(FieldNamesTracker* names, 
		 Parse_Bool *expr,
		 OL_Context::Rule* rule);

  string pelFunction(FieldNamesTracker* names, 
		     Parse_Function *expr,
		     OL_Context::Rule* rule);

  void pelSelect(OL_Context::Rule* rule, 
		 FieldNamesTracker *names, 
		 Parse_Select *expr, 
                 string nodeID, 
		 int selectionID);

  void pelAssign(OL_Context::Rule* rule, 
		 FieldNamesTracker *names, 
		 Parse_Assign *expr, 
                 string nodeID, 
		 int assignID);

  // Other helper functions
  void hookUp(ElementSpecPtr firstElement, 
	      int firstPort,
	      ElementSpecPtr secondElement, 
	      int secondPort);  

  void hookUp(ElementSpecPtr secondElement, 
	      int secondPort);  
  
  /** Create a secondary index in the given table. */
  void
  secondaryIndex(Table2Ptr table, 
                 Table2::Key key, 
                 string nodeID);
  

  int numFunctors(OL_Context::Rule* rule);

  /** Return the parse functor that makes up the vent term of the given
      rule, if one exists. Otherwise, return null */
  Parse_Functor*
  eventTerm(OL_Context::Rule* rule);
  


  bool hasPeriodicTerm(OL_Context::Rule* curRule);

  void
  debugRule(OL_Context::Rule* curRule, 
            string debugMsg) { 
    TELL_WARN << "Planner debug rule (" << curRule->ruleID<< "): "
              << debugMsg; 
  }

  void error(string msg);
  void error(string msg, OL_Context::Rule* rule);
  void checkFunctor(Parse_Functor* baseFunctor, OL_Context::Rule* rule);

  // convince placeholder to figure out the cur fields in a tuple in flight
  class FieldNamesTracker {
  public:
    std::vector< string > fieldNames;    


    FieldNamesTracker();   


    FieldNamesTracker(Parse_Term* pf);


    void initialize(Parse_Term* pf);


    /** Generate a lookup key and an index key (as per Table2::lookup())
        for a join from the probe tuple to my local tuple.
        Specifically, in order of my fields, find those fields that
        match an argument of the probe tuple and record for each one of
        my matching field numbers (in the index key) the matching field
        number of the probe tuple (in the lookup key). For example, if
        my arguments are (A, B, C, D) and the probe arguments are (C, L,
        B), then the index key returned is (2, 3) and the lookup key
        returned is (3, 1).  That is B, in my position 2 and C in my
        position 3 match B in the probe's position 3 and C in the
        probe's position 1. Note that field numbers are offset by one to
        account for relation names, which always occupy field position
        0. So indexing starts at 1 here.  The remaining base key
        contains those base field number that did not participate in the
        join, or (1, 4) in the example above. */
    void
    joinKeys(FieldNamesTracker* probeNames,
             Table2::Key& lookupKey,
             Table2::Key& indexKey,
             Table2::Key& remainingBaseKey);
    

    void mergeWith(std::vector<string> names);


    void mergeWith(std::vector<string> names, int numJoinKeys);


    /** Which of my arguments matches the given one? If none matches,
        return 0 */
    int
    fieldPosition(string var);


    string toString();
  };

  
  // keep track of where joins need to be performed
  struct JoinKey {
    string _firstTableName;
    string _firstFieldName;
    string _secondTableName;
    string _secondFieldName;   

    JoinKey(string firstTableName, string firstFieldName, string secondTableName, string secondFieldName) {
      _firstTableName = firstTableName;
      _firstFieldName = firstFieldName;
      _secondTableName = secondTableName;
      _secondFieldName = secondFieldName;
    }
  };

  struct ReceiverInfo {
    std::vector<ElementSpecPtr> _receivers;
    std::vector<int> _ruleNums;
    string _name;
    u_int _arity;
    ReceiverInfo(string name, u_int arity) {
      _name = name;
      _arity = arity;
    }      
    void addReceiver(ElementSpecPtr elementSpecPtr, int ruleNum) { 
      _receivers.push_back(elementSpecPtr);
      _ruleNums.push_back(ruleNum);
      TELL_INFO << "Pushing ruleNum " << ruleNum << " for element " << elementSpecPtr->element()->name() << "\n";
    }
  };

  // rule tracing: data structure to hold precondition info [EuroSys]

  struct PreconditionInfo{
    std::vector<ElementSpecPtr> _preconds;
    int ruleNum;
    PreconditionInfo(int rNum, ElementSpecPtr t){
      ruleNum = rNum;
      addPrecondition(t);
    }
    void addPrecondition(ElementSpecPtr tap){
      _preconds.push_back(tap);
    }
  };

};

#endif /* __OL_RTR_CONFGEN_H_ */
