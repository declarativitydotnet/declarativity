
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
 * DESCRIPTION: Code to generate dataflows
 *
 */

#ifndef __PL_RULEPLANNER_C__
#define __PL_RULEPLANNER_C__


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
//#include "aggregate.h"
#include "duplicateConservative.h"
//#include "aggwrap.h"
#include "tupleseq.h"
#include "cc.h"
#include "scan.h"
#include "agg.h"
#include "ruleStrand.h"
#include "planContext.h"
#include "catalog.h"


void debugRule(PlanContext* pc, str msg)
{
  warn << "DEBUG RULE: " << pc->_ruleStrand->_ruleID << " " 
       << pc->_ruleStrand->_ruleStrandID << " " << msg; 
}

#include "rulePel.C"

void addWatch(PlanContext* pc, strbuf b)
{
  RuleStrand* rs = pc->_ruleStrand;
  str bStr(b);

  if (pc->_outputDebugFile == NULL) {
    ElementSpecRef print = 
      pc->_conf->addElement(New refcounted< PrintWatch >(b, pc->_catalog->getWatchTables()));
    rs->addElement(pc->_conf, print);  
  } else {
    ElementSpecRef print = 
      pc->_conf->addElement(New refcounted< 
			    PrintWatch >(b, pc->_catalog->getWatchTables(), 
					 pc->_outputDebugFile));
    rs->addElement(pc->_conf, print);  
  }
}

void generateEventElement(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  pc->_namesTracker 
    = New PlanContext::FieldNamesTracker(rs->_eca_rule->_event->_pf);
  
  // update, create a scanner, 
  if (rs->eventType() == Parse_Event::UPDATE || 
      rs->eventType() == Parse_Event::AGGUPDATE) {
    debugRule(pc, strbuf() << "Update event NamesTracker " 
	      << pc->_namesTracker->toString() << "\n");
    Catalog::TableInfo* ti =  pc->_catalog->getTableInfo(rs->eventFunctorName());
    if (ti == NULL) {
      warn << "Table " << rs->eventFunctorName() << "is not found\n";
      exit(-1);
      return;
    }
    TableRef tableRef = ti->_table;
          
    unsigned primaryKey = ti->_tableInfo->primaryKeys.at(0);
    Table::UniqueScanIterator uniqueIterator = tableRef->uniqueScanAll(primaryKey, true);
    ElementSpecRef updateTable =
       pc->_conf->addElement(New refcounted< Scan >(strbuf("ScanUpdate|") 
						    << curRule->_ruleID 
						    << "|" << rs->eventFunctorName()
						    << "|" << pc->_nodeID,
						    uniqueIterator, 
						    true));
    rs->addElement(pc->_conf, updateTable);

    // add a debug element
    addWatch(pc, strbuf("DebugAfterUpdateEvent|")
	     << curRule->_ruleID 
	     << "|" << pc->_nodeID);
    
    if (curRule->_probeTerms.size() > 0) {
      ElementSpecRef pullPush = 
	pc->_conf->addElement(New refcounted< 
			      TimedPullPush >(strbuf("UpdateEventTimedPullPush|")
						     << curRule->_ruleID 
						     << "|" << pc->_nodeID, 0));
      rs->addElement(pc->_conf, pullPush);
    }
  }

  if (rs->eventType() == Parse_Event::RECV) {
    debugRule(pc, strbuf() << "Recv event NamesTracker " 
	      << pc->_namesTracker->toString() << "\n");
    addWatch(pc, strbuf("DebugAfterRecvEvent|")
	     << curRule->_ruleID 
	     << "|" << pc->_nodeID);

    if (curRule->_probeTerms.size() == 0) {
      ElementSpecRef slot = 
	pc->_conf->addElement(New refcounted< Slot >(strbuf("RecvEventSlot|")
						     << curRule->_ruleID 
						     << "|" << pc->_nodeID));
      rs->addElement(pc->_conf, slot);
    }
  }

}

void generateActionElement(PlanContext* pc)
{
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  RuleStrand* rs = pc->_ruleStrand;

  // add, insert
  if (rs->actionType() == Parse_Action::ADD) {    
    // add a debug element
    addWatch(pc, strbuf("DebugBeforeInsertAction|")
	     << curRule->_ruleID << "|"
	     << rs->actionFunctorName() 
	     << "|" << pc->_nodeID);

    Catalog::TableInfo* ti =  pc->_catalog->getTableInfo(rs->actionFunctorName());    
    if (ti == NULL) {
      warn << "Table " << rs->actionFunctorName() << "is not found\n";
      exit(-1);
      return;
    }
    TableRef tableRef = ti->_table;

    ElementSpecRef insertElement
      = pc->_conf->addElement(New refcounted< 
			  Insert >(strbuf("Insert|") << 
				   curRule->_ruleID << "|" 
				   << rs->actionFunctorName() 
				   << "|" << pc->_nodeID,
				   tableRef));

    ElementSpecRef insertPullPush = 
      pc->_conf->addElement(New refcounted<
			TimedPullPush>(strbuf("Insert|") <<
				       curRule->_ruleID 
				       << "|" << pc->_nodeID, 0));
    
    ElementSpecRef sinkS 
    = pc->_conf->addElement(New refcounted< Discard >("DiscardInsert"));

    rs->addElement(pc->_conf, insertElement);
    rs->addElement(pc->_conf, insertPullPush);
    rs->addElement(pc->_conf, sinkS);
  }

  if (rs->actionType() == Parse_Action::SEND) {    
    // do a pel transform, figure out what is the field number
    Parse_Functor* head = curRule->_action->_pf;
    str loc = head->fn->loc;
    int locationIndex = 0;
    for (int k = 0; k < head->args(); k++) {
      Parse_Var* parse_var = dynamic_cast<Parse_Var*>(head->arg(k));
      if (parse_var != NULL && parse_var->toString() == loc) {
	locationIndex = k+1;
      }
    }
    addWatch(pc, strbuf("DebugBeforeSendAction|") 
	     << curRule->_ruleID 
	     << "|" << pc->_nodeID);

    ElementSpecRef sendPelTransform =
      pc->_conf->addElement(New refcounted< 
			    PelTransform >(strbuf("SendActionAddress|") 
					   << curRule->_ruleID 
					   << "|" << pc->_nodeID,
					   strbuf("$") << 
					   locationIndex << " pop swallow pop"));
    
    rs->addElement(pc->_conf, sendPelTransform);   
  }   
}


void generateProbeElements(PlanContext* pc, Parse_Functor* probedFunctor)
{
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  RuleStrand* rs = pc->_ruleStrand;

  PlanContext::FieldNamesTracker* incomingNamesTracker = pc->_namesTracker;
  PlanContext::FieldNamesTracker *probedNamesTracker
    = new PlanContext::FieldNamesTracker(probedFunctor);   

  std::vector<int> leftJoinKeys 
    = probedNamesTracker->matchingJoinKeys(incomingNamesTracker->fieldNames);

  std::vector<int> rightJoinKeys 
    =  incomingNamesTracker->matchingJoinKeys(probedNamesTracker->fieldNames);

  str probedTableName = probedFunctor->fn->name;
  
  if (leftJoinKeys.size() == 0 || rightJoinKeys.size() == 0) {
    std::cerr << "No matching join keys " << rs->eventFunctorName() << " " << 
      probedTableName << " " << curRule->_ruleID << "\n";
  }

  // add one to offset for table name. Join the first matching key
  int leftJoinKey = leftJoinKeys.at(0) + 1;
  int rightJoinKey = rightJoinKeys.at(0) + 1;

  Catalog::TableInfo* ti =  pc->_catalog->getTableInfo(probedTableName);
  if (ti == NULL) {
    warn << "Table " << probedTableName << "is not found\n";
    exit(-1);
    return;
  }

  TableRef probedTable = ti->_table;
  
  // should we use a uniqLookup or a multlookup? 
  // Check that the rightJoinKey is the primary key
  OL_Context::TableInfo* tableInfo 
    = pc->_catalog->getTableInfo(probedTableName)->_tableInfo;
 
  if (tableInfo->primaryKeys.size() == 1 && 
      tableInfo->primaryKeys.at(0) == rightJoinKey) {
    // use a unique index
    ElementSpecRef lookupElement =
      pc->_conf->addElement(New refcounted< 
			UniqueLookup >(strbuf("UniqueLookup|") 
				       << curRule->_ruleID 
				       << "|" << probedTableName
				       << "|" << pc->_nodeID, 
				       probedTable,
				       leftJoinKey, 
				       rightJoinKey, 
				       cbv_null));
    debugRule(pc, str(strbuf() << "Unique lookup " << " " 
			   << rs->eventFunctorName() << " " 
			   << probedTableName << " " 
			   << leftJoinKey << " " 
			   << rightJoinKey << "\n"));

    pc->_ruleStrand->addElement(pc->_conf, lookupElement);    
  } else {
    ElementSpecRef lookupElement =
      pc->_conf->addElement(New refcounted< MultLookup >(strbuf("MultLookup|") 
						     << curRule->_ruleID 
						     << "|" << probedTableName 
						     << "|" << pc->_nodeID, 
						     probedTable,
						     leftJoinKey, 
						     rightJoinKey, cbv_null));

    pc->_catalog->createMultIndex(probedTableName, rightJoinKey);
  
    debugRule(pc, str(strbuf() << "Mult lookup " << curRule->_ruleID 
			   << " " << probedTableName << " " 
			   << leftJoinKey << " " << rightJoinKey << "\n"));

    pc->_ruleStrand->addElement(pc->_conf, lookupElement);
  }
  
 
  int numFieldsProbe = incomingNamesTracker->fieldNames.size();
  debugRule(pc, str(strbuf() << "Probe before merge " 
			 << incomingNamesTracker->toString() << "\n"));
  incomingNamesTracker->mergeWith(probedNamesTracker->fieldNames); 
  debugRule(pc, str(strbuf() << "Probe after merge " 
			 << incomingNamesTracker->toString() << "\n"));

 
  ElementSpecRef noNull 
    = pc->_conf->addElement(New refcounted< NoNullField >(strbuf("NoNull|") 
						      << curRule->_ruleID 
						      << "|" << probedTableName
						      << "|" << pc->_nodeID, 1));

  pc->_ruleStrand->addElement(pc->_conf, noNull);

  
  debugRule(pc, str(strbuf() << "Number of join selections " 
		    << leftJoinKeys.size()-1 << "\n"));

  for (uint k = 1; k < leftJoinKeys.size(); k++) {
    int leftField = leftJoinKeys.at(k);
    int rightField = rightJoinKeys.at(k);
    strbuf selectionPel;
    selectionPel << "$0 " << (leftField+1) << " field " << " $1 " 
		 << rightField+1 << " field == not ifstop $0 pop $1 pop";

    debugRule(pc, str(strbuf() << "Join selections " 
			   << str(selectionPel) << "\n"));
    ElementSpecRef joinSelections =
      pc->_conf->addElement(New refcounted< 
			    PelTransform >(strbuf("JoinSelections|") 
					   << curRule->_ruleID << "|" 
					   << probedTableName << "|" 
					   << k << "|" << pc->_nodeID, 
					   str(selectionPel)));    
    pc->_ruleStrand->addElement(pc->_conf, joinSelections);
  }
 
  // Take the joined tuples and produce the resulting path
  // form the pel projection. 
  //Keep all fields on left, all fields on right except the join keys
  strbuf pelProject("\"join|");
  pelProject << rs->eventFunctorName() << "|" << probedTableName << "|" 
	     << curRule->_ruleID << "|" << pc->_nodeID << "\" pop "; // table name
  for (int k = 0; k < numFieldsProbe; k++) {
    pelProject << "$0 " << k+1 << " field pop ";
  }
  for (uint k = 0; k < probedNamesTracker->fieldNames.size(); k++) {
    bool joinKey = false;
    for (uint j = 0; j < rightJoinKeys.size(); j++) {
      if (k == (uint) rightJoinKeys.at(j)) { // add one for table name
	joinKey = true;
	break;
      }
    }
    if (!joinKey) {
      pelProject << "$1 " << k+1 << " field pop ";
    }
  }

  str pelProjectStr(pelProject);
  ElementSpecRef joinPelTransform 
    = pc->_conf->addElement(New refcounted< 
			PelTransform >(strbuf("JoinPelTransform|") 
				       << curRule->_ruleID << "|" 
				       << probedTableName << "|" << pc->_nodeID, 
				       pelProjectStr));

  delete probedNamesTracker;

  pc->_ruleStrand->addElement(pc->_conf, joinPelTransform);

}


void generateMultipleProbeElements(PlanContext* pc)
{
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  RuleStrand* rs = pc->_ruleStrand;

  for (uint k = 0; k < curRule->_probeTerms.size(); k++) {    
    Parse_Functor* pf = curRule->_probeTerms.at(k);
    
    debugRule(pc, str(strbuf() << "Probing " << curRule->_event->_pf->fn->name
		      << " " << pf->toString() << "\n"));
    
    generateProbeElements(pc, pf);

    if (curRule->_probeTerms.size() - 1 != k) {
      ElementSpecRef pullPush =
	pc->_conf->addElement(New refcounted< 
			  TimedPullPush >(strbuf("ProbePullPush|") 
					  << curRule->_ruleID << "|" 
					  << pc->_nodeID << "|" << k,
					  0));
      rs->addElement(pc->_conf, pullPush);
    }
  }
}


void generateSelectionAssignmentElements(PlanContext* pc)
{
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  for (unsigned int j = 0; j < curRule->_selectAssignTerms.size(); j++) {
    Parse_Select* parse_select 
      = dynamic_cast<Parse_Select *>(curRule->_selectAssignTerms.at(j));
    if (parse_select != NULL) {
      debugRule(pc, str(strbuf() << "Selection term " << 
			parse_select->toString() << " " 
			<< curRule->_ruleID << "\n"));
      pelSelect(pc, parse_select, j); 
    }
    Parse_Assign* parse_assign 
      = dynamic_cast<Parse_Assign *>(curRule->_selectAssignTerms.at(j));
    if (parse_assign != NULL) {
      debugRule(pc, str(strbuf() << "Assignment term " << 
			parse_assign->toString() << " " 
			<< curRule->_ruleID << "\n"));
      pelAssign(pc, parse_assign, j);
    }
  }
}


void generateProjectElements(PlanContext* pc)
{
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  Parse_Functor* pf = curRule->_action->_pf;
  PlanContext::FieldNamesTracker* curNamesTracker = pc->_namesTracker;
  
  // determine the projection fields, and the first address to return. 
  // Add 1 for table name     
  std::vector<unsigned int> indices;  
  // iterate through all functor's output
  for (int k = 0; k < pf->args(); k++) {
    Parse_Var* parse_var = dynamic_cast<Parse_Var*>(pf->arg(k));
    int pos = -1;
    if (parse_var != NULL) {
      // care only about vars    
      pos = curNamesTracker->fieldPosition(parse_var->toString());    
    }
    if (pos == -1) { continue; }    
    indices.push_back(pos + 1);
  }
 
  strbuf pelTransformStrbuf("\"" << pf->fn->name << "\" pop");

  for (unsigned int k = 0; k < indices.size(); k++) {
    pelTransformStrbuf << " $" << indices.at(k) << " pop";
  }


  str pelTransformStr(pelTransformStrbuf);
  debugRule(pc, str(strbuf() << "Project head " 
		    << curNamesTracker->toString() 
		    << " " << pelTransformStr << "\n"));
 
  ElementSpecRef projectHeadPelTransform =
    pc->_conf->addElement(New refcounted< PelTransform >(strbuf("ProjectHead|") 
							 << curRule->_ruleID 
							 << "|" << pc->_nodeID,
							 pelTransformStr));

  pc->_ruleStrand->addElement(pc->_conf, projectHeadPelTransform);  
}

void generateAggElement(PlanContext* pc)
{  
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  Parse_AggTerm *aggTerm = curRule->_aggTerm;
  Parse_Functor* baseFunctor = dynamic_cast<Parse_Functor* > (aggTerm->_baseTerm);    

  if (baseFunctor == NULL) { 
    warn << "Cannot process " << curRule->toString() << "\n";
    exit(-1);
    return; 
  }

  Parse_ExprList* groupByFields = aggTerm->_groupByFields;
  Parse_Expr* aggField = aggTerm->_aggFields->at(0);
    
  // generate a names tracker for <groupby fields, agg field, other fields>
  pc->_namesTracker = new PlanContext::FieldNamesTracker(baseFunctor);
  std::vector<unsigned int> groupByFieldNos;      
  int aggFieldNo = -1;

  PlanContext::FieldNamesTracker* baseFunctorTracker = 
    new PlanContext::FieldNamesTracker(baseFunctor);
  
  for (unsigned int k = 0; k < groupByFields->size(); k++) {
    // go through the functor head, but skip the aggField itself    
    Parse_Var* pv = dynamic_cast<Parse_Var* > (groupByFields->at(k));
    if (pv == NULL) { continue; }
    int pos = baseFunctorTracker->fieldPosition(pv->toString()) + 1;
    groupByFieldNos.push_back((uint) pos);
    debugRule(pc, str(strbuf() << "GroupBy Field: " << pv->toString() << " " 
		      << pos << " " << baseFunctorTracker->toString() << "\n"));    
  }
  Parse_Var* pv = dynamic_cast<Parse_Var*> (aggField);
  aggFieldNo = baseFunctorTracker->fieldPosition(pv->toString()) + 1;
  debugRule(pc, str(strbuf() << "Agg Field: " << pv->toString() << " " 
		    << aggFieldNo << " " << baseFunctorTracker->toString() << "\n"));    
  delete baseFunctorTracker;

  Table::AggregateFunction* af = 0;
  str aggStr;
  if (aggTerm->_oper == Parse_Agg::MIN) {
    af = &Table::AGG_MIN;
    aggStr = "min";
  } 
  if (aggTerm->_oper == Parse_Agg::MAX) {
    af = &Table::AGG_MAX;
    aggStr = "max";
  } 

  if (aggTerm->_oper == Parse_Agg::COUNT) {
    af = &Table::AGG_COUNT;
    aggStr = "count";
  } 

  
  // get the table, create the index
  Catalog::TableInfo* ti =  pc->_catalog->getTableInfo(baseFunctor->fn->name);  
  TableRef aggTable = ti->_table;
  
  unsigned primaryKey = ti->_tableInfo->primaryKeys.at(0);
  ElementSpecRef aggElement =
    pc->_conf->addElement(New refcounted< Agg >(strbuf("Agg|") 
						<< curRule->_ruleID << "|" 
						<< baseFunctor->fn->name 
						<< "|" << pc->_nodeID 
						<< "|" << 
						primaryKey << "|" <<
						aggStr << "|" << aggFieldNo,
						groupByFieldNos, 
						aggFieldNo, 
						primaryKey, aggStr));

  pc->_ruleStrand->addElement(pc->_conf, aggElement);   

  addWatch(pc, strbuf("DebugAfterAggUpdateEvent|")
	   << curRule->_ruleID 
	   << "|" << pc->_nodeID);
}

void generateAggElements(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  ElementSpecRef pullPushTwo = 
    pc->_conf->addElement(New refcounted< 
			  TimedPullPush >(strbuf("ScanUpdateTimedPullPush|")
					  << curRule->_ruleID 
					  << "|" << pc->_nodeID, 0));
  rs->addElement(pc->_conf, pullPushTwo);  

  // generate the aggregate event listener
  generateAggElement(pc);
}

void generateEcaElements(PlanContext* pc)
{
  // first, generate the event element
  // for recv, register with mux later
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  Parse_Event* event = curRule->_event;
  generateEventElement(pc);        
  if (event->_event == Parse_Event::AGGUPDATE) {
    generateAggElements(pc);
  } else {
    generateMultipleProbeElements(pc);
  }

  // do selections and assignments
  generateSelectionAssignmentElements(pc);

  // do projection based on head
  generateProjectElements(pc);

  // do action. For send, later hook to round robin
  generateActionElement(pc);
}


#endif
