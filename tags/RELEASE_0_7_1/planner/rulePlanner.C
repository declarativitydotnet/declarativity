
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
 * DESCRIPTION: Code to generate dataflows
 *
 */

#ifndef __PL_RULEPLANNER_C__
#define __PL_RULEPLANNER_C__


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
#include "table.h"
#include "lookup.h"
#include "insert.h"
#include "update.h"
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
#include "agg.h"
#include "ruleStrand.h"
#include "planContext.h"
#include "catalog.h"


void debugRule(PlanContext* pc, string msg)
{
  warn << "DEBUG RULE: " << pc->_ruleStrand->_ruleID << " " 
       << pc->_ruleStrand->_ruleStrandID << " " << msg; 
}

#include "rulePel.C"

void addWatch(PlanContext* pc, string b)
{
  RuleStrand* rs = pc->_ruleStrand;

  if (pc->_outputDebugFile == NULL) {
    ElementSpecPtr print = 
      pc->_conf->addElement(ElementPtr(new PrintWatch(b, pc->_catalog->getWatchTables())));
    rs->addElement(pc->_conf, print);  
  } else {
    ElementSpecPtr print = 
      pc->_conf->addElement(ElementPtr(new PrintWatch(b, pc->_catalog->getWatchTables(), 
					 pc->_outputDebugFile)));
    rs->addElement(pc->_conf, print);  
  }
}

void generateEventElement(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  pc->_namesTracker = new PlanContext::FieldNamesTracker(rs->_eca_rule->_event->_pf);
  
  // update, create an updater
  if (rs->eventType() == Parse_Event::UPDATE || 
      rs->eventType() == Parse_Event::AGGUPDATE) {
    debugRule(pc, "Update event NamesTracker " + pc->_namesTracker->toString() + "\n");
    Catalog::TableInfo* ti =  pc->_catalog->getTableInfo(rs->eventFunctorName());
    if (ti == NULL) {
      warn << "Table " << rs->eventFunctorName() << "is not found\n";
      exit(-1);
      return;
    }
    TablePtr tablePtr = ti->_table;
    ElementSpecPtr updateTable =
       pc->_conf->addElement(ElementPtr(new Update("ScanUpdate|" + curRule->_ruleID 
                                                   + "|" + rs->eventFunctorName()
                                                   + "|" + pc->_nodeID, tablePtr)));
    rs->addElement(pc->_conf, updateTable);

    // add a debug element
    addWatch(pc, "DebugAfterUpdateEvent|"+curRule->_ruleID+"|"+pc->_nodeID);
    
    if (curRule->_probeTerms.size() > 0) {
      ElementSpecPtr pullPush = 
	pc->_conf->addElement(ElementPtr(new TimedPullPush("UpdateEventTimedPullPush|"
						     + curRule->_ruleID 
						     + "|" + pc->_nodeID, 0)));
      rs->addElement(pc->_conf, pullPush);
    }
  }

  if (rs->eventType() == Parse_Event::RECV) {
    debugRule(pc, "Recv event NamesTracker " + pc->_namesTracker->toString() + "\n");
    addWatch(pc, "DebugAfterRecvEvent|" + curRule->_ruleID + "|" + pc->_nodeID);

    if (curRule->_probeTerms.size() == 0) {
      ElementSpecPtr slot = 
	pc->_conf->addElement(ElementPtr(new Slot("RecvEventSlot|" + curRule->_ruleID 
						     + "|" + pc->_nodeID)));
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
    addWatch(pc, "DebugBeforeInsertAction|" + curRule->_ruleID + "|" + 
                 rs->actionFunctorName() + "|" + pc->_nodeID);

    Catalog::TableInfo* ti =  pc->_catalog->getTableInfo(rs->actionFunctorName());    
    if (ti == NULL) {
      warn << "Table " << rs->actionFunctorName() << "is not found\n";
      exit(-1);
      return;
    }
    TablePtr tablePtr = ti->_table;

    ElementSpecPtr insertElement
      = pc->_conf->addElement(ElementPtr(new Insert("Insert|" + curRule->_ruleID + "|" 
				   + rs->actionFunctorName() + "|" + pc->_nodeID, tablePtr)));

    ElementSpecPtr insertPullPush = 
      pc->_conf->addElement(ElementPtr(new TimedPullPush("Insert|" + curRule->_ruleID 
				       + "|" + pc->_nodeID, 0)));
    
    ElementSpecPtr sinkS 
    = pc->_conf->addElement(ElementPtr(new Discard("DiscardInsert")));

    rs->addElement(pc->_conf, insertElement);
    rs->addElement(pc->_conf, insertPullPush);
    rs->addElement(pc->_conf, sinkS);
  }

  if (rs->actionType() == Parse_Action::SEND) {    
    // do a pel transform, figure out what is the field number
    Parse_Functor* head = curRule->_action->_pf;
    string loc = head->fn->loc;
    int locationIndex = 0;
    for (int k = 0; k < head->args(); k++) {
      Parse_Var* parse_var = dynamic_cast<Parse_Var*>(head->arg(k));
      if (parse_var != NULL && parse_var->toString() == loc) {
	locationIndex = k+1;
      }
    }
    addWatch(pc, "DebugBeforeSendAction|"+ curRule->_ruleID + "|" + pc->_nodeID);
    ostringstream oss;
    oss << "$" << locationIndex << " pop swallow pop";
    ElementSpecPtr sendPelTransform =
      pc->_conf->addElement(ElementPtr(new PelTransform("SendActionAddress|"+curRule->_ruleID 
					   + "|" + pc->_nodeID,oss.str())));
    
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

  string probedTableName = probedFunctor->fn->name;
  
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

  TablePtr probedTable = ti->_table;
  
  // should we use a uniqLookup or a multlookup? 
  // Check that the rightJoinKey is the primary key
  OL_Context::TableInfo* tableInfo 
    = pc->_catalog->getTableInfo(probedTableName)->_tableInfo;
 
  if (tableInfo->primaryKeys.size() == 1 && 
      tableInfo->primaryKeys.at(0) == rightJoinKey) {
    // use a unique index
    ElementSpecPtr lookupElement =
      pc->_conf->addElement(ElementPtr(new UniqueLookup("UniqueLookup|"+ 
				       curRule->_ruleID + "|" + probedTableName
				       + "|" + pc->_nodeID, 
				       probedTable,
				       leftJoinKey, 
				       rightJoinKey, 
				       0)));
/*
    debugRule(pc, "Unique lookup " + " " + rs->eventFunctorName() + " " 
			   + probedTableName + " " + leftJoinKey << " " 
			   + rightJoinKey + "\n");
*/

    pc->_ruleStrand->addElement(pc->_conf, lookupElement);    
  } else {
    ElementSpecPtr lookupElement =
      pc->_conf->addElement(ElementPtr(new MultLookup("MultLookup|" 
						     + curRule->_ruleID 
						     + "|" + probedTableName 
						     + "|" + pc->_nodeID, 
						     probedTable,
						     leftJoinKey, 
						     rightJoinKey, 0)));

    pc->_catalog->createMultIndex(probedTableName, rightJoinKey);
  
/*
    debugRule(pc, "Mult lookup " + curRule->_ruleID + " " + probedTableName + " " 
			   + leftJoinKey + " " + rightJoinKey + "\n");
*/

    pc->_ruleStrand->addElement(pc->_conf, lookupElement);
  }
  
 
  int numFieldsProbe = incomingNamesTracker->fieldNames.size();
  debugRule(pc, "Probe before merge " + incomingNamesTracker->toString() + "\n");
  incomingNamesTracker->mergeWith(probedNamesTracker->fieldNames); 
  debugRule(pc, "Probe after merge " + incomingNamesTracker->toString() + "\n");

 
  ElementSpecPtr noNull 
    = pc->_conf->addElement(ElementPtr(new NoNullField("NoNull|" 
						      + curRule->_ruleID 
						      + "|" + probedTableName
						      + "|" + pc->_nodeID, 1)));

  pc->_ruleStrand->addElement(pc->_conf, noNull);

  
  // debugRule(pc, "Number of join selections " + leftJoinKeys.size()-1 + "\n");

  for (uint k = 1; k < leftJoinKeys.size(); k++) {
    int leftField = leftJoinKeys.at(k);
    int rightField = rightJoinKeys.at(k);
    ostringstream selectionPel;
    selectionPel << "$0 " << (leftField+1) << " field " << " $1 " 
		 << rightField+1 << " field == not ifstop $0 pop $1 pop";

    debugRule(pc, "Join selections " + selectionPel.str() + "\n");
    ElementSpecPtr joinSelections =
      pc->_conf->addElement(ElementPtr(new PelTransform("JoinSelections|"
					   + curRule->_ruleID + "|" 
					   + probedTableName + "|" + pc->_nodeID, 
					   selectionPel.str())));    
    pc->_ruleStrand->addElement(pc->_conf, joinSelections);
  }
 
  // Take the joined tuples and produce the resulting path
  // form the pel projection. 
  //Keep all fields on left, all fields on right except the join keys
  ostringstream pelProject("\"join|");
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

  string pelProjectStr = pelProject.str();
  ElementSpecPtr joinPelTransform 
    = pc->_conf->addElement(ElementPtr(new PelTransform("JoinPelTransform|" 
				       + curRule->_ruleID + "|" + probedTableName + 
                                       "|" + pc->_nodeID, pelProjectStr)));

  delete probedNamesTracker;

  pc->_ruleStrand->addElement(pc->_conf, joinPelTransform);

}


void generateMultipleProbeElements(PlanContext* pc)
{
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  RuleStrand* rs = pc->_ruleStrand;

  for (uint k = 0; k < curRule->_probeTerms.size(); k++) {    
    Parse_Functor* pf = curRule->_probeTerms.at(k);
    
    debugRule(pc, "Probing " + curRule->_event->_pf->fn->name
		      + " " + pf->toString() + "\n");
    
    generateProbeElements(pc, pf);

    if (curRule->_probeTerms.size() - 1 != k) {
      ElementSpecPtr pullPush =
	pc->_conf->addElement(ElementPtr(new TimedPullPush("ProbePullPush|" 
					  + curRule->_ruleID + "|" + pc->_nodeID, 0)));
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
      debugRule(pc, "Selection term " + parse_select->toString() + " " 
			+ curRule->_ruleID + "\n");
      pelSelect(pc, parse_select, j); 
    }
    Parse_Assign* parse_assign 
      = dynamic_cast<Parse_Assign *>(curRule->_selectAssignTerms.at(j));
    if (parse_assign != NULL) {
      debugRule(pc, "Assignment term " + parse_assign->toString() + " " 
			+ curRule->_ruleID + "\n");
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
 
  ostringstream pel;
  pel << "\"" << pf->fn->name << "\" pop";

  for (unsigned int k = 0; k < indices.size(); k++) {
    pel << " $" << indices.at(k) << " pop";
  }


  string pelTransformStr = pel.str();
  debugRule(pc, "Project head " + curNamesTracker->toString() 
		    + " " + pelTransformStr + "\n");
 
  ElementSpecPtr projectHeadPelTransform =
    pc->_conf->addElement(ElementPtr(new PelTransform("ProjectHead|" 
							 + curRule->_ruleID 
							 + "|" + pc->_nodeID,
							 pelTransformStr)));

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
    debugRule(pc, "GroupBy Field: " + pv->toString() + " " 
		      + baseFunctorTracker->toString() + "\n");    
  }
  Parse_Var* pv = dynamic_cast<Parse_Var*> (aggField);
  aggFieldNo = baseFunctorTracker->fieldPosition(pv->toString()) + 1;
  debugRule(pc, "Agg Field: " + pv->toString() + " " 
		    + baseFunctorTracker->toString() + "\n");    
  delete baseFunctorTracker;

  Table::AggregateFunction* af = 0;
  string aggStr;
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
  TablePtr aggTable = ti->_table;
  
  unsigned primaryKey = ti->_tableInfo->primaryKeys.at(0);
  ostringstream oss;
  oss << "Agg|" << curRule->_ruleID << "|" << baseFunctor->fn->name 
      << "|" << pc->_nodeID << "|" << primaryKey << "|" << aggStr << "|" << aggFieldNo;
  ElementSpecPtr aggElement =
    pc->_conf->addElement(ElementPtr(new Agg(oss.str(), groupByFieldNos, 
						aggFieldNo, primaryKey, aggStr)));

  pc->_ruleStrand->addElement(pc->_conf, aggElement);   

  addWatch(pc, "DebugAfterAggUpdateEvent|"+curRule->_ruleID+"|"+pc->_nodeID);
}

void generateAggElements(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;
  ECA_Rule* curRule = pc->_ruleStrand->_eca_rule;
  ElementSpecPtr pullPushTwo = 
    pc->_conf->addElement(ElementPtr(new TimedPullPush("ScanUpdateTimedPullPush|"
					  + curRule->_ruleID + "|" + pc->_nodeID, 0)));
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
